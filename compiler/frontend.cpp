#include "rxc/AST/AST.h"
#include "rxc/AST/ASTContext.h"
#include "rxc/Parser/Parser.h"
#include "rxc/Sema/Sema.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/WithColor.h"
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
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

std::pair<StringRef, StringRef> SplitPackage(const StringRef Pkg) {
  size_t First = Pkg.find_first_of(':');
  assert(First != std::string::npos && "Cannot find ':' separator");
  return std::make_pair(Pkg.substr(0, First), Pkg.substr(First + 1));
}

struct FileContext {
public:
  FileContext(llvm::StringRef Filename, std::unique_ptr<MemoryBuffer> FileBuf)
      : Filename(Filename), FileBuf(std::move(FileBuf)) {}

  FileContext(const FileContext &) = delete;
  FileContext(FileContext &&) = default;
  FileContext &operator=(const FileContext &) = delete;
  FileContext &operator=(FileContext &&) = default;
  ~FileContext() = default;

public:
  std::string Filename;
  ast::ASTContext ASTCtx;
  std::unique_ptr<MemoryBuffer> FileBuf;
};

static ErrorOr<FileContext> OpenFile(llvm::StringRef Filename) {
  auto Result = llvm::MemoryBuffer::getFile(Filename);
  if (auto EC = Result.getError()) {
    return EC;
  }
  return FileContext(Filename, std::move(*Result));
}

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);

  cl::HideUnrelatedOptions(FrontendCategory);
  cl::ParseCommandLineOptions(argc, argv, "rx-frontend command line options");

  if (InputFile.empty()) {
    llvm::WithColor::error(llvm::errs(), "rx-frontend") << "no input file\n";
    return 1;
  }

  SmallString<256> AbsPath;
  if (auto EC = sys::fs::real_path(InputFile, AbsPath)) {
    llvm::WithColor::error(llvm::errs(), "rx-frontend")
        << EC.message() << ": " << InputFile << "\n";
    return 1;
  }

  auto Result = OpenFile(AbsPath);
  if (auto EC = Result.getError()) {
    llvm::WithColor::error(llvm::errs(), "rx-frontend") << EC.message() << "\n";
    return 1;
  }

  FileContext F(std::move(*Result));

  parser::Parser P(F.ASTCtx);
  auto *Root = P.parse(*F.FileBuf);
  P.printAST(outs());

  sema::ForwardDeclarePass FDP;
  FDP.run(static_cast<ast::ProgramDecl *>(Root));

  return 0;
}
