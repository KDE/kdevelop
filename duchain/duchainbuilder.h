#ifndef DUCHAINBUILDER_H
#define DUCHAINBUILDER_H

#include <clang/Parse/ParseAST.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/Sema.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/Diagnostic.h>
#include <llvm/Support/Host.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

#include <language/duchain/topducontext.h>
#include "duchainexport.h"

using namespace clang;

KDEVCLANGDUCHAIN_EXPORT KDevelop::TopDUContext *createFileContext(KDevelop::IndexedString url, const QByteArray& content);

#endif //DUCHAINBUILDER_H