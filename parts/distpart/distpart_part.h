/*
 *  Copyright (C) 2001 Ian Reinhart Geiser,LinuxPPC Inc,, <geiseri@elysium.linuxppc.org>   
 */


#ifndef __KDEVPART_DISTPART_H__
#define __KDEVPART_DISTPART_H__


#include <qguardedptr.h>
#include <kdevplugin.h>
#include <kaction.h>

#include "packagebase.h"

class DistpartDialog;

class DistpartPart : public KDevPlugin {
    Q_OBJECT

public:

    DistpartPart(QObject *parent, const char *name, const QStringList &);
    ~DistpartPart();

    DistpartDialog * getDlg();
    
public slots:
    void show();
    void hide();

private:
    packageBase *RpmPackage, *LsmPackage;
    QGuardedPtr<DistpartDialog> m_dialog;
    KAction *m_action;
};


#endif
