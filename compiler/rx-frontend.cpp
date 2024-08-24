#include "rxc/AST/AST.h"
#include "rxc/AST/ASTContext.h"
#include "rxc/AST/ASTPrinter.h"
#include "rxc/Basic/Diagnostic.h"
#include "rxc/Basic/SourceManager.h"
#include "rxc/Frontend/TranslationUnitContext.h"
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

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);

  cl::HideUnrelatedOptions(FrontendCategory);
  cl::ParseCommandLineOptions(argc, argv, "rx-frontend command line options");

  if (InputFile.empty()) {
    llvm::WithColor::error(llvm::errs(), "rx-frontend") << "no input file\n";
    return 1;
  }

  using Path = SmallString<256>;

  Path AbsPath;
  if (auto EC = sys::fs::real_path(InputFile, AbsPath)) {
    llvm::WithColor::error(llvm::errs(), "rx-frontend")
        << InputFile << ": " << EC.message() << "\n";
  }

  ConsoleDiagnosticConsumer CDC;
  SourceManager SM;
  TranslationUnitContext TUContext(SM, CDC);

  auto OpenResult = SM.OpenFile(AbsPath);
  if (auto EC = OpenResult.getError()) {
    llvm::WithColor::error(llvm::errs(), "rx-frontend") << EC.message() << "\n";
    exit(1);
  }

  auto *RootTU = TUContext.setRootFile(*OpenResult);
  TUContext.traverseFileImports(RootTU);
  TUContext.debug(llvm::errs());

  for (auto *N : llvm::depth_first(&TUContext)) {
    llvm::outs() << "DFS: " << N->file()->getFilename() << "\n";
  }

  dumpDotGraphToFile(&TUContext, "TranslationUnitDependenceGraph.dot",
                     "Translation Unit Dependence Graph");

  return 0;
}
