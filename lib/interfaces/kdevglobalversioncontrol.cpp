/***************************************************************************
 *   Copyright (C) 2002 by Yann Hodique                                    *
 *   Yann.Hodique@lifl.fr                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "kdevglobalversioncontrol.h"

#include <qlabel.h>

static QMap<QString,KDevGlobalVersionControl*> vcs_map;


KDevGlobalVersionControl::KDevGlobalVersionControl(QObject *parent, const char *name, QString id)
        : KDevPlugin(parent, name ? name : "KDevGlobalVersionControl"), m_id(id) {
    vcs_map.insert(m_id,this);
}


KDevGlobalVersionControl::~KDevGlobalVersionControl() {
    vcs_map.remove(m_id);
}

QString KDevGlobalVersionControl::getVcsName() {
    return m_id;
}

QMap<QString,KDevGlobalVersionControl*> KDevGlobalVersionControl::getVcsMap() {
    return vcs_map;
}
