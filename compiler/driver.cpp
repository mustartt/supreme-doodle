#include "llvm/Support/CommandLine.h"
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/InitLLVM.h>
#include <utility>

using namespace llvm;

static cl::list<std::string>
    Pkgs("pkg", cl::ZeroOrMore, cl::value_desc("pkg:path"),
         cl::desc("Where to find the location of the package source"));

std::pair<StringRef, StringRef> SplitPackage(const StringRef Pkg) {
  size_t First = Pkg.find_first_of(':');
  assert(First != std::string::npos && "Cannot find ':' separator");
  return std::make_pair(Pkg.substr(0, First), Pkg.substr(First + 1));
}

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv,
                              "compiler driver command line options");

  for (const auto &P : Pkgs) {
    const auto [PkgName, PkgPath] = SplitPackage(P);
    outs() << PkgName << " : " << PkgPath << "\n";
  }

  return 0;
}
