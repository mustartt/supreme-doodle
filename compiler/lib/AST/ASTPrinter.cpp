#include "rxc/AST/ASTPrinter.h"
#include "rxc/AST/AST.h"
#include "rxc/AST/ASTVisitor.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>

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
  void visit(ExportedDecl *node) override;
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
  void visit(ObjectLiteral *node) override;
  void visit(BoolLiteral *node) override;
  void visit(CharLiteral *node) override;
  void visit(NumLiteral *node) override;
  void visit(StringLiteral *node) override;

private:
  static llvm::raw_ostream::Colors GetNodeColor(const ASTNode *Node) {
    if (dynamic_cast<const Decl *>(Node))
      return llvm::raw_ostream::RED;
    if (dynamic_cast<const Expression *>(Node)) {
      if (dynamic_cast<const LiteralExpr *>(Node)) {
        return llvm::raw_ostream::YELLOW;
      }
      return llvm::raw_ostream::GREEN;
    }
    return llvm::raw_ostream::CYAN;
  }

  llvm::raw_ostream &PrintNodeDetails(ASTNode *Node) {
    auto &OS = printNodePrefix();
    llvm::WithColor(OS, GetNodeColor(Node), true) << Node->name();
    OS << ": id(" << Node << ")";
    OS << " range(" << Node->Loc << ")";

    if (auto *D = dynamic_cast<Decl *>(Node)) {
      OS << " decl(" << D->getName() << ")";
      OS << " loc(" << D->getDeclLoc() << ")";
      if (D->getDeclaredType()) {
        OS << " decl_type(" << D->getDeclaredType()->getType().getTypeName()
           << ")";
      }
    }

    if (auto *E = dynamic_cast<Expression *>(Node)) {
      OS << " type(" << E->getExprType().getTypeName() << ")";
    }

    if (auto *S = dynamic_cast<ScopedASTNode *>(Node)) {
      OS << " scope(" << S->getLexicalScope() << ")";
    }
    return OS << " ";
  }

  llvm::raw_ostream &printNodePrefix() {
    for (size_t i = 1; i < Depth; ++i) {
      if (DepthFlag[i]) {
        Output << "| ";
      } else {
        Output << "  ";
      }
    }
    if (Depth == 0) {
    } else if (IsLast.back()) {
      Output << "`-";
      DepthFlag[Depth] = false;
    } else {
      Output << "|-";
    }
    return Output;
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
  PrintNodeDetails(Node) << "\n";

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
  PrintNodeDetails(Node) << Node->getName() << "\n";

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ImportDecl *Node) {
  auto &OS = PrintNodeDetails(Node);

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
  OS << "\n";

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ExportedDecl *Node) {
  PrintNodeDetails(Node) << Node->getVisibility() << "\n";
  {
    Scope _(*this, true);
    assert(Node->getExportedDecl() && "Decl is missing");
    Node->getExportedDecl()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(VarDecl *Node) {
  PrintNodeDetails(Node) << "\n";

  if (Node->getInitializer()) {
    Scope _(*this, true);
    Node->getInitializer()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(TypeDecl *Node) {
  assert(Node->getDeclaredType() && "TypeDecl missing declared type");
  PrintNodeDetails(Node) << "\n";

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(UseDecl *Node) {
  assert(Node->getDeclaredType() && "UseDecl missing declared type");
  PrintNodeDetails(Node) << "\n";

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ImplDecl *Node) {
  PrintNodeDetails(Node) << "\n";

  for (const auto [Idx, Impl] : llvm::enumerate(Node->getImpls())) {
    Scope _(*this, Idx == Node->getImpls().size() - 1);
    Impl->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(FuncDecl *Node) {
  PrintNodeDetails(Node) << "\n";

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
  PrintNodeDetails(Node) << "\n";

  if (Node->getDefaultValue()) {
    Scope _(*this, true);
    Node->getDefaultValue()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(BlockStmt *Node) {
  PrintNodeDetails(Node) << "\n";

  for (auto [Idx, Stmt] : llvm::enumerate(Node->getStmts())) {
    Scope _(*this, Idx == Node->getStmts().size() - 1);
    Stmt->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ReturnStmt *Node) {
  PrintNodeDetails(Node) << "\n";

  if (Node->getExpr()) {
    Scope _(*this, true);
    Node->getExpr()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(DeclStmt *Node) {
  PrintNodeDetails(Node) << "\n";

  assert(Node->getDecl() && "Must have actual decl");
  {
    Scope _(*this, true);
    Node->getDecl()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ExprStmt *Node) {
  PrintNodeDetails(Node) << "\n";
  {
    Scope _(*this, true);
    Node->getExpr()->accept(*this);
  }
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(IfExpr *Node) {
  PrintNodeDetails(Node) << "\n";
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
  PrintNodeDetails(Node) << "\n";

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
  PrintNodeDetails(Node) << "\n";
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
  PrintNodeDetails(Node) << "\n";
  {
    Scope _(*this, true);
    Node->getExpr()->accept(*this);
  }
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(CallExpr *Node) {
  PrintNodeDetails(Node) << "\n";

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
  PrintNodeDetails(Node) << "\n";

  if (Node->getExpr()) {
    Scope _(*this, true);
    Node->getExpr()->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(IndexExpr *Node) {
  PrintNodeDetails(Node) << "\n";

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
  PrintNodeDetails(Node) << "\n";

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
  PrintNodeDetails(Node) << "\n";
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(ObjectLiteral *Node) {
  auto &OS = PrintNodeDetails(Node);
  OS << "{";
  for (auto [Idx, KV] : llvm::enumerate(Node->getFields())) {
    OS << KV.first();
    if (Idx + 1 != Node->getFields().size()) {
      OS << ", ";
    }
  }
  OS << "}\n";

  for (auto [Idx, KV] : llvm::enumerate(Node->getFields())) {
    Scope _(*this, Idx + 1 == Node->getFields().size());
    KV.second->accept(*this);
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(BoolLiteral *Node) {
  PrintNodeDetails(Node) << (Node->getValue() ? "True" : "False") << "\n";
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(CharLiteral *Node) {
  PrintNodeDetails(Node) << " '" << Node->getValue() << "'\n";
  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(NumLiteral *Node) {
  auto &OS = PrintNodeDetails(Node);

  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Node->getValue().print(Os);
  Str.pop_back();

  OS << Str;
  if (Node->isInteger()) {
    OS << " integer\n";
  } else {
    OS << " float\n";
  }

  DepthFlag[Depth] = true;
}

void ASTPrinterVisitor::visit(StringLiteral *Node) {
  PrintNodeDetails(Node) << " \"" << Node->getValue() << "\"\n";
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
