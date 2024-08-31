#include "rxc/AST/ASTPrinter.h"
#include "rxc/AST/AST.h"
#include "rxc/AST/ASTVisitor.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>

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
  void visit(VarDecl *node) override;
  void visit(TypeDecl *node) override;
  void visit(UseDecl *node) override;
  void visit(ImplDecl *node) override;
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
  void visit(DeclRefExpr *node) override;
  void visit(BoolLiteral *node) override;
  void visit(CharLiteral *node) override;
  void visit(NumLiteral *node) override;
  void visit(StringLiteral *node) override;

private:
  static llvm::raw_ostream &PrintNodeDetails(llvm::raw_ostream &OS,
                                             ASTNode *Node) {
    OS << "id(" << Node << ")";
    OS << " range(" << Node->Loc << ")";

    if (auto *D = dynamic_cast<Decl *>(Node)) {
      OS << " decl(" << D->getName() << ")";
      OS << " loc(" << D->getDeclLoc() << ")";
      auto *T = D->getType();
      OS << " declared_type(" << (T ? T->getTypeName() : "<unknown>") << ")";
    }

    if (auto *E = dynamic_cast<Expression*>(Node)) {
      auto *T = E->getExprType();
      OS << " expr_type(" << (T ? T->getTypeName() : "<unknown>") << ")";
    }

    if (auto *S = dynamic_cast<ScopedASTNode *>(Node)) {
      OS << " scope(" << S->getLexicalScope() << ")";
    }
    return OS;
  }

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

