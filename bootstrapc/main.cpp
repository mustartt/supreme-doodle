#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include <fstream>
#include <iostream>

#include "parser.h"

using namespace llvm;

static cl::OptionCategory ToolCategory("parser-cli options");
static cl::opt<std::string> InputFilename(cl::Positional,
                                          cl::desc("<input file>"),
                                          cl::Optional, cl::cat(ToolCategory));
static cl::opt<std::string> OutputFilename("o",
                                           cl::desc("Specify output filename"),
                                           cl::value_desc("filename"),
                                           cl::Optional, cl::cat(ToolCategory));
static cl::opt<bool> Verbose("v", cl::desc("Enable verbose mode"),
                             cl::init(false), cl::cat(ToolCategory));

int main(int argc, const char *argv[]) {
  InitLLVM X(argc, argv);

  cl::HideUnrelatedOptions(ToolCategory);
  cl::ParseCommandLineOptions(argc, argv, "parser-cli command line options");

  std::ifstream InputFile;
  std::ofstream OutputFile;
  std::istream* Input = &std::cin;
  std::ostream* Output = &std::cout;

  if (!InputFilename.empty()) {
    InputFile.open(InputFilename);
    if (InputFile.fail()) {
      std::error_code ec(errno, std::generic_category());
      errs() << "Error opening input file: " << InputFilename << " - "
             << ec.message() << "\n";
      return 1;
    }
    Input = &InputFile;
  }

  if (!OutputFilename.empty()) {
    OutputFile.open(OutputFilename);
    if (OutputFile.fail()) {
      std::error_code ec(errno, std::generic_category());
      errs() << "Error opening output file: " << OutputFilename << " - "
             << ec.message() << "\n";
      return 1;
    }
    Output = &OutputFile;
  }

  parse(*Input, *Output);

  return 0;
}
