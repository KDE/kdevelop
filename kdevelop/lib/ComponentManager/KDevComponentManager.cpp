/***************************************************************************
                          KDevComponentManager.cpp  -  description
                             -------------------
    begin                : Wed Feb 14 2001
    copyright            : (C) 2001 by Omid Givi
    email                : omid@givi.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KDevComponentManager.h"
#include "partloader.h"
#include <kdevcomponent.h>
#include <qobjectlist.h>
#include <kdebug.h>

class KDialogBase;

// Constructor and Destructor
KDevComponentManager::KDevComponentManager(QWidget *parent, const char *name ) : QObject(parent,name) {
	m_runningComponents.setAutoDelete(true);
}
KDevComponentManager::~KDevComponentManager(){
}

// Load procedures. for more details take a look at partloader
// Note: partloader may not be included directly
QObject* KDevComponentManager::loadByName(QWidget *parent, const QString &name, const char *className){
	QObject* qObj = PartLoader::loadByName(parent, name, className);
	if (qObj){
		KDevComponent* kdevC = static_cast<KDevComponent*>(qObj);
		registerComponent(className, kdevC);
		postSetupComponent(kdevC);
	}else{
    kdDebug(9000) << "KDevComponentManager::component not found:" << name << endl;
	}
	return qObj;
}

QObject* KDevComponentManager::loadByQuery(QWidget *parent, const QString &serviceType,
													 								 const QString &constraint, const char *className){
	QObject* qObj = PartLoader::loadByQuery(parent, serviceType, constraint, className);
	KDevComponent* kdevC = static_cast<KDevComponent*>(qObj);
	registerComponent(className, kdevC);
	postSetupComponent(kdevC);
	return qObj;
}

QObjectList KDevComponentManager::loadAllByQuery(QWidget *parent, const QString &serviceType,
																								 const QString &constraint, const char *className){
	QObjectList qObjList = PartLoader::loadAllByQuery(parent, serviceType, constraint, className);
  QObjectListIt it(qObjList);
  for (; it.current(); ++it){
	  KDevComponent* kdevC = static_cast<KDevComponent*>(it.current());
		registerComponent(kdevC->name(), kdevC);
		/* This may be a problem, if the additional components should be recognized to access
		 * directly. In that case we would want to think about another identification system
		 * for them.
		*/
		postSetupComponent(kdevC);
  }
	return qObjList;
}

// some setup procedures grouped together
void KDevComponentManager::postSetupComponent(KDevComponent* kdc){
  kdc->registerManager(this);
	kdc->setupComponent();
	setupConnections(kdc);
}

// mark a component as running.
// this is needed by the core, perhaps this functionality should be taken off the core and added here.
void KDevComponentManager::addRunningComponent(const KDevComponent* kdc){
  m_runningComponents.append(kdc);
}

// removing the running baby
void KDevComponentManager::removeRunningComponent(const KDevComponent* kdc){
  m_runningComponents.remove(kdc);
}

// Let me (the Component Manager) register the component I've just made
void KDevComponentManager::registerComponent(const QString &name, KDevComponent* kdc){
  kdDebug(9000) << "KDevComponentManager:: Registering: " << name << endl;
  name2Component.insert(name, kdc);
}

// querying the component manager for a component by its ID-name
// Note: this is the superclass name of the component
KDevComponent* KDevComponentManager::component(const QString &name){
  kdDebug(9000) << "KDevComponentManager::component: " << name << endl;
  if (! name2Component[name]){
	  kdDebug(9000) << "KDevComponentManager::component WARNING: the component:" << name << " is null and may be not registerd with the correct ID!" << endl;
  }
	return name2Component[name];
	
}

// we don't need the component anymore, get rid of it
void KDevComponentManager::removeComponent(const QString &name){
  kdDebug(9000) << "KDevComponentManager:: removing: " << name << endl;
	delete name2Component[name];
	name2Component.remove(name);
}

// get rid of all the components (handy for the core)
void KDevComponentManager::removeAllComponents(){
  QMap<QString, KDevComponent*>::Iterator it;
  for( it = name2Component.begin(); it != name2Component.end(); ++it ){
  	removeComponent(it.key());
  }
}

// QT SIGNALS
// distribute the QT signals which the Component Manager receives.
void KDevComponentManager::slotProjectChanged(){
	emit sigProjectChanged();
}

void KDevComponentManager::slotStopButtonClicked(){
	emit sigStopButtonClicked();
}

void KDevComponentManager::slotProjectSpaceOpened(){
	emit sigProjectSpaceOpened();
}

void KDevComponentManager::slotProjectSpaceClosed(){
	emit sigProjectSpaceClosed();
}

void KDevComponentManager::slotConfigWidgetRequested(KDialogBase* pDlg){
	emit sigConfigWidgetRequested(pDlg);
}

// Connections to components. Let every component I make reserve the slots for my signals
void KDevComponentManager::setupConnections(KDevComponent* kdc){
  kdDebug(9000) << "KDevComponentManager::setting up connections for: " << kdc->name() << endl;
	connect (this, SIGNAL(sigProjectSpaceOpened()), kdc, SLOT(slotProjectSpaceOpened()));
	connect (this, SIGNAL(sigProjectSpaceClosed()), kdc, SLOT(slotProjectSpaceClosed()));
	connect (this, SIGNAL(sigProjectChanged()), kdc, SLOT(slotProjectChanged()));
	connect (this, SIGNAL(sigStopButtonClicked()), kdc, SLOT(slotStopButtonClicked()));
	connect (this, SIGNAL(sigConfigWidgetRequested(KDialogBase*)), kdc, SLOT(slotConfigWidgetRequested(KDialogBase*)));
}

// list of all running components
QList<KDevComponent> KDevComponentManager::runningComponents(){
	return m_runningComponents;
}

// list of all the components, for the case somebody (core) would need to access all the component directly.
QList<KDevComponent> KDevComponentManager::allComponents(){
	QList<KDevComponent> list;
  QMap<QString, KDevComponent*>::Iterator it;
  for( it = name2Component.begin(); it != name2Component.end(); ++it ){
  	list.append(name2Component[it.key()]);
  }
  return list;
}
