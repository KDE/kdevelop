/* Copyright (C) 2002 
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

#include "domutil.h"
#include "svnoptionswidget.h"
#include "svn_part.h"
#include <qcheckbox.h>

	svnOptionsWidget::svnOptionsWidget(SvnPart *part, QWidget *parent, const char *name)
: svnoptions(parent, name) 
{
	m_part = part;
	readConfig();
}

svnOptionsWidget::~svnOptionsWidget() {}

void svnOptionsWidget::readConfig() {
	QDomDocument &dom = *m_part->projectDom();

	force->setChecked(DomUtil::readBoolEntry(dom,"/kdevsvn/force",m_part->force));
	recurse->setChecked(DomUtil::readBoolEntry(dom,"/kdevsvn/recurse",m_part->recursive));
	verbose->setChecked(DomUtil::readBoolEntry(dom,"/kdevsvn/verbose",m_part->verbose));
}

void svnOptionsWidget::storeConfig() {
	QDomDocument &dom = *m_part->projectDom();

	DomUtil::writeBoolEntry(dom, "/kdevsvn/force", force->isChecked());
	DomUtil::writeBoolEntry(dom, "/kdevsvn/recurse", recurse->isChecked());
	DomUtil::writeBoolEntry(dom, "/kdevsvn/verbose", verbose->isChecked());
}

void svnOptionsWidget::accept() {
	storeConfig();
	m_part->readConf();
}
/* vim: set ai ts=8 sw=8 : */
#include "svnoptionswidget.moc"
