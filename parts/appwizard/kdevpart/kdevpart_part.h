/*
 *  Copyright (C) 2001 $AUTHOR$ <$EMAIL$>   
 */
 

#ifndef __KDEVPART_$APPNAMEUC$_H__
#define __KDEVPART_$APPNAMEUC$_H__


#include <qguardedptr.h>
#include <kdevpart.h>


class $APPNAME$Widget;


class $APPNAME$Part : public KDevPart
{
  Q_OBJECT

public:
   
  $APPNAME$Part(KDevApi *api, QObject *parent=0, const char *name=0);
  ~$APPNAME$Part();

  
private:
    
  QGuardedPtr<$APPNAME$Widget> m_widget;

};


#endif
