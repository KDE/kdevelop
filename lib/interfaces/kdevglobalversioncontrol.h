/***************************************************************************
 *   Copyright (C) 2002 by Yann Hodique                                    *
 *   Yann.Hodique@lifl.fr                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

// Provides global-level features for version control

#ifndef _KDEVGLOBALVERSIONCONTROL_H_
#define _KDEVGLOBALVERSIONCONTROL_H_

#include <qobject.h>
#include <qstring.h>
#include <qwidget.h>
#include <qmap.h>

#include "kdevplugin.h"

class KDevGlobalVersionControl : public KDevPlugin {
    Q_OBJECT

public:
    KDevGlobalVersionControl(QObject *parent, const char *name, QString id);
    ~KDevGlobalVersionControl();
    
    QString getVcsName();
    virtual QWidget* newProjectWidget(QWidget *parent) = 0;
    virtual void createNewProject(QString dir) = 0;
  

static QMap<QString,KDevGlobalVersionControl*> getVcsMap();
  
private:
    QString m_id;
};

#endif
