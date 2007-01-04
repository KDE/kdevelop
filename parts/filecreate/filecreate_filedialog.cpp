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

#include <qfileinfo.h>
#include <kdeversion.h>
#include <kurlcombobox.h>
#include <kdebug.h>

#include "filecreate_part.h"
#include "filecreate_filetype.h"

namespace FileCreate {

  FileDialog::FileDialog(const QString& startDir, const QString& filter,
                         QWidget *parent, const char *name,
                         bool modal, QWidget * extraWidget) :
    KFileDialog(startDir, filter, parent, name, modal, extraWidget) {

    setOperationMode(Saving);

    m_extraWidget = extraWidget;
    m_typeChooser = dynamic_cast<TypeChooser*>(extraWidget);

    connect(this, SIGNAL(filterChanged(const QString &)), this, SLOT(slotActionFilterChanged(const QString &)) );
    connect(locationEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotActionTextChanged(const QString &)) );

  }

  FileDialog::~FileDialog() { }


  void FileDialog::initGUI()
  {
    KFileDialog::initGUI();
  }

  void FileDialog::slotActionFilterChanged(const QString & /*filter*/) {
  }

  void FileDialog::slotActionTextChanged(const QString & text) {
    kdDebug(9034) << "slotActionTextChanged - " << text << endl;
    if (!m_typeChooser) return;
    QString ext = QFileInfo(text).extension();
    kdDebug(9034) << "Extension is: " << ext << endl;
    FileType * filetype = m_typeChooser->part()->getType(ext);
    if (!filetype) return;
    kdDebug(9034) << "found matching extension: " << ext << endl;
    kdDebug(9034) << "Description: " << filetype->descr() << endl;
    m_typeChooser->setCurrent(filetype);
  }



}

#include "filecreate_filedialog.moc"
