/***************************************************************************
                          kdeprojectspace.h 
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEPROJECTSPACE_H
#define KDEPROJECTSPACE_H

#include "automakeprojectspace.h"
#include <klibloader.h>
#include <kaboutdata.h>

/** handles all po's for every binary project...
  *@author Sandy Meier
  */

class KDEProjectSpace : public AutomakeProjectSpace  {
Q_OBJECT
public: 
	KDEProjectSpace(QObject* parent=0,const char* name=0);
	~KDEProjectSpace();
	
	virtual	void modifyDefaultFiles();
	virtual void setupGUI();
	virtual bool writeGlobalConfig(QDomDocument& doc,QDomElement& psElement);
	virtual bool writeUserConfig(QDomDocument& doc,QDomElement& psElement);
	
	virtual bool readGlobalConfig(QDomDocument& doc,QDomElement& psElement);
	virtual bool readUserConfig(QDomDocument& doc,QDomElement& psElement);
	virtual KAboutData* aboutPlugin();
	QList<FileGroup> defaultFileGroups();

	protected slots:
  void slotProjectAddNewTranslationFile();
	virtual void updateMakefilesAm();
	virtual void slotBuildMake();
	virtual void slotBuildRebuildAll();
	virtual void slotBuildCleanRebuildAll();
	virtual void slotBuildExecute();
	virtual void slotBuildExecuteWithArgs();
	virtual void slotBuildDistClean();
	virtual void slotBuildAutoconf();
	virtual void slotBuildConfigure();


 protected:
	KAboutData* m_pAboutData;
};
#endif
