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
	 the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
	 Boston, MA 02110-1301, USA.
	 */

#include <kparts/part.h>
#include <kdevcore.h>

#include "subversion_part.h"
#include "subversionoptionswidgetimpl.h"
#include <qcheckbox.h>

subversionOptionsWidgetImpl::subversionOptionsWidgetImpl(QWidget *parent, subversionPart *part, const char* name)
 : subversionOptionsWidget(parent, name) {
	 m_part = part;
	 update_recurse->setChecked( m_part->m_update_recurse );
	 switch_recurse->setChecked( m_part->m_switch_recurse );
	 add_recurse->setChecked(m_part->m_add_recurse);
	 remove_force->setChecked( m_part->m_remove_force );
	 commit_recurse->setChecked ( m_part->m_commit_recurse );
	 diff_recurse->setChecked( m_part->m_diff_recurse );
	 merge_recurse->setChecked ( m_part->m_merge_recurse );
	 merge_force->setChecked( m_part->m_merge_overwrite );
	 relocate_recurse->setChecked ( m_part->m_relocate_recurse );
	 revert_recurse->setChecked(m_part->m_revert_recurse);
	 resolve_recurse->setChecked(m_part->m_resolve_recurse);
	 propset_recurse->setChecked( m_part->m_propset_recurse );
	 propget_recurse->setChecked( m_part->m_propget_recurse );
	 proplist_recurse->setChecked( m_part->m_proplist_recurse );
}

subversionOptionsWidgetImpl::~subversionOptionsWidgetImpl() {
}

void subversionOptionsWidgetImpl::accept() {
	m_part->m_update_recurse = update_recurse->isChecked();
	m_part->m_switch_recurse = switch_recurse->isChecked();
	m_part->m_add_recurse = add_recurse->isChecked();
	m_part->m_remove_force = remove_force->isChecked();
	m_part->m_commit_recurse = commit_recurse->isChecked();
	m_part->m_diff_recurse = diff_recurse->isChecked();
	m_part->m_merge_recurse = merge_recurse->isChecked();
	m_part->m_merge_overwrite = merge_force->isChecked();
	m_part->m_relocate_recurse = relocate_recurse->isChecked();
	m_part->m_revert_recurse = revert_recurse->isChecked();
	m_part->m_resolve_recurse = resolve_recurse->isChecked();
	m_part->m_propset_recurse = propset_recurse->isChecked();
	m_part->m_propget_recurse = propget_recurse->isChecked();
	m_part->m_proplist_recurse = proplist_recurse->isChecked();
}

#include "subversionoptionswidgetimpl.moc"
