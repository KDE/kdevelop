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

#include <Python.h>
#include <stdlib.h>
#include <kdebug.h>
#include <kstddirs.h>

#include "kdevcore.h"
#include "pythonfactory.h"
#include "pythonpart.h"


static PythonPart *python_part;
extern "C" {
#if 0
    PyObject *python_add_menuitem(PyObject *self, PyObject *args);
    PyObject *python_connect(PyObject *self, PyObject *args);
#endif
    void initmodule();
}


PythonPart::PythonPart(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name)
{
    setInstance(PythonFactory::instance());
    
    QString xml = QString::fromLatin1("<!DOCTYPE kpartgui SYSTEM \"kpartgui.dtd\">\n"
                                      "<kpartgui version=\"1\" name=\"editorpart\">\n"
                                      "<MenuBar>\n"
                                      "</MenuBar>\n"
                                      "</kpartgui>");
    guiDocument.setContent(xml);
    setDOMDocument(guiDocument);

#if 0
    connect( core(), SIGNAL(configWidgetRequested(KDialogBase*)),
             this, SLOT(configWidgetRequested(KDialogBase*)) );
    connect( core(), SIGNAL(stopButtonClicked()),
             this, SLOT(stopButtonClicked()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
#endif

    python_part = this;

    QString moddir = KGlobal::dirs()->findResourceDir("data", "kdevpython/gideon.py") + "python";
    char *env = strdup(QString::fromLatin1("PYTHONPATH=%1").arg(moddir).latin1());
    putenv(env);
    Py_Initialize();
    free(env);
    initmodule();

    PyRun_SimpleString((char*)"import gideon");

    QString initfile = locate("data", "kdevpython/init.py");
    FILE *f1 = fopen(initfile.latin1(), "r");
    PyRun_SimpleFile(f1, (char*)"");
    fclose(f1);
}


PythonPart::~PythonPart()
{}


PyObject *PythonPart::add_menu_item(PyObject *args)
{
    char *menu, *submenu;
    PyObject *func;
    
    if (!PyArg_ParseTuple(args, (char*)"ssO", &menu, &submenu, &func))
        return 0;

    if (!PyCallable_Check(func)) {
        kdDebug(9011) << "Python function not callable" << endl;
        return 0;
    }
    
    QString menustr = QString::fromLatin1(menu);
    QString submenustr = QString::fromLatin1(submenu);
    QString ident = menustr + submenustr;

    Py_XINCREF(func);
    
    actions.insert(ident, func);
    
    (void) new KAction(submenustr, 0, python_part, SLOT(slotScriptAction()),
                       python_part->actionCollection(), ident.latin1());
    
    QDomElement el = guiDocument.documentElement();
    el = el.namedItem("MenuBar").toElement();
    QDomElement child = el.firstChild().toElement();
    while (!child.isNull()) {
        if (child.tagName() == "Menu" && child.attribute("name") == menustr)
            break;
        child = child.nextSibling().toElement();
    }
    if (child.isNull()) {
        child = guiDocument.createElement(QString::fromLatin1("Menu"));
        child.setAttribute(QString::fromLatin1("name"), menustr);
        el.appendChild(child);
    }
    el = child;
    
    child = guiDocument.createElement(QString::fromLatin1("Action"));
    child.setAttribute(QString::fromLatin1("name"), ident);
    el.appendChild(child);

    kdDebug(9011) << "New dom document: " << guiDocument.toString() << endl;
    
    setDOMDocument(guiDocument);

    Py_INCREF(Py_None);
    return Py_None;
}


PyObject *PythonPart::connect(PyObject *args)
{
    char *hook;
    PyObject *func;
    
    if (!PyArg_ParseTuple(args, (char*)"sO", &hook, &func))
        return 0;

    // FIXME: throw exception otherwise
    if (strcmp(hook, "core_project_opened") == 0) {
        kdDebug(9011) << "connect to core()->projectOpened()" << endl;
        if (projectOpenedHook.isEmpty())
            QObject::connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
        projectOpenedHook.append(func);
        Py_INCREF(func);
    } else if (strcmp(hook, "core_project_closed") == 0) {
        kdDebug(9011) << "connect to core()->projectClosed()" << endl;
        if (projectClosedHook.isEmpty())
            QObject::connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
        projectClosedHook.append(func);
        Py_INCREF(func);
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}


PyObject *PythonPart::disconnect(PyObject *args)
{
    char *hook;
    PyObject *func;
    
    if (!PyArg_ParseTuple(args, (char*)"sO", &hook, &func))
        return 0;

    // FIXME: throw exception otherwise
    if (strcmp(hook, "core_project_opened")) {
        Py_DECREF(func);
        projectOpenedHook.removeRef(func);
        if (projectOpenedHook.isEmpty())
            QObject::disconnect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    } else if (strcmp(hook, "core_project_closed")) {
        Py_DECREF(func);
        projectClosedHook.removeRef(func);
        if (projectClosedHook.isEmpty())
            QObject::disconnect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    }

    Py_INCREF(Py_None);
    return Py_None;
}


void PythonPart::evalHook(const QList<PyObject> &hook)
{
    QListIterator<PyObject> it(hook);
    for (; it.current(); ++it) {
        Py_INCREF(Py_None);
        PyObject *arglist = Py_BuildValue((char*)"()");
        PyEval_CallObject(it.current(), arglist);
    }
}


void PythonPart::projectOpened()
{
    evalHook(projectOpenedHook);
}


void PythonPart::projectClosed()
{
    evalHook(projectClosedHook);
}


void PythonPart::slotScriptAction()
{
    QString ident = QString::fromLatin1(sender()->name());
    kdDebug(9011) << "Action " << ident << " activated" << endl;

    PyObject *arglist = Py_BuildValue((char*)"()");
    PyEval_CallObject(actions[ident], arglist);
}


// Functions callable from scheme

extern "C" {

    static PyObject *gideon_add_menuitem(PyObject */*self*/, PyObject *args)
    {
        return python_part->add_menu_item(args);
    }

    static PyObject *gideon_connect(PyObject */*self*/, PyObject *args)
    {
        return python_part->connect(args);
    }

    static PyObject *gideon_disconnect(PyObject */*self*/, PyObject *args)
    {
        return python_part->disconnect(args);
    }

    static PyObject *core_goto_source_file(PyObject */*self*/, PyObject *args)
    {
        char *filename;
        
        if (!PyArg_ParseTuple(args, (char*)"s", &filename))
            return 0;
        
        python_part->core()->gotoSourceFile(QString::fromLatin1(filename));
        
        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyObject *core_goto_documentation_file(PyObject */*self*/, PyObject *args)
    {
        char *filename;
        
        if (!PyArg_ParseTuple(args, (char*)"s", &filename))
            return 0;
        
        python_part->core()->gotoDocumentationFile(QString::fromLatin1(filename));
        
        Py_INCREF(Py_None);
        return Py_None;
    }

    static struct PyMethodDef gideon_methods[] = {
        { (char*)"add_menuitem",            gideon_add_menuitem,          1, 0 },
        { (char*)"connect",                 gideon_connect,               1, 0 },
        { (char*)"disconnect",              gideon_disconnect,            1, 0 },
        { (char*)"goto_source_file",        core_goto_source_file,        1, 0 },
        { (char*)"goto_documentation_file", core_goto_documentation_file, 1, 0 },
        { 0,                                0,                            0, 0 }
    };

    static void initmodule()
    {
        (void) Py_InitModule((char*)"gideonc", gideon_methods);
    }
}

#include "pythonpart.moc"
