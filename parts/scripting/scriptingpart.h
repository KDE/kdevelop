/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SCRIPTINGPART_H_
#define _SCRIPTINGPART_H_

#include <Python.h>
#include <qdom.h>
#include <qguardedptr.h>
#include "kdevplugin.h"


class ScriptingPart : public KDevPlugin
{
    Q_OBJECT

public:
    ScriptingPart( QObject *parent, const char *name, const QStringList & );
    ~ScriptingPart();

    PyObject *addMenuItem(PyObject *args);
    PyObject *appId(PyObject *args);

private slots:
    void slotScriptAction();

private:
    QDomDocument guiDocument;
    QMap<QString, PyObject*> actions;
};

#endif
