#include "rxc/Package/Package.h"

#include "llvm/Support/JSON.h"
#include <llvm/Support/Error.h>

namespace rx::package {

void PackageInfo::serialize(llvm::raw_ostream &OS) {
  llvm::json::Object Obj;
  Obj["Name"] = Name;
  Obj["Version"] = Version;
  Obj["Description"] = Description;
  Obj["RelSrcPath"] = RelSrcPath;

  llvm::json::Array DependenciesArray;
  for (const auto &Dep : Dependencies) {
    DependenciesArray.push_back(Dep);
  }
  Obj["Dependencies"] = std::move(DependenciesArray);

  llvm::json::Value Value(std::move(Obj));
  OS << Value;
}

void PackageInfo::deserialize(llvm::MemoryBufferRef Buf) {
  llvm::StringRef Data = Buf.getBuffer();

  // Parse the JSON data
  llvm::Expected<llvm::json::Value> Result = llvm::json::parse(Data);
  if (auto Err = Result.takeError()) {
    llvm::errs() << "Failed to parse pkg.json: "
                 << llvm::toString(std::move(Err)) << "\n";
    return;
  }

  const auto &Value = Result.get();

  if (!Value.getAsObject()) {
    llvm::errs()
        << "Failed to parse pkg.json: Expected the root to be an object\n";
    return;
  }

  const auto *Root = Value.getAsObject();

  // Deserialize fields
  if (auto *NameValue = Root->get("name")) {
    if (auto V = NameValue->getAsString()) {
      Name = V.value();
    }
  }

  if (auto *NameValue = Root->get("version")) {
    if (auto V = NameValue->getAsString()) {
      Version = V.value();
    }
  }
  if (auto *NameValue = Root->get("description")) {
    if (auto V = NameValue->getAsString()) {
      Description = V.value();
    }
  }

  if (auto *NameValue = Root->get("src")) {
    if (auto V = NameValue->getAsString()) {
      RelSrcPath = V.value();
    }
  }

  if (auto *DependenciesArray = Root->get("dependencies")) {
    if (const auto *Arr = DependenciesArray->getAsArray()) {
      Dependencies.clear();
      for (const auto &Item : *Arr) {
        if (const auto V = Item.getAsString()) {
          Dependencies.push_back(V.value().str());
        }
      }
    }
  }
}

} // namespace rx::package
