/*
 *  Copyright (C) 2001 Eray Ozkural (exa) <erayo@cs.bilkent.edu.tr>   
 */
 

#ifndef __KDEVPART_SOURCEBROWSER_H__
#define __KDEVPART_SOURCEBROWSER_H__


#include <qguardedptr.h>
#include <kdevpart.h>


class SourceBrowserWidget;


class SourceBrowserPart : public KDevPart
{
  Q_OBJECT

public:
   
  SourceBrowserPart(KDevApi *api, QObject *parent=0, const char *name=0);
  ~SourceBrowserPart();

  
private:
    
  QGuardedPtr<SourceBrowserWidget> m_widget;

};


#endif
