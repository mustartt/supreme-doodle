#include "ASTPrinter.h"
#include "AST.h"
#include "ASTVisitor.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/ErrorHandling.h"

namespace rx::ast {

class Scope;
class ASTPrinterVisitor final : public BaseDeclVisitor,
                                public BaseStmtVisitor,
                                public BaseExprVisitor {
public:
  ASTPrinterVisitor(llvm::raw_ostream &Output)
      : Output(Output), Depth(0), DepthFlag(32, true) {
    IsLast.push_back(false);
  }

  friend class Scope;

public:
  void visit(ProgramDecl *node) override;
  void visit(PackageDecl *node) override;
  void visit(ImportDecl *node) override;
  void visit(StructDecl *node) override;
  void visit(FieldDecl *node) override;
  void visit(VarDecl *node) override;
  void visit(FuncDecl *node) override;
  void visit(BlockStmt *node) override;
  void visit(ReturnStmt *node) override;
  void visit(DeclStmt *node) override;
  void visit(ExprStmt *node) override;
  void visit(IfStmt *node) override;
  void visit(ForStmt *node) override;
  void visit(BinaryExpr *node) override;
  void visit(UnaryExpr *node) override;

private:
  void printTreePrefix() {
    for (size_t i = 1; i < Depth; ++i) {
      if (DepthFlag[i]) {
        Output << "| ";
      } else {
        Output << "  ";
      }
    }
  }

  void printNodePrefix(const std::string &message, bool end = true) {
    printTreePrefix();
    if (Depth == 0) {
      Output << message;
    } else if (IsLast.back()) {
      Output << "`-" << message;
      DepthFlag[Depth] = false;
    } else {
      Output << "|-" << message;
    }
    if (end)
      Output << "\n";
  }

private:
  int Depth;
  llvm::raw_ostream &Output;
  llvm::SmallVector<bool, 32> DepthFlag;
  llvm::SmallVector<bool, 32> IsLast;
};

class Scope {
public:
  Scope(ASTPrinterVisitor &Visitor, bool IsLast) : Visitor(Visitor) {
    Visitor.IsLast.push_back(IsLast);
    ++Visitor.Depth;
  }

  ~Scope() {
    Visitor.IsLast.pop_back();
    --Visitor.Depth;
  }

private:
  ASTPrinterVisitor &Visitor;
};

void ASTPrinterVisitor::visit(ProgramDecl *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "ProgramDecl: " << node->Loc;

  printNodePrefix(Str);

  if (node->getPackage()) {
    Scope _(*this, node->getDecls().empty() && node->getImports().empty());
    node->getPackage()->accept(*this);
  }

  const auto Imports = node->getImports();
  for (const auto [Idx, Value] : llvm::enumerate(Imports)) {
    Scope _(*this, Idx == Imports.size() - 1 && node->getDecls().empty());
    Value->accept(*this);
  }

  const auto Decls = node->getDecls();
  for (const auto [Idx, Value] : llvm::enumerate(Decls)) {
    Scope _(*this, Idx == Decls.size() - 1);
    Value->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(PackageDecl *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "PackageDecl: " << node->getName() << " " << node->Loc;

  printNodePrefix(Str);
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ImportDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "ImportDecl: ";
  auto Path = Node->getPath();
  assert(Path.size() && "Path cannot be empty");

  Os << Path[0];
  for (auto Name : llvm::drop_begin(Path)) {
    Os << "." << Name;
  }
  Os << " ";
  if (Node->getAlias()) {
    Os << "alias " << Node->getAlias();
  }
  Os << " " << Node->Loc;

  printNodePrefix(Str);
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(StructDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "StructDecl: " << Node->getVisibility() << " " << Node->getName() << " "
     << Node->Loc;

  printNodePrefix(Str);

  for (auto Field : Node->getFields()) {
    Scope _(*this, Field == Node->getFields().back());
    Field->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(FieldDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "FieldDecl: " << Node->getVisibility() << " " << Node->getName() << " "
     << Node->Loc;

  if (Node->getDefaultValue()) {
    Scope _(*this, true);
    Node->getDefaultValue()->accept(*this);
  }

  printNodePrefix(Str);
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(VarDecl *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "VarDecl: " << node->getName();

  printNodePrefix(Str);
  if (node->getInitializer()) {
    Scope _(*this, false);
    node->getInitializer()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(FuncDecl *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "FuncDecl: " << node->getName() << "()";

  printNodePrefix(Str);

  if (node->getBody()) {
    Scope _(*this, true);
    node->getBody()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(BlockStmt *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "Block";

  printNodePrefix(Str);

  for (auto &Stmt : node->getStmts()) {
    Scope _(*this, Stmt == node->getStmts().back());
    Stmt->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ReturnStmt *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "Return";

  printNodePrefix(Str);

  if (node->getExpr()) {
    Scope _(*this, true);
    node->getExpr()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(DeclStmt *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "DeclStmt";

  printNodePrefix(Str);

  if (node->getInitializer()) {
    node->getInitializer()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ExprStmt *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "ExprStmt";

  printNodePrefix(Str);
  node->getExpr()->accept(*this);

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(IfStmt *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "If";

  printNodePrefix(Str);

  {
    Scope _(*this, false);
    node->getCondition()->accept(*this);
  }

  {
    Scope _(*this, !node->getElseBlock());
    node->getElseBlock()->accept(*this);
  }

  if (node->getElseBlock()) {
    Scope _(*this, true);
    node->getElseBlock()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ForStmt *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "For";

  printNodePrefix(Str);

  if (node->getPreHeader()) {
    Scope _(*this, false);
    node->getPreHeader()->accept(*this);
  }

  if (node->getCondition()) {
    Scope _(*this, false);
    node->getCondition()->accept(*this);
  }

  if (node->getPostExpr()) {
    Scope _(*this, false);
    node->getPostExpr()->accept(*this);
  }

  Scope _(*this, true);
  node->getBody()->accept(*this);

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(BinaryExpr *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "BinaryExpr: ";

  printNodePrefix(Str);

  {
    Scope _(*this, false);
    node->getLHS()->accept(*this);
  }

  {
    Scope _(*this, true);
    node->getRHS()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(UnaryExpr *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "UnaryExpr: ";

  printNodePrefix(Str);

  {
    Scope _(*this, true);
    node->getExpr()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinter::print(llvm::raw_ostream &Output, ASTNode *root) const {
  ASTPrinterVisitor V(Output);
  if (auto Node = dynamic_cast<Decl *>(root)) {
    Node->accept(V);
    return;
  } else if (auto Node = dynamic_cast<Stmt *>(root)) {
    Node->accept(V);
    return;
  } else if (auto Node = dynamic_cast<Expression *>(root)) {
    Node->accept(V);
    return;
  }
  llvm_unreachable("Invalid AST Node");
}

} // namespace rx::ast
