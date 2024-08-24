#include "rxc/AST/AST.h"
#include "rxc/AST/ASTContext.h"
#include "rxc/Basic/Diagnostic.h"
#include "rxc/Basic/SourceManager.h"
#include "rxc/Frontend/TranslationUnitContext.h"
#include "rxc/Sema/LexicalContext.h"
#include "rxc/Sema/LexicalScope.h"
#include "rxc/Sema/Sema.h"
#include "llvm/Support/CommandLine.h"
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

using namespace llvm;
using namespace rx;
using namespace rx::sema;
using namespace rx::ast;

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
    exit(1);
  }

  ConsoleDiagnosticConsumer CDC;
  SourceManager SM;
  TranslationUnitContext TUC(SM, CDC);

  auto OpenResult = SM.OpenFile(AbsPath);
  if (auto EC = OpenResult.getError()) {
    llvm::WithColor::error(llvm::errs(), "rx-frontend") << EC.message() << "\n";
    exit(1);
  }

  auto *RootTU = TUC.setRootFile(*OpenResult);
  TUC.traverseFileImports(RootTU);

  ASTContext GlobalASTContext;
  LexicalScope GlobalScope(LexicalScope::Kind::Global);
  GlobalScope.insert("void",
                     GlobalASTContext.createTypeDecl(
                         SrcRange::Builtin(), SrcRange::Builtin(), "void",
                         Visibility::Public,
                         GlobalASTContext.createBuiltinType(NativeType::Void)));
  GlobalScope.insert("bool",
                     GlobalASTContext.createTypeDecl(
                         SrcRange::Builtin(), SrcRange::Builtin(), "bool",
                         Visibility::Public,
                         GlobalASTContext.createBuiltinType(NativeType::i1)));
  GlobalScope.insert("char",
                     GlobalASTContext.createTypeDecl(
                         SrcRange::Builtin(), SrcRange::Builtin(), "i8",
                         Visibility::Public,
                         GlobalASTContext.createBuiltinType(NativeType::i8)));
  GlobalScope.insert("i32",
                     GlobalASTContext.createTypeDecl(
                         SrcRange::Builtin(), SrcRange::Builtin(), "i32",
                         Visibility::Public,
                         GlobalASTContext.createBuiltinType(NativeType::i32)));
  GlobalScope.insert("i64",
                     GlobalASTContext.createTypeDecl(
                         SrcRange::Builtin(), SrcRange::Builtin(), "i64",
                         Visibility::Public,
                         GlobalASTContext.createBuiltinType(NativeType::i64)));
  GlobalScope.insert("f32",
                     GlobalASTContext.createTypeDecl(
                         SrcRange::Builtin(), SrcRange::Builtin(), "f32",
                         Visibility::Public,
                         GlobalASTContext.createBuiltinType(NativeType::f32)));
  GlobalScope.insert("f64",
                     GlobalASTContext.createTypeDecl(
                         SrcRange::Builtin(), SrcRange::Builtin(), "f64",
                         Visibility::Public,
                         GlobalASTContext.createBuiltinType(NativeType::f64)));
  GlobalScope.insert(
      "string", GlobalASTContext.createTypeDecl(
                    SrcRange::Builtin(), SrcRange::Builtin(), "string",
                    Visibility::Public,
                    GlobalASTContext.createBuiltinType(NativeType::String)));
  LexicalContext LC;

  llvm::SmallVector<TranslationUnit *> BestEffortVisitOrder; // reverse topo order of sccs

  for (auto It = llvm::scc_begin(&TUC), End = llvm::scc_end(&TUC); It != End;
       ++It) {
    auto SCC = *It;
    if (SCC.size() == 1) {
      BestEffortVisitOrder.push_back(SCC[0]);
      continue;
    }
    Diagnostic SccError(Diagnostic::Type::Error, "Found import cycle");
    CDC.emit(std::move(SccError));
    for (auto *TU : SCC) {
      BestEffortVisitOrder.push_back(TU);
      Diagnostic Note(Diagnostic::Type::Note,
                      TU->file()->getAbsPath().str() +
                          " is part of the import cycle");
      CDC.emit(std::move(Note));
    }
  }

  for (auto *TU : BestEffortVisitOrder) {
    llvm::WithColor::remark()
        << "TopoOrder: " << TU->file()->getAbsPath() << "\n";
    /*
    SemaPassManager SPM(CDC, LC);
    SPM.registerPass(ForwardDeclarePass());
    SPM.registerPass(ResolveGlobalType());

    SPM.run(TU->getProgramAST());
    */
  }

  if (Debug) {
    TUC.debug(errs());
    dumpDotGraphToFile(&TUC, "TranslationUnitDependenceGraph.dot",
                       "Translation Unit Dependence Graph");
  }

  return 0;
}
