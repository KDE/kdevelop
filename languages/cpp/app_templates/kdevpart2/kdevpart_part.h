
#ifndef __KDEVPART_%{APPNAMEUC}_H__
#define __KDEVPART_%{APPNAMEUC}_H__


#include <qguardedptr.h>
#include <kdevelop/kdevplugin.h>


class %{APPNAME}Widget;


/*
 Please read the README.dox file for more info about this part
 */
class %{APPNAME}Part : public KDevPlugin
{
  Q_OBJECT

public:
   
  %{APPNAME}Part(QObject *parent, const char *name, const QStringList &);
  ~%{APPNAME}Part();

  
private:
    
  QGuardedPtr<%{APPNAME}Widget> m_widget;

};


#endif
