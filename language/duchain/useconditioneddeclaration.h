#ifndef KDEVPLATFORM_USECONDITIONEDDECLARATION_H
#define KDEVPLATFORM_USECONDITIONEDDECLARATION_H

#include "ducontext.h"
#include "use.h"

namespace KDevelop
{

struct DeclarationConditionUse {
  DeclarationConditionUse() : use(-1) {
  }
  IndexedDUContext context;
  int use;
  
  /**
   * Returns whether the use still exists and points to the given declaration
   * 
   * DUChain must be at least read-locked
   * */
  bool isValid(Declaration* decl) const {
    DUContext* ctx = context.data();
    if(!ctx)
      return false;
    if(use < 0 || use >= ctx->usesCount())
      return false;
    
    return ctx->uses()[use].usedDeclaration(ctx->topContext()) == decl;
  }
};

/**
 * A declaration which is automatically deleted when it has no valid uses any more
 * */
class UseConditionedDeclaration
{
  
};

}

#endif // KDEVPLATFORM_USECONDITIONEDDECLARATION_H
