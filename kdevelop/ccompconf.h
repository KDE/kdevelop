/***************************************************************************
                          ccompconf.h  -  description
                             -------------------
    begin                : Mon Nov 19 2001
    copyright            : (C) 2001 by Ralf Nolden
    email                : nolden@.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CCOMPCONF_H
#define CCOMPCONF_H

#include <qwidget.h>
#include <ccompconfdlg.h>

class KConfig;
class QStringList;
/**
  *@author Ralf Nolden
  */

class CCompConf : public CCompConfDlg  {
   Q_OBJECT
public: 
	CCompConf(QWidget *parent=0, const char *name=0);
	~CCompConf();
protected slots: // Protected slots
  /** No descriptions */
  void slotTargetArchAdd();
  /** No descriptions */
  void slotUnixVerAdd();
  /** No descriptions */
  void slotHostArchAdd();
  /** No descriptions */
  void slotTargetPlatformAdd();

 private:
  KConfig* config;
	// the stringlist of host architectures
	QStringList hostarchs;
	// the stringlist of unix versions that the host is running on
	QStringList unixvers;
	// the stringlist that the host shall compile for with a crosscompiler
  QStringList compilearchs;
	// the stringlist that the host shall compile for with a crosscompiler
  QStringList compileplatforms;

public slots: // Public slots
  /** Architecture changed, updates the line edit contents
 */
  void slotArchChanged(const QString& arch);
  /** the C preprocessor command CPP has changed for the given
architecture; write it into the according section of the config file. */
  void slotCPPChanged(const QString& cpp);
  /** the current CXX has changed for the architecture
set in the combo above; write it into the according section of
the config file. */
  void slotCXXChanged(const QString& cxx);
  /** the CC for the set architecture changed, save it
into the section of the current architecture */
  void slotCCChanged(const QString& cc);
  /** No descriptions */
  void slotPlatformChanged(const QString& platform);
};

#endif
