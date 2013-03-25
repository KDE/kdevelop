#ifndef DECLARATIONBUILDER_H
#define DECLARATIONBUILDER_H

#include <language/duchain/builders/abstractdeclarationbuilder.h>

#include "typebuilder.h"
#include "duchainexport.h"

typedef KDevelop::AbstractDeclarationBuilder<AST, NameAST, TypeBuilder> DeclarationBuilderBase;

class KDEVCLANGDUCHAIN_EXPORT DeclarationBuilder : public DeclarationBuilderBase
{
public:
    DeclarationBuilder(ParseSession* session);

    virtual KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString& url, AST* node,
                                                   KDevelop::ReferencedTopDUContext updateContext
                                                   = KDevelop::ReferencedTopDUContext());
protected:
    virtual bool VisitDeclRefExpr(clang::DeclRefExpr* refExpr);
    virtual bool TraverseDecl(clang::Decl* decl);
    virtual bool VisitTypeLoc(clang::TypeLoc TL);
private:
    QHash<clang::Decl*, KDevelop::Declaration*> m_declToDeclaration;
};

#endif // DECLARATIONBUILDER_H
