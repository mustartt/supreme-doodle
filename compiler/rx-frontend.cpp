#include "rxc/AST/AST.h"
#include "rxc/AST/ASTContext.h"
#include "rxc/AST/ASTPrinter.h"
#include "rxc/Parser/Parser.h"
#include "rxc/Sema/LexicalContext.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/WithColor.h"
#include <llvm/ADT/DepthFirstIterator.h>
#include <llvm/ADT/GraphTraits.h>
#include <llvm/ADT/SCCIterator.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/DOTGraphTraits.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/GraphWriter.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <queue>
#include <utility>

using namespace llvm;
using namespace rx;

static llvm::cl::OptionCategory FrontendCategory("rx-frontend options");

static cl::list<std::string>
    Pkgs("pkg", cl::ZeroOrMore, cl::value_desc("pkg:path"),
         cl::desc("Where to find the location of the package source"),
         cl::cat(FrontendCategory));

static cl::opt<std::string>
    InputFile("src", cl::Positional, cl::value_desc("input file"),
              cl::desc("Source file to pass to the compiler frontend"),
              cl::cat(FrontendCategory));

static cl::opt<bool> Debug("debug", cl::Optional, cl::init(false),
                           cl::desc("Enable debugging"),
                           cl::cat(FrontendCategory));

std::pair<StringRef, StringRef> SplitPackage(const StringRef Pkg) {
  size_t First = Pkg.find_first_of(':');
  assert(First != std::string::npos && "Cannot find ':' separator");
  return std::make_pair(Pkg.substr(0, First), Pkg.substr(First + 1));
}

using Path = llvm::SmallString<256>;

class SourceFile {
public:
  SourceFile(llvm::StringRef AbsPath, std::unique_ptr<MemoryBuffer> FileBuf)
      : AbsPath(AbsPath), FileBuf(std::move(FileBuf)), ProgramAST(nullptr) {}

  SourceFile(const SourceFile &) = delete;
  SourceFile(SourceFile &&) = default;
  SourceFile &operator=(const SourceFile &) = delete;
  SourceFile &operator=(SourceFile &&) = default;
  ~SourceFile() = default;

public:
  llvm::StringRef getFilename() const { return sys::path::filename(AbsPath); }
  llvm::StringRef getBaseDir() const { return sys::path::parent_path(AbsPath); }

  void parse() {
    parser::Parser P(ASTCtx);
    ProgramAST = P.parse(*FileBuf);
  }

  ArrayRef<ast::ImportDecl *> getImports() {
    assert(ProgramAST && "No AST Available");
    return ProgramAST->getImports();
  }

  void addImportedFiles(SourceFile *File) { ImportedFiles.push_back(File); }

  void debug(raw_ostream &OS) {
    OS << "File: " << AbsPath << "\n";
    for (auto *ImportedFile : ImportedFiles) {
      OS << "Imported: " << ImportedFile->AbsPath << "\n";
    }
    OS << FileBuf->getBuffer();
    if (ProgramAST) {
      ast::ASTPrinter Printer;
      Printer.print(OS, ProgramAST);
    }
    LexContext.debug(OS);
  }

public:
  Path AbsPath;
  ast::ASTContext ASTCtx;
  sema::LexicalContext LexContext;
  std::unique_ptr<MemoryBuffer> FileBuf;
  ast::ProgramDecl *ProgramAST;
  llvm::SmallVector<SourceFile *> ImportedFiles;
};

class FileContext {
public:
  static ErrorOr<SourceFile> OpenFile(llvm::StringRef AbsPath) {
    auto Result = llvm::MemoryBuffer::getFile(AbsPath);
    if (auto EC = Result.getError()) {
      return EC;
    }
    return SourceFile(AbsPath, std::move(*Result));
  }

  SourceFile *setRootFile(llvm::StringRef AbsPath) {
    assert(OpenFiles.empty() && "FileContext is not empty");
    auto Result = FileContext::OpenFile(AbsPath);
    if (auto EC = Result.getError()) {
      llvm::WithColor::error(llvm::errs(), "rx-frontend")
          << InputFile << ": " << EC.message() << "\n";
      exit(1);
    }
    OpenFiles.emplace(AbsPath, std::move(*Result));
    return &OpenFiles.at(AbsPath);
  }

  void TraverseFileImports(SourceFile *Start) {
    std::queue<SourceFile *> Queue;
    Queue.push(Start);

    while (!Queue.empty()) {
      auto *File = std::move(Queue.front());
      File->parse();
      Queue.pop();

      if (Debug)
        errs() << "visiting: " << File->AbsPath << "\n";
      Path BaseDir = File->getBaseDir();

      for (auto Import : File->getImports()) {
        if (Import->getImportType() != ast::ImportDecl::ImportType::File)
          continue;

        Path AbsFilePath = BaseDir;
        sys::path::append(AbsFilePath, Import->getImportPath());

        if (!OpenFiles.count(AbsFilePath)) {
          auto Result = OpenFile(AbsFilePath);
          if (auto EC = Result.getError()) {
            llvm::WithColor::error(llvm::errs(), "rx-frontend")
                << Import->getImportPath() << " imported from "
                << File->getFilename() << ": " << EC.message() << "\n";
            continue;
          }
          OpenFiles.emplace(AbsFilePath, std::move(*Result));
          Queue.push(&OpenFiles.at(AbsFilePath));
        }
        errs() << "adding " << AbsFilePath << "\n";
        File->addImportedFiles(&OpenFiles.at(AbsFilePath));
      }
    }
  }

