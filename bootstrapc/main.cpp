#include "LangLexer.h"
#include "LangParser.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

static cl::OptionCategory ToolCategory("parser-cli options");
static cl::opt<std::string> InputFilename(cl::Positional,
                                          cl::desc("<input file>"),
                                          cl::Required, cl::cat(ToolCategory));
static cl::opt<std::string> OutputFilename("o",
                                           cl::desc("Specify output filename"),
                                           cl::value_desc("filename"),
                                           cl::cat(ToolCategory));
static cl::opt<bool> Verbose("v", cl::desc("Enable verbose mode"),
                             cl::init(false), cl::cat(ToolCategory));

int main(int argc, const char *argv[]) {
  InitLLVM X(argc, argv);

  cl::HideUnrelatedOptions(ToolCategory);
  cl::ParseCommandLineOptions(argc, argv, "parser-cli command line options");

  if (Verbose) {
    outs() << "Verbose mode enabled\n";
  }
  outs() << "Input file: " << InputFilename << "\n";
  outs() << "Output file: " << OutputFilename << "\n";

  return 0;
}

void test(std::istream &stream) {
  antlr4::ANTLRInputStream input(stream);
  antlrcpptest::LangLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  antlrcpptest::LangParser parser(&tokens);
  antlr4::tree::ParseTree *tree = parser.program();

  std::cout << tree->toStringTree(&parser, true) << std::endl;
}
