/*
 *  Copyright (C) 2001 Ian Reinhart Geiser,LinuxPPC Inc,, <geiseri@elysium.linuxppc.org>   
 */
 

#ifndef __KDEVPART_DISTPART_H__
#define __KDEVPART_DISTPART_H__


#include <qguardedptr.h>
#include <kdevpart.h>
#include <kaction.h>

#include "packagebase.h"

class distpartWidget;


class distpartPart : public KDevPart
{
  Q_OBJECT

public:

  distpartPart(KDevApi *api, QObject *parent=0, const char *name=0);
  ~distpartPart();

public slots:
  void show();

private:
  packageBase *thePackage;
  QGuardedPtr<distpartWidget> m_widget;
  KAction *m_action;
};


#endif
