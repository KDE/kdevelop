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
class KDevViewHandler;
class CProject;
class ProjectSpace;

class KDevelopCore : QObject
{
    Q_OBJECT
    
public:
    KDevelopCore(KDevelop *gui);
    virtual ~KDevelopCore();

    void loadInitialComponents();
    KDevViewHandler* viewHandler();

private:
    void initActions();
    void initComponent(KDevComponent *component);
    
    void loadVersionControl(const QString &name);
    void unloadVersionControl();
    void loadLanguageSupport(const QString &lang);
    void unloadLanguageSupport();
    void newFile();
    bool loadProjectSpace(const QString &name);
    void unloadProjectSpace();
    void loadProject(const QString &fileName);
    void unloadProject();
    

private slots:
    // Handling of actions
    void slotFilePrint();
    void slotFileNew();
    void slotProjectNew();
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
    KDevelop *m_pKDevelopGUI;
    QList<KDevComponent> m_components;
    QList<KDevComponent> m_runningComponents;
    
    KDevVersionControl *m_pVersionControl;
    KDevLanguageSupport *m_pLanguageSupport;
    KDevComponent *m_pMakeFrontend;
    KDevComponent *m_pAppFrontend;
    CProject *m_pProject;
    ClassStore *m_pClassStore;
    ProjectSpace* m_pProjectSpace;
    KDevViewHandler* m_pViewHandler;
};

#endif
