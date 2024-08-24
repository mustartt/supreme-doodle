#ifndef RXC_FRONTEND_TRANSLATIONUNITCONTEXT_H
#define RXC_FRONTEND_TRANSLATIONUNITCONTEXT_H

#include "TranslationUnit.h"

#include "rxc/Basic/SourceManager.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/Support/DOTGraphTraits.h"

namespace rx {

class DiagnosticConsumer;
class TranslationUnitContext {
public:
  using Path = llvm::SmallString<256>;
  using FileTable = std::map<Path, TranslationUnit>;
  using IteratorType = FileTable::iterator;

  TranslationUnitContext(SourceManager &SM, DiagnosticConsumer &DC)
      : DC(DC), SM(SM) {}
  TranslationUnitContext(const TranslationUnitContext &) = delete;
  TranslationUnitContext &operator=(const TranslationUnitContext &) = delete;

public:
  TranslationUnit *setRootFile(SourceFile *File);
  void traverseFileImports(TranslationUnit *Start);
  void debug(llvm::raw_ostream &OS);

  IteratorType begin() { return OpenFiles.begin(); }
  IteratorType end() { return OpenFiles.end(); }
  size_t size() const { return OpenFiles.size(); }

private:
  // OpenFiles maps the absolute paths to the source file
  FileTable OpenFiles;
  DiagnosticConsumer &DC;
  SourceManager &SM;
};

} // namespace rx

namespace llvm {

template <>
struct GraphTraits<rx::TranslationUnitContext *>
    : public GraphTraits<rx::TranslationUnit *> {

  class SourceFileIterator {
  public:
    using MapIterator = rx::TranslationUnitContext::FileTable::iterator;
    using iterator_category = std::forward_iterator_tag;
    using value_type = rx::TranslationUnit *;
    using difference_type = std::ptrdiff_t;

    SourceFileIterator(MapIterator It) : It(It) {}

    value_type operator*() const { return &It->second; }

    SourceFileIterator &operator++() {
      ++It;
      return *this;
    }

    SourceFileIterator operator++(int) {
      SourceFileIterator Tmp = *this;
      ++It;
      return Tmp;
    }

    bool operator==(const SourceFileIterator &Other) const {
      return It == Other.It;
    }

    bool operator!=(const SourceFileIterator &Other) const {
      return It != Other.It;
    }

  private:
    MapIterator It;
  };

  using nodes_iterator = SourceFileIterator;

  static NodeRef getEntryNode(rx::TranslationUnitContext *FC) {
    // Return the root source file, assuming it is the first one added.
    assert(FC->size() &&
           "TranslationUnitContext should have at least one file");
    return &FC->begin()->second;
  }

  static nodes_iterator nodes_begin(rx::TranslationUnitContext *FC) {
    return FC->begin();
  }

  static nodes_iterator nodes_end(rx::TranslationUnitContext *FC) {
    return FC->end();
  }

  static size_t size(rx::TranslationUnitContext *FC) { return FC->size(); }
};

template <>
struct DOTGraphTraits<rx::TranslationUnitContext *> : public DefaultDOTGraphTraits {
  explicit DOTGraphTraits(bool Simple = false)
      : DefaultDOTGraphTraits(Simple) {}

  std::string getGraphName(rx::TranslationUnitContext *FC) {
    return "import_graph";
  }

  template <typename GraphType>
  std::string getNodeLabel(const rx::TranslationUnit *SF, const GraphType &) {
    return SF->file()->getFilename().str();
  }

  std::vector<rx::TranslationUnit *> getNodes(rx::TranslationUnitContext *FC) {
    std::vector<rx::TranslationUnit *> Nodes;
    for (auto &[_, TU] : *FC) {
      Nodes.push_back(&TU);
    }
    return Nodes;
  }
};
} // namespace llvm

#endif
