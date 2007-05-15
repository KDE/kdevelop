#include "cmakeprojectvisitor.h"
#include "cmakeast.h"

#include <kdebug.h>


void CMakeProjectVisitor::notImplemented() const {
    kDebug(9032) << "not implemented!" << endl;
}

void CMakeProjectVisitor::visit(const CMakeAst *ast)
{
    QList<CMakeAst*> children = ast->children();
    QList<CMakeAst*>::const_iterator it = children.begin();
    QList<CMakeAst*>::const_iterator end = children.end();
    for(; it!=end; it++) {
        kDebug(9032) << "Parsing ast" << endl;
        ast->accept(this);
    }
}
