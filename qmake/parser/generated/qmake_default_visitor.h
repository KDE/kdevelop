// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef qmake_DEFAULT_VISITOR_H_INCLUDED
#define qmake_DEFAULT_VISITOR_H_INCLUDED

#include "qmake_visitor.h"

namespace qmake
  {

  class KDEVPG_EXPORT default_visitor:  public visitor
    {

    public:
      virtual void visit_project(project_ast *node);
      virtual void visit_stmt(stmt_ast *node);
    };

} // end of namespace qmake

#endif


