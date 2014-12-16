/*
 * GDB Debugger Support
 *
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SELECTCOREDIALOG_H
#define SELECTCOREDIALOG_H

#include <QDialog>
#include <QUrl>

#include "ui_selectcoredialog.h"

namespace GDBDebugger {

class SelectCoreDialog : public QDialog
{
public:
    SelectCoreDialog(QWidget *parent = 0);
    QUrl binary() const;
    QUrl core() const;

private:
    Ui::SelectCoreDialog m_ui;
};

}

#endif
