/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVPLUGIN_H_
#define _KDEVPLUGIN_H_

#include <qobject.h>
#include <kxmlguiclient.h>

class KDevCore;
class KDevProject;
class KDevVersionControl;
class KDevLanguageSupport;
class KDevEditorManager;
class KDevMakeFrontend;
class KDevAppFrontend;
class ClassStore;
class KDevApi;
class KDevPartController;
class KDevTopLevel;
class KDevDebugger;


// 2002-02-08 added ccClassStore( ) - daniel

/**
 * This is the base class for all components like doc tree view, LFV, RFV
 * and so on.
 */
class KDevPlugin : public QObject, public KXMLGUIClient
{
    Q_OBJECT
    
public:
    /**
     * Constructs a component.
     */
    KDevPlugin( QObject *parent, const char *name=0 );
    /**
     * Destructs a component.
     */
    ~KDevPlugin();

    /**
     * Gives a reference to the toplevel widget.
     */
    KDevTopLevel *topLevel();

    /**
     * Gives a reference to the application core
     */
    KDevCore *core();
    /**
     * Gives a reference to the current project component
     */
    KDevProject *project();
    /**
     * Gives a reference to the version control component
     */
    KDevVersionControl *versionControl();
    /**
     * Gives a reference to the language support component
     */
    KDevLanguageSupport *languageSupport();
    /**
     * Gives a reference to the make frontend component
     */
    KDevMakeFrontend *makeFrontend();
    /**
     * Gives a reference to the application frontend component
     */
    KDevAppFrontend *appFrontend();
    /**
     * Gives a reference to the class store
     */
    ClassStore *classStore();
    
    /**
     * Gives a reference to the codecompletion classstore
     * added by daniel
     */
    ClassStore* ccClassStore( ); 
    
    /**
     * Gives a reference to the DOM tree that represents
     * the project file.
     */
    QDomDocument *projectDom();

    /**
     * Returns a reference to the part controller.
     */
    KDevPartController *partController(); 

    /**
     * Returns a reference to the debugger API.
     */
    KDevDebugger *debugger();


private:
    KDevApi *m_api;
    class Private;
    Private *d;
};

#endif
