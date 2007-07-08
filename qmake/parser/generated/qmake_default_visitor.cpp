// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "qmake_default_visitor.h"

namespace qmake
  {

  void default_visitor::visit_project(project_ast *node)
  {
    if  (node->stmt_sequence)
      {
        const list_node<stmt_ast*> *__it =  node->stmt_sequence->to_front(),  *__end =  __it;

        do
          {
            visit_node(__it->element);
            __it =  __it->next;
          }

        while  (__it !=  __end);
      }
  }

  void default_visitor::visit_stmt(stmt_ast *)
{}


} // end of namespace qmake


