/***************************************************************************
 *   Copyright (C) 2000 by The KDevelop Team                               *
 *   kdevelop-team@kdevelop.org                                            *
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
class KDevEditorManager;
class KDevMakeFrontend;
class KDevAppFrontend;
class KDevViewHandler;
class KDevApi;
class ProjectSpace;

class KDevelopCore : QObject
{
    Q_OBJECT
    
public:
    KDevelopCore(KDevelop *gui);
    virtual ~KDevelopCore();

    void loadGlobalComponents();
    void unloadGlobalComponents();

    bool openProjectSpace(const QString &fileName);
    void closeProjectSpace();
    
    KDevViewHandler* viewHandler();
    // Session management
    virtual void saveProperties(KConfig*);
    virtual void readProperties(KConfig*);

private:
    void initActions();
    void initComponent(KDevComponent *component);
    
    void newFile();
    

private slots:
    // Handling of actions
    void slotFilePrint();
    void slotFileNew();
    void slotProjectNew();
    void slotProjectOpen();
    void slotProjectOpenRecent(const KURL &url);
    void slotProjectClose();
    void slotProjectOptions();
    void slotStop();
    void slotOptionsKDevelopSetup();
    
    // Handling of component requests
    void running(bool runs);
    void needKDevNodeActions(KDevNode* pNode,QList<KAction>* pList);

private:
    KDevelop *m_pKDevelopGUI;
    QList<KDevComponent> m_components;
    QList<KDevComponent> m_runningComponents;
    KDevApi *m_api;
};

#endif
