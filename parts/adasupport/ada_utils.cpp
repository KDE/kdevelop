/* $Id$
 */
#include "ada_utils.hpp"
#include "AdaTokenTypes.hpp"

const RefAdaAST AdaAST::nullAdaAST = static_cast<RefAdaAST>(antlr::nullAST);

using namespace std;

string text (const RefAdaAST& n)
{
  if (n == 0 || n == AdaAST::nullAdaAST)
    return "";
  string retval;
  int type = n->getType();
  if (type == AdaTokenTypes::DOT) {
    const RefAdaAST& sibs = n->down ();
    retval = text (sibs);
    retval.append (".");
    retval.append (text (sibs->right()));
  } else {
    retval = n->getText();
  }
  /*
  const RefAdaAST& r = n->right();
  if (r != 0 && r->getType () == AdaTokenTypes::DOT) {
    retval.append (".");
    retval.append (text (r->right()));
  }
   */
  return retval;
}

int txteq (RefAdaAST n1, RefAdaAST n2)
{
  if (!n1 || !n2 || n1 == antlr::nullAST || n2 == antlr::nullAST)
    return 0;
  const char* s1 = n1->getText().c_str();
  const char* s2 = n2->getText().c_str();
  if (strcasecmp (s1, s2) != 0)
    return 0;
  n1 = n1->right ();
  n2 = n2->right ();
  if (!n1 || !n2 || n1 == antlr::nullAST || n2 == antlr::nullAST)
    return 1;
  if (n1->getType () == AdaTokenTypes::DOT)
    if (n2->getType () == AdaTokenTypes::DOT)
      return txteq (n1->right (), n2->right ());
    else
      return 0;
  else if (n2->getType () == AdaTokenTypes::DOT)
    return 0;
  return 1;
}

