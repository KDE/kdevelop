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
  void setName( QString &aName );
  void setName( const char *aName );

  /** Set the type */
  void setType( QString &aType );

public: // Public attributes
  
  /** Output this object to stdout */
  void out();
};

#endif
