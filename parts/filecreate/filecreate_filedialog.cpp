/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "filecreate_filedialog.h"

#include <kdeversion.h>

namespace FileCreate {

  FileDialog::FileDialog(const QString& startDir, const QString& filter,
                         QWidget *parent, const char *name,
                         bool modal, QWidget * extraWidget) :
#if KDE_VERSION >= 310
    KFileDialog(startDir, filter, parent, name, modal, extraWidget) {
#else
    KFileDialog(startDir, filter, parent, name, modal) {
      setPreviewWidget(extraWidget);
#endif

  }

  FileDialog::~FileDialog() { } 


  void FileDialog::initGUI()
  {
    KFileDialog::initGUI();
  }
  
}
