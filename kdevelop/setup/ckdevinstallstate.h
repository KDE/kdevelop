/***************************************************************************
                          ckdevinstallstate.h  -  description
                             -------------------
    begin                : Sun Jun 10 2001
    copyright            : (C) 2001 by Falk Brettschneider
    email                : falk.brettschneider@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>

class KShellProcess;
class QDir;

#ifndef CKDEVINSTALLSTATE_H
#define CKDEVINSTALLSTATE_H

class CKDevInstallState
{
public:
	CKDevInstallState(const QString& qtDocPath, const QString& kdeDocPath);
	int highlightStyle;	// 0 = new KDevelop-2.0 style, 1 = Emacs, 2 = KWrite

  bool qt_test;
  bool kde_test;
  bool successful;

  bool glimpse;
  bool glimpseindex;
  bool htdig;
  bool htsearch;
  bool finished_glimpse;

  bool make;
  bool gmake;

  QString qt;
  QString kde;

	bool install;
	QDir* finish_dir;
	
	QString searchEngine;
	int userInterfaceMode;
	
  KShellProcess* shell_process;
};

#endif  // CKDEVINSTALLSTATE_H
