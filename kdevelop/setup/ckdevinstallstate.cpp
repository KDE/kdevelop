/***************************************************************************
                          ckdevinstallstate.cpp  -  description
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

#include <kprocess.h>

#include "ckdevinstallstate.h"

CKDevInstallState::CKDevInstallState(const QString& qtDocuPath, const QString& kdeDocuPath)
 : highlightStyle(0)	// emacs
  ,qt_test(false)
  ,kde_test(false)
  ,successful(false)

  ,glimpse(false)
  ,glimpseindex(false)
  ,htdig(false)
  ,htsearch(false)
  ,finished_glimpse(false)

  ,make(false)
  ,gmake(false)

  ,install(true)	// we are installing at the moment
  ,finish_dir(0L)
  ,userInterfaceMode(1)	// childframe mode is default
{
  shell_process = new KShellProcess();
  qt = qtDocuPath;
  kde = kdeDocuPath;
}
