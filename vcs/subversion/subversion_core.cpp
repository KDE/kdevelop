/* Copyright (C) 2003
	 Mickael Marchand <marchand@kde.org>

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
	 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	 Boston, MA 02111-1307, USA.
	 */

#include <kparts/part.h>
#include <kdevcore.h>

#include "subversion_part.h"
#include "subversion_core.h"
#include "subversion_widget.h"
#include <kdevmainwindow.h>

subversionCore::subversionCore(subversionPart *part)
 : QObject(this, "subversion core") {
	m_part = part;
	m_widget = new subversionWidget(part, 0 , "processwidget");
//	connect( m_part->core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
}

subversionCore::~subversionCore() {
	if ( processWidget() ) {
		(( KDevMainWindow *) m_part->mainWindow() )->removeView( m_widget );
		delete m_widget;
	}
}

QWidget *subversionCore::processWidget() {
	return m_widget;
}

#include "subversion_core.moc"
