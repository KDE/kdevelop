/*
 *  Copyright (C) 2001 Ian Reinhart Geiser,LinuxPPC Inc,, <geiseri@elysium.linuxppc.org>   
 */
 

#ifndef __KDEVPART_DISTPART_H__
#define __KDEVPART_DISTPART_H__


#include <qguardedptr.h>
#include <kdevpart.h>


class distpartWidget;


class distpartPart : public KDevPart
{
  Q_OBJECT

public:

  distpartPart(KDevApi *api, QObject *parent=0, const char *name=0);
  ~distpartPart();


private:

  QGuardedPtr<distpartWidget> m_widget;

};


#endif
