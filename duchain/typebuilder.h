
#ifndef TYPEBUILDER_H
#define TYPEBUILDER_H

#include <language/duchain/builders/abstracttypebuilder.h>

#include "duchainexport.h"
#include "contextbuilder.h"

typedef KDevelop::AbstractTypeBuilder<AST, NameAST, ContextBuilder> TypeBuilderBase;

class KDEVCLANGDUCHAIN_EXPORT TypeBuilder : public TypeBuilderBase
{
public:
    TypeBuilder();
protected:
    virtual bool VisitTypeLoc(clang::TypeLoc TL);
};

#endif // TYPEBUILDER_H
