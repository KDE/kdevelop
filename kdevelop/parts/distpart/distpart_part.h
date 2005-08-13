/***************************************************************************
 *   Copyright (C) 2004 by ian reinhart geiser                             *
 *   geiseri@kde.org                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/


#ifndef __KDEVPART_DISTPART_H__
#define __KDEVPART_DISTPART_H__


#include <qguardedptr.h>
#include <kdevplugin.h>
#include <kaction.h>

#include "packagebase.h"

class DistpartDialog;
class KDialogBase;

class DistpartPart : public KDevPlugin {
    Q_OBJECT

public:

    DistpartPart(QObject *parent, const char *name, const QStringList &);
    ~DistpartPart();
    
public slots:
    void show();
    void hide();

private:
    packageBase *RpmPackage, *LsmPackage;
    QGuardedPtr<DistpartDialog> m_dialog;
    KAction *m_action;
    QGuardedPtr<KDialogBase> m_dlg;
};


#endif
