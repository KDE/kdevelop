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

#ifndef _PYTHONPART_H_
#define _PYTHONPART_H_

#include <Python.h>
#include <qdom.h>
#include <qguardedptr.h>
#include "kdevpart.h"


class PythonPart : public KDevPart
{
    Q_OBJECT

public:
    PythonPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~PythonPart();

    PyObject *add_menu_item(PyObject *args);
    PyObject *connect(PyObject *args);
    PyObject *disconnect(PyObject *args);

private slots:
    void projectOpened();
    void projectClosed();
    void slotScriptAction();

private:
    void evalHook(const QList<PyObject> &hook);
    
    QDomDocument guiDocument;
    QMap<QString, PyObject*> actions;
    
    QList<PyObject> projectOpenedHook;
    QList<PyObject> projectClosedHook;
};

#endif
