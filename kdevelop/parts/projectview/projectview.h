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

    virtual void readProjectSpaceGlobalConfig(QDomDocument& doc);
    virtual void writeProjectSpaceGlobalConfig(QDomDocument& doc);

protected:
    virtual void setupGUI();
    virtual void projectSpaceOpened(ProjectSpace *pProjectSpace);
    virtual void projectSpaceClosed();

    ProjectTreeWidget* m_pProjectTree;
    

 
    
};

#endif
