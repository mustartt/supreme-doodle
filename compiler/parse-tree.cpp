#include "ast/include/ASTContext.h"
#include "parser/parser.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include <llvm/Support/ErrorHandling.h>

static llvm::cl::OptionCategory ToolCategory("parser-cli options");
static llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                                llvm::cl::desc("<input file>"),
                                                llvm::cl::Optional,
                                                llvm::cl::cat(ToolCategory));
static llvm::cl::opt<std::string>
    OutputFilename("o", llvm::cl::desc("Specify output filename"),
                   llvm::cl::value_desc("filename"), llvm::cl::Optional,
                   llvm::cl::cat(ToolCategory));
static llvm::cl::opt<bool> Verbose("v", llvm::cl::desc("Enable verbose mode"),
                                   llvm::cl::init(false),
                                   llvm::cl::cat(ToolCategory));
static llvm::cl::opt<std::string>
    ProductionMode("mode", llvm::cl::desc("Specify which mode to run"),
                   llvm::cl::value_desc("ast | tree"), llvm::cl::Optional,
                   llvm::cl::init("ast"), llvm::cl::cat(ToolCategory));

int main(int argc, const char *argv[]) {
  llvm::InitLLVM X(argc, argv);

  llvm::cl::HideUnrelatedOptions(ToolCategory);
  llvm::cl::ParseCommandLineOptions(argc, argv,
                                    "parse-tree command line options");

  // Read input file
  std::unique_ptr<llvm::MemoryBuffer> Buffer;
  if (InputFilename.empty()) {
    auto FileOrErr = llvm::MemoryBuffer::getSTDIN();
    if (std::error_code EC = FileOrErr.getError()) {
      llvm::WithColor::error(llvm::errs(), "parse-tree")
          << "Error reading from stdin: " << EC.value() << " " << EC.message()
          << "\n";
      return 1;
    }
    Buffer = std::move(*FileOrErr);
  } else {
    auto FileOrErr = llvm::MemoryBuffer::getFile(InputFilename);
    if (std::error_code EC = FileOrErr.getError()) {
      llvm::WithColor::error(llvm::errs(), "parse-tree")
          << "Error opening input file: " << InputFilename << " - "
          << EC.message() << "\n";
      return 1;
    }
    Buffer = std::move(*FileOrErr);
  }

  // Open output file
  std::unique_ptr<llvm::ToolOutputFile> Out;
  if (OutputFilename.empty()) {
    std::error_code EC;
    Out =
        std::make_unique<llvm::ToolOutputFile>("-", EC, llvm::sys::fs::OF_None);
  } else {
    std::error_code EC;
    Out = std::make_unique<llvm::ToolOutputFile>(OutputFilename, EC,
                                                 llvm::sys::fs::OF_None);
    if (EC) {
      llvm::WithColor::error(llvm::errs(), "parse-tree")
          << "Error opening output file: " << OutputFilename << " - "
          << EC.message() << "\n";
      return 1;
    }
  }

  rx::ast::ASTContext Context;
  rx::parser::Parser TheParser(Context);

  TheParser.parse(*Buffer);

  if (ProductionMode == "tree") {
    TheParser.printParseTree(Out->os());
  } else if (ProductionMode == "ast") {
    TheParser.printAST(Out->os());
  } else {
    llvm::WithColor::error(llvm::errs(), "parse-tree")
        << "Invalid mode: " << ProductionMode << "\n";
  }

  Out->keep();

  if (Verbose) {
    llvm::errs() << "Parsing completed successfully.\n";
    auto Errors = TheParser.getErrors();
    if (!Errors.empty()) {
      llvm::errs() << "Encountered " << Errors.size() << " errors:\n";
      for (const auto &Error : Errors) {
        // llvm::errs() << Error << "\n";
      }
    } else {
      llvm::errs() << "No errors encountered.\n";
    }
  }

  return 0;
}

