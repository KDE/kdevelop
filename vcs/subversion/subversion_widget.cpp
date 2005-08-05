/**
	 Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>

	 This program is free software; you can redistribute it and/or
	 modify it under the terms of the GNU General Public
	 License as published by the Free Software Foundation; either
	 version 2 of the License, or (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program; see the file COPYING.  If not, write to
	 the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
	 Boston, MA 02110-1301, USA.
	 */

#include <kparts/part.h>
#include <kdevcore.h>

#include "subversion_part.h"
#include "subversion_widget.h"

subversionWidget::subversionWidget(subversionPart *part, QWidget *parent, const char* name)
 : QTextEdit(parent, name)
{
	m_part = part;
	setReadOnly( true );
#if QT_VERSION >= 0x030100
	setTextFormat( Qt::LogText );
#endif
}

subversionWidget::~subversionWidget()
{
}

#include "subversion_widget.moc"
