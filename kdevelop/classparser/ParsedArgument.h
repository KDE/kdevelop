/********************************************************************
* Name    :                                                         *
* ------------------------------------------------------------------*
* File    : ParsedArgument                                            *
* Author  :                                         *
* Date    : ~dy-~dn-~dd                                             *
*                                                                   *
* ------------------------------------------------------------------*
* Purpose :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Usage   :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Macros:                                                           *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Types:                                                            *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Functions:                                                        *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Modifications:                                                    *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
*********************************************************************/
#ifndef _PARSEDARGUMENT_H_INCLUDED
#define _PARSEDARGUMENT_H_INCLUDED

#include <qstring.h>

class CParsedArgument
{
public: // Constructor & Destructor

  CParsedArgument();
  ~CParsedArgument();

public: // Public attributes

  /** The name of the argument. */
  QString name;

  /** The type of the argument. */
  QString type;

public: // Public methods to set attribute values

  /** Set the argument name */
  void setName( const char *aName );

  /** Set the type */
  void setType( const char *aType );

public: // Public queries

  /** Return this arguments as a string. */
  void toString( QString &str );

  /** Output this object to stdout */
  void out();
};

#endif
