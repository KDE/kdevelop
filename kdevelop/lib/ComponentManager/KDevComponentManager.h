/***************************************************************************
                          KDevComponentManager.h  -  description
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

#ifndef KDEVCOMPONENTMANAGER_H
#define KDEVCOMPONENTMANAGER_H

#include <qwidget.h>
#include <qmap.h>
#include "kdevcomponent.h"

/**
  *@author Omid Givi
  */

class KDialogBase;

class KDevComponentManager : public QObject  {
   Q_OBJECT
public: 
	KDevComponentManager(QWidget *parent=0, const char *name=0);
	~KDevComponentManager();

	// loading components	
  QObject *loadByName(QWidget *parent, const QString &name, const char *className);
  QObject *loadByQuery(QWidget *parent, const QString &serviceType,
  										 const QString &constraint, const char *className);
  QObjectList loadAllByQuery(QWidget *parent, const QString &serviceType, const QString &constraint,
                     				 const char *className);
	
	// unloading components
	void removeComponent(const QString &name);
	void removeAllComponents();
	
	// register a (running) component
	void addRunningComponent(const KDevComponent* kdc);
	void removeRunningComponent(const KDevComponent* kdc);
		
	// querying
	KDevComponent* component(const QString &name);
	QList<KDevComponent> runningComponents();
	QList<KDevComponent> allComponents();
	
	// signalling all components
signals:
	void sigProjectChanged();
	void sigProjectSpaceOpened();
	void sigProjectSpaceClosed();
	void sigStopButtonClicked();
	void sigConfigWidgetRequested(KDialogBase*);
	 	
public slots:	
	void slotProjectChanged();
	void slotProjectSpaceOpened();
	void slotProjectSpaceClosed();
	void slotStopButtonClicked();
	void slotConfigWidgetRequested(KDialogBase*);

private:
  void registerComponent(const QString &name, KDevComponent* kdc);
  void setupConnections(KDevComponent* kdc);
	void postSetupComponent(KDevComponent* kdc);
  QMap<QString, KDevComponent*> name2Component;
  QList<KDevComponent> m_runningComponents;
};

#endif
