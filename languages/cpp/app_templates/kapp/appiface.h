
#ifndef _%{APPNAMEUC}IFACE_H_
#define _%{APPNAMEUC}IFACE_H_

#include <dcopobject.h>

class %{APPNAME}Iface : virtual public DCOPObject
{
  K_DCOP
public:

k_dcop:
  virtual void openURL(QString url) = 0;
};

#endif // _%{APPNAMEUC}IFACE_H_
