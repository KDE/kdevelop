/********************************************************************
* Name    :                                                         *
* ------------------------------------------------------------------*
* File    : ParsedSignalText                                            *
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
#ifndef _PARSEDSIGNALTEXT_H_INCLUDED
#define _PARSEDSIGNALTEXT_H_INCLUDED

#include <qstring.h>

class CParsedSignalText
{
public: // Constructor & Destructor

  CParsedSignalText();
  ~CParsedSignalText();

public: // Public attributes

  /** Name of the signal */
  QString signal;

  /** Defaulttext */
  QString defaultText;

public: // Public methods to set attribute values.

  /** Set the signalname. */
  void setSignal( QString &aSignal );
  void setSignal( const char *aSignal );

  /** Set the slot method. */
  void setText( QString &aText );
  void setText( const char *aText );

public: // Public attributes

  /** Output the class as text on stdout */
  void out();
};

#endif
