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
#include <iostream.h>

ProjectView::ProjectView(QObject *parent, const char *name)
    : KDevComponent(parent, name){
    setInstance(ProjectViewFactory::instance());
    //    setXMLFile("kdevprojectview.rc");
}


ProjectView::~ProjectView(){
    delete m_pProjectTree;
}


void ProjectView::setupGUI(){
    kdDebug(9003) << "Building ProjectTreeWidget" << endl;

    m_pProjectTree = new ProjectTreeWidget(this);
    //    w->setIcon()
    m_pProjectTree->setCaption(i18n("Project View"));
    
    embedWidget(m_pProjectTree, SelectView, i18n("PV"), i18n("Project View"));

}

void ProjectView::projectSpaceOpened(ProjectSpace *pProjectSpace){
  m_pProjectTree->setProjectSpace(pProjectSpace);
}
  
void ProjectView::projectSpaceClosed(){
  m_pProjectTree->clear();
}


void ProjectView::readProjectSpaceGlobalConfig(QDomDocument& doc){
  cerr << "kdevelop (projectview): readProjectSpaceGlobalConfig:" << endl;
  m_pProjectTree->readProjectSpaceGlobalConfig(doc);
}

void ProjectView::writeProjectSpaceGlobalConfig(QDomDocument& doc){
  cerr << "kdevelop (projectview): writeProjectSpaceGlobalConfig" << endl;
  m_pProjectTree->writeProjectSpaceGlobalConfig(doc);
}

void ProjectView::setKDevNodeActions(QList<KAction>* pActions){
  cerr << "kdevelop (projectview): setFileActions" << endl;
  m_pFileActions = pActions;
}
QList<KAction>* ProjectView::assembleKDevNodeActions(KDevNode* pNode){
  cerr << "kdevelop (projectview): ProjectView::assembleFileActions" << endl;
  // and now the trick :-)
  emit needKDevNodeActions(this,pNode);
  // now the m_pFileActions should be filled
  return m_pFileActions;
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
void ProjectView::fileClicked(QString absFileName){
  emit gotoSourceFile(absFileName,0);
}
#include "projectview.moc"
