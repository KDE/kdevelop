/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef ADDFILESDIALOG_H
#define ADDFILESDIALOG_H

#include <kdeversion.h>
#include <kfiledialog.h>

class QComboBox;

/**
This class allows you to choose additional modes when adding
files to the project.
Currently available modes are Copy, Link and Relative.
Relative means that file should be not copied but added
with the relative path (e.g. ../../dir/filename)
*/
class AddFilesDialog : public KFileDialog
{
    Q_OBJECT
public:
    enum Mode { Copy, Link, Relative };

    AddFilesDialog(const QString& startDir, const QString& filter,
               QWidget *parent, const char *name, bool modal);
#if KDE_VERSION >= 310
    AddFilesDialog(const QString& startDir, const QString& filter,
               QWidget *parent, const char *name, bool modal, QComboBox *extraWidget);
#endif
    virtual ~AddFilesDialog();

    virtual AddFilesDialog::Mode mode();

private:
    QComboBox * m_extraWidget;

private slots:
    void storePreferred(int index);
};

#endif
