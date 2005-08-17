/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef QUICKOPENFILEDIALOG_H
#define QUICKOPENFILEDIALOG_H

#include "quickopendialog.h"
#include <qstringlist.h>

class QuickOpenPart;
class KCompletion;

class QuickOpenFileDialog : public QuickOpenDialog
{
    Q_OBJECT
public:
    QuickOpenFileDialog(QuickOpenPart* part, QWidget* parent = 0, const char* name = 0, bool modal = TRUE, Qt::WFlags fl = 0 );
    virtual ~QuickOpenFileDialog();

public slots:
    virtual void slotExecuted( Q3ListBoxItem* );
    virtual void slotReturnPressed();

private:
    QStringList m_fileList;
};

#endif

