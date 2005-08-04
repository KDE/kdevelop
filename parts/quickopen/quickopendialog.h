/*
 *  Copyright (C) KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2004
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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef QUICKOPENDIALOG_H
#define QUICKOPENDIALOG_H

#include "quickopenbase.h"
//Added by qt3to4:
#include <QEvent>

class QuickOpenPart;
class KCompletion;

class QuickOpenDialog : public QuickOpenDialogBase
{
  Q_OBJECT

public:
  QuickOpenDialog(QuickOpenPart* part, QWidget* parent = 0, const char* name = 0, bool modal = TRUE, Qt::WFlags fl = 0 );
  virtual ~QuickOpenDialog();

  virtual bool eventFilter(QObject *watched, QEvent *e);
  
public slots:
  virtual void slotTextChanged(const QString&);

protected:
	KCompletion* m_completion;
	QuickOpenPart* m_part;

};

#endif