  void debug(llvm::raw_ostream &OS) {
    for (auto &[P, SF] : OpenFiles) {
      OS << P << ":\n";
      SF.debug(OS);
      OS << "\n";
    }
  }

public:
  // OpenFiles maps the absolute paths to the source file
  std::map<llvm::SmallString<256>, SourceFile> OpenFiles;
};

namespace llvm {

template <> struct GraphTraits<SourceFile *> {
  using NodeRef = SourceFile *;
  using ChildIteratorType = llvm::SmallVectorImpl<SourceFile *>::iterator;

  static NodeRef getEntryNode(SourceFile *SF) { return SF; }

  static ChildIteratorType child_begin(NodeRef N) {
    return N->ImportedFiles.begin();
  }

  static ChildIteratorType child_end(NodeRef N) {
    return N->ImportedFiles.end();
  }
};

// Graph type specialization for FileContext
template <>
struct GraphTraits<FileContext *> : public GraphTraits<SourceFile *> {

  class SourceFileIterator {
  public:
    using MapIterator = std::map<Path, SourceFile>::iterator;
    using iterator_category = std::forward_iterator_tag;
    using value_type = SourceFile *;
    using difference_type = std::ptrdiff_t;
    using pointer = SourceFile **;
    using reference = SourceFile *&;

    SourceFileIterator(MapIterator It) : It(It) {}

    value_type operator*() const { return &It->second; }

    SourceFileIterator &operator++() {
      ++It;
      return *this;
    }

    SourceFileIterator operator++(int) {
      SourceFileIterator Tmp = *this;
      ++It;
      return Tmp;
    }

    bool operator==(const SourceFileIterator &Other) const {
      return It == Other.It;
    }

    bool operator!=(const SourceFileIterator &Other) const {
      return It != Other.It;
    }

  private:
    MapIterator It;
  };

  using nodes_iterator = SourceFileIterator;

  static NodeRef getEntryNode(FileContext *FC) {
    // Return the root source file, assuming it is the first one added.
    assert(!FC->OpenFiles.empty() &&
           "FileContext should have at least one file");
    return &FC->OpenFiles.begin()->second;
  }

  static nodes_iterator nodes_begin(FileContext *FC) {
    return FC->OpenFiles.begin();
  }

  static nodes_iterator nodes_end(FileContext *FC) {
    return FC->OpenFiles.end();
  }

  static size_t size(FileContext *FC) { return FC->OpenFiles.size(); }
};

template <>
struct DOTGraphTraits<FileContext *> : public DefaultDOTGraphTraits {
  explicit DOTGraphTraits(bool Simple = false)
      : DefaultDOTGraphTraits(Simple) {}

  std::string getGraphName(FileContext *FC) { return "FileContextGraph"; }

  template <typename GraphType>
  std::string getNodeLabel(const SourceFile *SF, const GraphType &) {
    return SF->getFilename().str();
  }
  std::vector<SourceFile *> getNodes(FileContext *FC) {
    std::vector<SourceFile *> Nodes;
    for (auto &Entry : FC->OpenFiles) {
      Nodes.push_back(&Entry.second);
    }
    return Nodes;
  }
};

} // namespace llvm

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);

  cl::HideUnrelatedOptions(FrontendCategory);
  cl::ParseCommandLineOptions(argc, argv, "rx-frontend command line options");

  if (InputFile.empty()) {
    llvm::WithColor::error(llvm::errs(), "rx-frontend") << "no input file\n";
    return 1;
  }

  Path AbsPath;
  if (auto EC = sys::fs::real_path(InputFile, AbsPath)) {
    llvm::WithColor::error(llvm::errs(), "rx-frontend")
        << InputFile << ": " << EC.message() << "\n";
  }

  FileContext FContext;
  auto *F = FContext.setRootFile(AbsPath);
  FContext.TraverseFileImports(F);

  llvm::outs() << "\n";
  FContext.debug(llvm::outs());

  for (auto Node : llvm::depth_first(&FContext)) {
    llvm::errs() << "DFS : " << Node->getFilename() << "\n";
  }

  auto It = llvm::scc_begin(&FContext);
  auto End = llvm::scc_end(&FContext);

  for (; It != End; ++It) {
    auto SCC = *It;
    llvm::errs() << "SCC: "
                 << "\n";
    for (auto *SF : SCC) {
      errs() << "  " << SF->getFilename() << "\n";
    }
  }

  dumpDotGraphToFile(&FContext, "file-include.dot", "File Include Graph");

  return 0;
}
