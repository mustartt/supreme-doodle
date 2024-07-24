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
      : Depth(0), Output(Output), DepthFlag(32, true) {
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
  void visit(FuncParamDecl *node) override;
  void visit(BlockStmt *node) override;
  void visit(ReturnStmt *node) override;
  void visit(DeclStmt *node) override;
  void visit(ExprStmt *node) override;
  void visit(IfExpr *node) override;
  void visit(ForStmt *node) override;
  void visit(BinaryExpr *node) override;
  void visit(UnaryExpr *node) override;
  void visit(CallExpr *node) override;
  void visit(AccessExpr *node) override;
  void visit(IndexExpr *node) override;
  void visit(AssignExpr *node) override;
  void visit(IdentifierExpr *node) override; 
  void visit(BoolLiteral *node) override;
  void visit(CharLiteral *node) override;
  void visit(NumLiteral *node) override;
  void visit(StringLiteral *node) override;

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
  Os << Node->Loc;

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

void ASTPrinterVisitor::visit(VarDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "VarDecl: " << Node->getVisibility() << " " << Node->getName() << " "
     << Node->Loc;

  printNodePrefix(Str);
  if (Node->getInitializer()) {
    Scope _(*this, true);
    Node->getInitializer()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(FuncDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "FuncDecl: " << Node->getVisibility() << " " << Node->getName() << " "
     << Node->Loc;

  printNodePrefix(Str);

  for (const auto [Idx, Param] : llvm::enumerate(Node->getParams())) {
    Scope _(*this, Idx == Node->getParams().size() - 1 && !Node->getBody());
    Param->accept(*this);
  }

  if (Node->getBody()) {
    Scope _(*this, true);
    Node->getBody()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(FuncParamDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "FuncParamDecl: " << Node->getName() << " " << Node->Loc;

  printNodePrefix(Str);

  if (Node->getDefaultValue()) {
    Scope _(*this, true);
    Node->getDefaultValue()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(BlockStmt *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "BlockStmt: " << Node->Loc;

  printNodePrefix(Str);

  for (auto [Idx, Stmt] : llvm::enumerate(Node->getStmts())) {
    Scope _(*this, Idx == Node->getStmts().size() - 1);
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
  Os << "DeclStmt: " << node->Loc;

  printNodePrefix(Str);

  assert(node->getDecl());
  {
    Scope _(*this, true);
    node->getDecl()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ExprStmt *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "ExprStmt: " << node->Loc;

  printNodePrefix(Str);
  {
    Scope _(*this, true);
    node->getExpr()->accept(*this);
  }
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(IfExpr *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "IfExpr: " << node->Loc;

  printNodePrefix(Str);

  {
    assert(node->getCondition());
    Scope _(*this, false);
    node->getCondition()->accept(*this);
  }
  {
    assert(node->getBody());
    Scope _(*this, !node->getElseBlock());
    node->getBody()->accept(*this);
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

void ASTPrinterVisitor::visit(CallExpr *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "CallExpr: " << node->Loc;

  printNodePrefix(Str);

  if (node->getCallee()) {
    Scope _(*this, node->getArgs().empty());
    node->getCallee()->accept(*this);
  }

  for (const auto [Idx, Arg] : llvm::enumerate(node->getArgs())) {
    Scope _(*this, Idx == node->getArgs().size() - 1);
    Arg->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(AccessExpr *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "AccessExpr: " << node->getAccessor() << " " << node->Loc;

  printNodePrefix(Str);

  if (node->getExpr()) {
    Scope _(*this, true);
    node->getExpr()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(IndexExpr *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "IndexExpr: " << node->Loc;

  printNodePrefix(Str);

  if (node->getExpr()) {
    Scope _(*this, false);
    node->getExpr()->accept(*this);
  }

  if (node->getIdx()) {
    Scope _(*this, true);
    node->getIdx()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(AssignExpr *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "AssignExpr: " << node->Loc;

  printNodePrefix(Str);

  {
    assert(node->getLHS() && "Missing LHS");
    Scope _(*this, false);
    node->getLHS()->accept(*this);
  }
  {
    assert(node->getLHS() && "Missing RHS");
    Scope _(*this, true);
    node->getRHS()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(IdentifierExpr *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "IdentifierExpr: " << node->getSymbol() << " " << node->Loc;

  printNodePrefix(Str);
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(BoolLiteral *node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "BoolLiteral: ";
  if (node->getValue())
    Os << "true";
  else
    Os << "false";
  Os << " " << node->Loc;

  printNodePrefix(Str);

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(CharLiteral *node) {
    std::string str;
    llvm::raw_string_ostream os(str);
    os << "CharLiteral: '" << node->getValue() << "' " << node->Loc;

    printNodePrefix(str);
    DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(NumLiteral *node) {
    std::string str;
    llvm::raw_string_ostream os(str);
    os << "NumLiteral: ";
    node->getValue().print(os); 
    str.pop_back();
    if (node->isInteger()) {
      os << " integer";
    } else {
      os << " float";
    }
    os << " " << node->Loc;

    printNodePrefix(str);
    DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(StringLiteral *node) {
    std::string str;
    llvm::raw_string_ostream os(str);
    os << "StringLiteral: \"" << node->getValue() << "\" " << node->Loc;

    printNodePrefix(str);
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
