/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVAPI_H_
#define _KDEVAPI_H_

#include <qobject.h>

class KDevCore;
class KDevProject;
class KDevVersionControl;
class KDevLanguageSupport;
class KDevMakeFrontend;
class KDevAppFrontend;
class ClassStore;
class QDomDocument;
class KDevPartController;
class KDevTopLevel;
class KDevDebugger;


class KDevApiPrivate;

class KDevApi : public QObject
{
    Q_OBJECT
public:

    KDevApi();
    virtual ~KDevApi();

    virtual KDevTopLevel *topLevel() = 0;
    virtual KDevPartController *partController() = 0;
    virtual KDevCore *core() = 0;
    virtual ClassStore *classStore() = 0;
    virtual ClassStore *ccClassStore() = 0;
    virtual KDevDebugger *debugger() = 0;

    QDomDocument *projectDom();
    void setProjectDom(QDomDocument *dom);

    KDevProject *project();
    void setProject(KDevProject *project);
   
    KDevMakeFrontend *makeFrontend();
    void setMakeFrontend(KDevMakeFrontend *makeFrontend);

    KDevAppFrontend *appFrontend();
    void setAppFrontend(KDevAppFrontend *appFrontend);
   
    KDevLanguageSupport *languageSupport();
    void setLanguageSupport(KDevLanguageSupport *languageSupport);

    KDevVersionControl *versionControl();
    void setVersionControl(KDevVersionControl *versionControl);


private:
    
    KDevApiPrivate *d;

};

#endif
