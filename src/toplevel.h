#ifndef __TOPLEVEL_H__
#define __TOPLEVEL_H__


#include "kdevtoplevel.h"


/*! \mainpage GIDEON ARCHITECTURE

\section intro Introduction
This page explains how the various peaces of gideon work together.

\section install Installed Files
TBD.

\section framework Framework
\subsection toplevel_object The Toplevel Opject
There is only one toplevel object of class KDevTopLevel in Gideon. It can be accessed through the static function
TopLevel::getInstance() (see TopLevel).

KDevTopLevel has two derived classes. One is used for the SDI mode (Single Document Interface), the other for the
MDI modes (Multiple Document Interface).They are named TopLevelSDI and TopLevelMDI, respectively.

\subsection SDI Single Document Interface Mode

\subsection MDI Multiple Document Interface Modes
*/




/**\brief This class handles the single object of type KDevTopLevel.

It is completely static (all methods and attributes).
*/

class TopLevel
{
public:

  static KDevTopLevel *getInstance();                                 //!< Get a pointer to the single KDevTopLevel object
  static bool topLevelValid();
  static void          invalidateInstance(KDevTopLevel *instance);    //!< Signal that the object has been (or is about to be) destroyed


private:

  static KDevTopLevel *s_instance;                                    //!< Pointer to the single KDevTopLevel object or 0L
  
};


#endif
