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
  CPrjConfChange(const QString & = QString::null);
  virtual ~CPrjConfChange();

  
  void setConfDir(const QString &confDir);
  QString getConfDir() const {return dir;};
  QString getConfFile() const {return confFile;};
  bool readConfFile(); 
  bool writeConfFile(); 
  
  bool isConfChanged() const {return bChanged; };
  
  bool isOptionAvailable(const QString &option, const QString &suffix=QString::null) const;
  bool isOptionEnabled(const QString &option, const QString &suffix=QString::null) const;
  bool isOptionDisabled(const QString &option, const QString &suffix=QString::null) const 
      { return !isOptionEnabled(option, suffix); };
  
  bool enableOption(const QString &option, const QString &val =QString::null, 
     const QString &suffix=QString::null);
  bool disableOption(const QString &option, const QString &val =QString::null, 
     const QString &suffix=QString::null);
  
  // raw access to configure.in(.in)
  bool isLineAvailable(const QString &regExpLine) const;
  QString getLine(const QString &line) const;
  bool setLine(const QString &, const QString &line=QString::null);
  QString getMacroParameter(const QString &regExpLine) const;

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
