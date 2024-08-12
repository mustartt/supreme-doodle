#include "llvm/ADT/Statistic.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/TypeFinder.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <cstdint>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/Casting.h>

using namespace llvm;

namespace {

#define DEBUG_TYPE "gc-heapmap"

STATISTIC(NumHeapMaps, "Number of heap maps generated");
STATISTIC(NumStructScanned, "Number of structs scanned");

struct HeapMapRecord {


};

class HeapMapPass : public PassInfoMixin<HeapMapPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
    scanStructType(M);
    return PreservedAnalyses::all();
  }

  void scanStructType(Module &M) {
    TypeFinder finder;
    finder.run(M, false);
    for (StructType *ST : finder) {
      if (processStructType(ST, ST, M.getDataLayout(), 0)) {
        ++NumHeapMaps;
      }
      ++NumStructScanned;
    }
  }

  bool processStructType(StructType* Root, StructType *ST, const DataLayout &DL,
                         uint64_t CurrOffset) {
    bool processed = false;
    const auto *Layout = DL.getStructLayout(ST);
    auto Offsets = Layout->getMemberOffsets();

    ST->dump();
    for (auto [Idx, T] : enumerate(ST->elements())) {
      if (PointerType *Ptr = dyn_cast<PointerType>(T)) {
        if (Ptr->getAddressSpace() == 1) {
          processed = true;
        }
      }
      if (StructType *NestedST = dyn_cast<StructType>(T)) {
        processed |= processStructType(Root, NestedST, DL, Offsets[Idx]);
      }
    }

    return processed;
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "HeapMapPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == DEBUG_TYPE) {
                    MPM.addPass(HeapMapPass());
                    return true;
                  }
                  return false;
                });
          }};
}
