/* $Id$
 */
#include "ada_utils.hpp"
#include "adasupport.hpp"

QString qtext (const RefAdaAST& n)
{
  return QString::fromLatin1 (text (n).c_str ());
}

