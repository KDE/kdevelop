#ifndef __TOPLEVEL_H__
#define __TOPLEVEL_H__


#include "kdevmainwindow.h"


/**\brief This class handles the single object of type KDevMainWindow.

It is completely static (all methods and attributes).
*/

class TopLevel
{
public:

  static KDevMainWindow *getInstance();                                 //!< Get a pointer to the single KDevTopLevel object
  static bool mainWindowValid();
  static void invalidateInstance(KDevMainWindow *instance);             //!< Signal that the object has been (or is about to be) destroyed

  enum Mode { IDEMode, AssistantMode };
  static Mode mode;

private:

  static KDevMainWindow *s_instance;                                    //!< Pointer to the single KDevTopLevel object or 0L
  
};

#endif