void ASTPrinterVisitor::visit(ProgramDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  OS << "ProgramDecl: ";
  PrintNodeDetails(OS, Node);
  printNodePrefix(Str);

  if (Node->getPackage()) {
    Scope _(*this, Node->getDecls().empty() && Node->getImports().empty());
    Node->getPackage()->accept(*this);
  }

  const auto Imports = Node->getImports();
  for (const auto [Idx, Value] : llvm::enumerate(Imports)) {
    Scope _(*this, Idx == Imports.size() - 1 && Node->getDecls().empty());
    Value->accept(*this);
  }

  const auto Decls = Node->getDecls();
  for (const auto [Idx, Value] : llvm::enumerate(Decls)) {
    Scope _(*this, Idx == Decls.size() - 1);
    Value->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(PackageDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  OS << "PackageDecl: ";
  PrintNodeDetails(OS, Node);
  printNodePrefix(Str);

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ImportDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  OS << "ImportDecl: ";
  PrintNodeDetails(OS, Node);

  switch (Node->getImportType()) {
  case ImportDecl::ImportType::File: {
    OS << "File ";
    break;
  }
  case ImportDecl::ImportType::Module: {
    OS << "Module ";
    break;
  }
  }
  OS << Node->getImportPath();
  if (Node->getAlias()) {
    OS << " alias " << Node->getAlias();
  }

  printNodePrefix(Str);
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(VarDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  OS << "VarDecl: ";
  PrintNodeDetails(OS, Node);
  printNodePrefix(Str);

  if (Node->getInitializer()) {
    Scope _(*this, true);
    Node->getInitializer()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(TypeDecl *Node) {
  assert(Node->getType() && "TypeDecl missing declared type");
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  OS << "TypeDecl: ";
  PrintNodeDetails(OS, Node);

  if (Node->getType()) {
    OS << " type(" << Node->getType()->getTypeName() << ")";
  } else {
    OS << " type(<unkown_type>)";
  }

  printNodePrefix(Str);
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(UseDecl *Node) {
  assert(Node->getType() && "UseDecl missing declared type");

  std::string Str;
  llvm::raw_string_ostream OS(Str);
  OS << "UseDecl: ";
  PrintNodeDetails(OS, Node);

  printNodePrefix(Str);
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ImplDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "ImplDecl: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);

  for (const auto [Idx, Impl] : llvm::enumerate(Node->getImpls())) {
    Scope _(*this, Idx == Node->getImpls().size() - 1);
    Impl->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(FuncDecl *Node) {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  OS << "FuncDecl: ";
  PrintNodeDetails(OS, Node);
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
  Os << "FuncParamDecl: ";
  PrintNodeDetails(Os, Node);
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
  Os << "BlockStmt: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);

  for (auto [Idx, Stmt] : llvm::enumerate(Node->getStmts())) {
    Scope _(*this, Idx == Node->getStmts().size() - 1);
    Stmt->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ReturnStmt *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "Return: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);

  if (Node->getExpr()) {
    Scope _(*this, true);
    Node->getExpr()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(DeclStmt *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "DeclStmt: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);

  assert(Node->getDecl() && "Must have actual decl");
  {
    Scope _(*this, true);
    Node->getDecl()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ExprStmt *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "ExprStmt: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);
  {
    Scope _(*this, true);
    Node->getExpr()->accept(*this);
  }
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(IfExpr *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "IfExpr: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);

  {
    assert(Node->getCondition());
    Scope _(*this, false);
    Node->getCondition()->accept(*this);
  }
  {
    assert(Node->getBody());
    Scope _(*this, !Node->getElseBlock());
    Node->getBody()->accept(*this);
  }
  if (Node->getElseBlock()) {
    Scope _(*this, true);
    Node->getElseBlock()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ForStmt *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "ForStmt: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);

  if (Node->getPreHeader()) {
    Scope _(*this, false);
    Node->getPreHeader()->accept(*this);
  }

  if (Node->getCondition()) {
    Scope _(*this, false);
    Node->getCondition()->accept(*this);
  }

  if (Node->getPostExpr()) {
    Scope _(*this, false);
    Node->getPostExpr()->accept(*this);
  }

  Scope _(*this, true);
  Node->getBody()->accept(*this);

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(BinaryExpr *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "BinaryExpr: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);

  {
    Scope _(*this, false);
    Node->getLHS()->accept(*this);
  }

  {
    Scope _(*this, true);
    Node->getRHS()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(UnaryExpr *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "UnaryExpr: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);

  {
    Scope _(*this, true);
    Node->getExpr()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(CallExpr *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "CallExpr: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);

  if (Node->getCallee()) {
    Scope _(*this, Node->getArgs().empty());
    Node->getCallee()->accept(*this);
  }

  for (const auto [Idx, Arg] : llvm::enumerate(Node->getArgs())) {
    Scope _(*this, Idx == Node->getArgs().size() - 1);
    Arg->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(AccessExpr *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "AccessExpr: ";
  PrintNodeDetails(Os, Node) << " " << Node->getAccessor();
  printNodePrefix(Str);

  if (Node->getExpr()) {
    Scope _(*this, true);
    Node->getExpr()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(IndexExpr *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "IndexExpr: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);

  if (Node->getExpr()) {
    Scope _(*this, false);
    Node->getExpr()->accept(*this);
  }

  if (Node->getIdx()) {
    Scope _(*this, true);
    Node->getIdx()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(AssignExpr *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "AssignExpr: ";
  PrintNodeDetails(Os, Node);
  printNodePrefix(Str);

  {
    assert(Node->getLHS() && "Missing LHS");
    Scope _(*this, false);
    Node->getLHS()->accept(*this);
  }
  {
    assert(Node->getLHS() && "Missing RHS");
    Scope _(*this, true);
    Node->getRHS()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(DeclRefExpr *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "DeclRefExpr: ";
  PrintNodeDetails(Os, Node) << " " << Node->getSymbol();
  printNodePrefix(Str);
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(BoolLiteral *Node) {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << "BoolLiteral: ";
  PrintNodeDetails(Os, Node);
  if (Node->getValue())
    Os << " true";
  else
    Os << "  false";
  printNodePrefix(Str);

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(CharLiteral *Node) {
  std::string str;
  llvm::raw_string_ostream OS(str);
  OS << "CharLiteral: ";
  PrintNodeDetails(OS, Node) << " '" << Node->getValue() << "'";
  printNodePrefix(str);

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(NumLiteral *Node) {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  OS << "NumLiteral: ";
  PrintNodeDetails(OS, Node) << " ";
  Node->getValue().print(OS);
  Str.pop_back();
  if (Node->isInteger()) {
    OS << " integer";
  } else {
    OS << " float";
  }
  printNodePrefix(Str);
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(StringLiteral *Node) {
  std::string str;
  llvm::raw_string_ostream os(str);
  os << "StringLiteral: ";
  PrintNodeDetails(os, Node) << " \"" << Node->getValue() << "\"";
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
