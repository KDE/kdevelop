/***************************************************************************
 *   Copyright (C) 2000 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVNOTES_H_
#define _KDEVNOTES_H_

#include <qstring.h>
#include <qobject.h>

/** KDevNodes are data objects for all information objects in KDevelop
    for instance files,directories,projects,classes
    they contains all nessesary information to describe a unique object
    KDevNodes works together with KDevActions, but are not restricted to this context
*/

class KDevNode : public QObject { // QObject only for metainformation
  Q_OBJECT
public:
  KDevNode(QString projectSpaceName="",QString projectName="");
  virtual ~KDevNode(){}
  void setProjectSpaceName(QString projectSpaceName);
  void setProjectName(QString projectName);
  QString projectSpaceName();
  QString projectName();
  virtual void show();
protected:
  QString m_projectSpaceName;
  QString m_projectName;
};

class KDevFileNode : public KDevNode {
  Q_OBJECT
public:
  KDevFileNode(QString absFileName,QString projectSpaceName="",QString projectName="");
  void setAbsoluteFileName(QString absFileName);
  QString absoluteFileName();
  virtual void show();
protected:
  QString m_absFileName;
};

class KDevDirectoryNode : public KDevNode {
  Q_OBJECT
public:
  KDevDirectoryNode(QString dir,QString projectSpaceName="",QString projectName="");
  void setDir(QString dir);
  QString dir();
  virtual void show();
protected:
  QString m_dir;
};

#endif
