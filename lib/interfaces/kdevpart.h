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

#ifndef _KDEVPART_H_
#define _KDEVPART_H_

#include <qstring.h>
#include <kparts/part.h>
#include <qdom.h>
#include <kaction.h>
#include <qptrlist.h>

class KDevCore;
class KDevProject;
class KDevVersionControl;
class KDevLanguageSupport;
class KDevEditorManager;
class KDevMakeFrontend;
class KDevAppFrontend;
class ClassStore;
class KDevApi;



/**
 * This is the base class for all components like doc tree view, LFV, RFV
 * and so on.
 */

class KDevPart : public KParts::Part
{
    Q_OBJECT
    
public:
    /**
     * Constructs a component. The first parameters (of type KDevApi)
     * should not be just by any part. It's just internally used.
     */
    KDevPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    /**
     * Destructs a component.
     */
    ~KDevPart();

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
     * Gives a reference to the DOM tree that represents
     * the project file.
     */
    QDomDocument *projectDom();

private:
    // These methods are not meaningful and are therefore forbidden
    void setWidget(QWidget *w);
    QWidget *widget();
    KDevApi *m_api;
};

#endif
