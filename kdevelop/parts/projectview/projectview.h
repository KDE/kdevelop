/***************************************************************************
 *   Copyright (C) 2000 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PROJECTVIEW_H_
#define _PROJECTVIEW_H_

#include "kdevcomponent.h"
#include "kdevnodes.h"

class ProjectTreeWidget;

class ProjectView : public KDevComponent
{
    Q_OBJECT

public:
    ProjectView( QObject *parent=0, const char *name=0 );
    ~ProjectView();

    enum ItemType { Scope, Class, Struct,
                    PublicAttr, ProtectedAttr, PrivateAttr,
                    PublicMethod, ProtectedMethod, PrivateMethod,
                    PublicSlot, ProtectedSlot, PrivateSlot,
                    Signal, GlobalFunction, GlobalVariable };

    QList<KAction>* assembleKDevNodeActions(KDevNode* pNode);

protected:
    virtual void setupGUI();
    virtual void projectSpaceOpened();

private slots:
    void addedFileToProject(KDevFileNode* pNode);
    void removedFileFromProject(KDevFileNode* pNode);
    void addedProject(KDevNode* pNode);

private:
    ProjectTreeWidget* m_pProjectTree;
    QList<KAction> m_pFileActions;
    friend class ProjectTreeWidget;
};

#endif
