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

class KDevCore;
class KDevProject;
class KDevVersionControl;
class KDevLanguageSupport;
class KDevMakeFrontend;
class KDevAppFrontend;
class ClassStore;
class QDomDocument;

// 2002-02-08 add ccClassStore - daniel
class KDevApi
{
public:
    KDevApi()
        : classStore(0),
          projectDom(0),
          core(0),
          makeFrontend(0),
          appFrontend(0),
          project(0),
          languageSupport(0),
          versionControl(0),
	  /* added by daniel */
	  ccClassStore( 0 )
        {}

    ClassStore *classStore;
    QDomDocument *projectDom;
    KDevCore *core;
    KDevMakeFrontend *makeFrontend;
    KDevAppFrontend *appFrontend;
    KDevProject *project;
    KDevLanguageSupport *languageSupport;
    KDevVersionControl *versionControl;
    /**
     * a new classstore for codecompletion
     * added by daniel
     */
    ClassStore* ccClassStore;

};

#endif
