/***************************************************************************
                          cprjconfchange.h  -  description
                             -------------------
    begin                : Wed Nov 21 2001
    copyright            : (C) 2001 by Walter Tasin
    email                : tasin@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CPRJCONFCHANGE_H
#define CPRJCONFCHANGE_H

#include <qstring.h>
#include <qstringlist.h>

class CPrjConfChange 
{

public: 
  /* set up the class with the directory, where configure.in(.in) should be */ 
  CPrjConfChange(const QString &);
  virtual ~CPrjConfChange();

  
  void setConfDir(const QString &confDir);
  QString getConfDir() const {return dir;};
  QString getConfFile() const {return confFile;};
  bool readConfFile(); 
  bool writeConfFile(); 
  
  bool isConfChanged() const {return bChanged; };
  
  bool isOptionAvailable(const QString &option) const;
  bool isOptionEnabled(const QString &option) const;
  bool isOptionDisabled(const QString &option) const 
      { return !isOptionEnabled(option); };
  bool setOption(const QString &option, const QString &value);
  
protected:
  void searchConfFile(); 
  QStringList getConfContent(); 
  void setConfContent(const QStringList &); 
  
private:
  bool bChanged;
  QString dir;
  QString confFile;

  QStringList content;
};

#endif
