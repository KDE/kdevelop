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
#include "kdevpart.h"


class ScriptingPart : public KDevPart
{
    Q_OBJECT

public:
    ScriptingPart( KDevApi *api, QObject *parent=0, const char *name=0 );
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
