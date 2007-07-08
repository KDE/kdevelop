// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef qmake_AST_H_INCLUDED
#define qmake_AST_H_INCLUDED

#include <kdev-pg-list.h>




#include "qmakeparserexport.h"
#include "kdebug.h"
#include <QtCore/QString>

namespace QMake
  {

  class Lexer;
}


namespace qmake
  {

  struct project_ast;

  struct stmt_ast;


  struct ast_node
    {
      enum ast_node_kind_enum {
        Kind_project = 1000,
        Kind_stmt = 1001,
        AST_NODE_KIND_COUNT
      };

      int kind;
      std::size_t start_token;
      std::size_t end_token;
    };

  struct project_ast: public ast_node
    {
      enum
      {
        KIND = Kind_project
      };

      const list_node<stmt_ast *> *stmt_sequence;
    };

  struct stmt_ast: public ast_node
    {
      enum
      {
        KIND = Kind_stmt
      };

    };



} // end of namespace qmake

#endif


