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

#include <qpopupmenu.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

#include "main.h"
#include "projectview.h"
#include "projecttreewidget.h"
#include "projectspace.h"

ProjectView::ProjectView(QObject *parent, const char *name)
    : KDevComponent(parent, name){
    setInstance(ProjectViewFactory::instance());
    //    setXMLFile("kdevprojectview.rc");
}


ProjectView::~ProjectView(){
    delete m_pProjectTree;
}


void ProjectView::setupGUI(){
    kdDebug(9009) << "Building ProjectTreeWidget" << endl;

    m_pProjectTree = new ProjectTreeWidget(this);
    //    w->setIcon()
    m_pProjectTree->setCaption(i18n("Project View"));
    
    embedWidget(m_pProjectTree, SelectView, i18n("PV"), i18n("Project View"));


}

void ProjectView::projectSpaceOpened()
{
    kdDebug(9009) << "projectSpaceOpened" << endl;
    ProjectSpace *ps = projectSpace();

    m_pProjectTree->setProjectSpace(ps);
    m_pProjectTree->readProjectSpaceGlobalConfig();

    connect( ps, SIGNAL(sigAddedFileToProject(KDevFileNode*)),
             this, SLOT(addedFileToProject(KDevFileNode*)) );
    connect( ps, SIGNAL(sigRemovedFileFromProject(KDevFileNode*)),
             this, SLOT(removedFileFromProject(KDevFileNode*)) );
    connect( ps, SIGNAL(sigAddedProject(KDevNode*)),
             this, SLOT(addedProject(KDevNode*)) );
}

void ProjectView::projectSpaceClosed()
{
    m_pProjectTree->writeProjectSpaceGlobalConfig();
    m_pProjectTree->clear();
}


QList<KAction>* ProjectView::assembleKDevNodeActions(KDevNode* pNode){
  kdDebug(9009) << "ProjectView::assembleFileActions" << endl;
  // and now the trick :-)
  emit needKDevNodeActions(pNode,&m_pFileActions);
  // now the m_pFileActions should be filled
  return &m_pFileActions;
}


void ProjectView::addedFileToProject(KDevFileNode* pNode){
  m_pProjectTree->addedFileToProject(pNode);
}   


void ProjectView::removedFileFromProject(KDevFileNode* pNode){
  m_pProjectTree->removedFileFromProject(pNode);
}


void ProjectView::addedProject(KDevNode*){
  m_pProjectTree->refresh();
}

#include "projectview.moc"
