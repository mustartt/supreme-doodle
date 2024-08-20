
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <llvm/Support/raw_ostream.h>

namespace rx::package {

class PackageInfo {
public:
  void serialize(llvm::raw_ostream &);
  void deserialize(llvm::MemoryBufferRef);

private:
  std::string Name;
  std::string Version;
  std::string Description;
  std::string RelSrcPath;
  llvm::SmallVector<std::string, 8> Dependencies;
};

} // namespace rx::package
