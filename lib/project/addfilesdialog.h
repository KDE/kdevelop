/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
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
