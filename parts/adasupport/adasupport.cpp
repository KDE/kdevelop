#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stack>
#include <string>
#include <antlr/AST.hpp>
#include "AdaParser.hpp"
#include "ada_utils.hpp"

#define eq !strcmp

using namespace std;

std::stack<RefAdaAST> defid_stack;

void AdaParser::push_def_id (const RefAdaAST& defid)
{
#ifdef __DEBUG__
  string txt (text (defid));
  printf ("push_def_id: pushing %s\n", txt.c_str());
#endif
  defid_stack.push (defid);
}

const RefAdaAST& AdaParser::pop_def_id ()
{
  if (defid_stack.size() == 0) {
    fprintf (stderr, "pop_def_id() called on empty stack\n");
    // return static_cast<RefAdaAST>(antlr::nullAST);
    return AdaAST::nullAdaAST;
  }
  RefAdaAST& top = defid_stack.top ();
#ifdef __DEBUG__
  string txt (text (top));
  printf ("pop_def_id: popping %s\n", txt.c_str());
#endif
  defid_stack.pop ();
  return top;
}

bool AdaParser::end_id_matches_def_id (const RefAdaAST& endid)
{
  if (defid_stack.size() == 0)
    return false;
  RefAdaAST& top = defid_stack.top ();
  string defid (text (top));
  defid_stack.pop();
  if (endid == 0 || endid == antlr::nullAST)
    return false;
  string txt (text (endid));
  if (strcasecmp (defid.c_str (), txt.c_str ()) != 0) {
    string errtxt ("End id ");
    errtxt.append (txt);
    errtxt.append (" does not match ");
    errtxt.append (defid);
    reportError (errtxt);
    return false;
  }
#ifdef __DEBUG__
  printf ("end_id_matches_def_id: popped %s\n", txt.c_str());
#endif
  return true;
}

char * strtolower (char *string)
{
  char *p = string;
  if (!p)
    return NULL;
  while (*p)
  {
    if (isupper (*p))
      *p = tolower (*p);
    p++;
  }
  return string;
}

char * extracted_operator (const char *string)
{
  int len = strlen (string);
  static char op[10];

  if (len < 4 && len > 5 || *string != '"' || *(string + len - 1) != '"')
    return NULL;

  strcpy (op, string + 1);
  op[len - 2] = '\0';  /* discard ending quotation mark */
  strtolower (op);
  return op;
}

bool AdaParser::definable_operator (const char *string)
{                                 // operator_symbol sans "/="
  char *op = extracted_operator (string);
  if (op == NULL)
    return false;
  return
     (eq (op, "=") ||
      eq (op, "<") || eq (op, ">") ||
      eq (op, "<=") || eq (op, ">=") ||
      eq (op, "&") || eq (op, "**") ||
      eq (op, "*") || eq (op, "/") || eq (op, "+") || eq (op, "-") ||
      eq (op, "abs") || eq (op, "rem") || eq (op, "mod") ||
      eq (op, "and") || eq (op, "or") || eq (op, "xor") || eq (op, "not"));
}

bool AdaParser::is_operator_symbol (const char *string)
{
  char *op;
  if (definable_operator (string))
    return true;
  op = extracted_operator (string);
  return (eq (op, "/="));
}

