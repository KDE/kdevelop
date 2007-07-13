// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "qmake_visitor.h"

namespace QMake
  {

  visitor::parser_fun_t visitor::_S_parser_table[] =  {
        reinterpret_cast<parser_fun_t>(&visitor::visit_arg_list),
        reinterpret_cast<parser_fun_t>(&visitor::visit_function_args),
        reinterpret_cast<parser_fun_t>(&visitor::visit_function_scope),
        reinterpret_cast<parser_fun_t>(&visitor::visit_id_or_value),
        reinterpret_cast<parser_fun_t>(&visitor::visit_op),
        reinterpret_cast<parser_fun_t>(&visitor::visit_project),
        reinterpret_cast<parser_fun_t>(&visitor::visit_scope_body),
        reinterpret_cast<parser_fun_t>(&visitor::visit_stmt),
        reinterpret_cast<parser_fun_t>(&visitor::visit_value_list),
        reinterpret_cast<parser_fun_t>(&visitor::visit_variable_assignment)
      }; // _S_parser_table[]

} // end of namespace QMake


