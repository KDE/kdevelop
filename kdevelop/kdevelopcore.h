/***************************************************************************
 *   Copyright (C) 2000 by The KDevelop Team                               *
 *   kdevelop-team@fara.cs.uni-potsdam.de                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVELOPCORE_H_
#define _KDEVELOPCORE_H_


#include <qlist.h>

class KDevelop;
class KDevComponent;
class KDevVersionControl;
class KDevLanguageSupport;
class CProject;
class ProjectSpace;


class KDevelopCore : QObject
{
    Q_OBJECT
    
public:
    KDevelopCore(KDevelop *gui);
    ~KDevelopCore();

    void loadInitialComponents();

private:
    void initActions();
    void initComponent(KDevComponent *component);
    
    void loadVersionControl(const QString &name);
    void unloadVersionControl();
    void loadLanguageSupport(const QString &lang);
    void unloadLanguageSupport();
    void loadProjectSpace(const QString &name);
    void unloadProjectSpace();
    void loadProject(const QString &fileName);
    void unloadProject();

private slots:
    // Handling of actions
    void slotFilePrint();
    void slotProjectOpen();
    void slotProjectOpenRecent(const KURL &url);
    void slotProjectClose();
    void slotProjectAddExistingFiles();
    void slotProjectAddNewTranslationFile();
    void slotProjectOptions();
    void slotStop();
    void slotOptionsKDevelopSetup();

    // Handling of component requests
    void executeMakeCommand(const QString &command);
    void executeAppCommand(const QString &command);
    void running(bool runs);
    void gotoSourceFile(const QString &fileName, int lineNo);
    void gotoDocumentationFile(const QString &fileName);
    void gotoProjectApiDoc();
    void gotoProjectManual();

private:
    KDevelop *m_kdevelopgui;
    QList<KDevComponent> m_components;
    QList<KDevComponent> m_runningcomponents;
    
    KDevVersionControl *m_versioncontrol;
    KDevLanguageSupport *m_languagesupport;
    KDevComponent *m_makefrontend;
    KDevComponent *m_appfrontend;
    CProject *m_project;
    CClassStore *m_classstore;
    ProjectSpace* m_projectspace;
};

#endif
