#ifndef __TOPLEVEL_H__
#define __TOPLEVEL_H__


#include "kdevtoplevel.h"


class TopLevel
{
public:

  static KDevTopLevel *getInstance();
  static void          invalidateInstance(KDevTopLevel *instance);


private:

  static KDevTopLevel *s_instance;
  
};


#endif
