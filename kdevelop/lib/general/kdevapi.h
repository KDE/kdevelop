/***************************************************************************
                             kdevapi.h
                             -------------------                                         
    copyright            : (C) 2000 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef _KDEVAPI_H_
#define _KDEVAPI_H_

class KDevVersionControl;
class KDevLanguageSupport;
class KDevEditorManager;
class KDevMakeFrontend;
class KDevAppFrontend;
class ProjectSpace;
class ClassStore;


class KDevApi
{
public:
    KDevApi()
        : projectSpace(0),
          classStore(0),
          versionControl(0),
          languageSupport(0),
          editorManager(0),
          makeFrontend(0),
          appFrontend(0)
        {}

    ProjectSpace *projectSpace;
    ClassStore *classStore;
    KDevVersionControl *versionControl;
    KDevLanguageSupport *languageSupport;
    KDevEditorManager *editorManager;
    KDevMakeFrontend *makeFrontend;
    KDevAppFrontend *appFrontend;
};

#endif
