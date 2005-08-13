/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "uib.h"

static const char commonStrings[] =
    "\0()\0(bool)\0(const QString&)\0(int)\0C++\0Layout1\0PNG\0QCheckBox\0"
    "QComboBox\0QDialog\0QFrame\0QGridLayout\0QGroupBox\0QHBoxLayout\0QLabel\0"
    "QLineEdit\0QListView\0QPushButton\0QRadioButton\0QVBoxLayout\0QWidget\0"
    "TextLabel1\0XPM.GZ\0accept\0autoDefault\0buddy\0caption\0clicked\0"
    "default\0destroy\0frameShadow\0frameShape\0geometry\0init\0margin\0"
    "maximumSize\0minimumSize\0name\0reject\0sizePolicy\0spacing\0text\0title\0"
    "toolTip\0unnamed\0whatsThis";

UibStrTable::UibStrTable()
    : out( table, IO_WriteOnly ), start( sizeof(commonStrings) )
{
    out.writeRawBytes( commonStrings, start );
}
