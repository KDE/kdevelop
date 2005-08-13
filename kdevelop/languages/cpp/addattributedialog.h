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
*  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
*  Boston, MA 02110-1301, USA.
*
*/

#ifndef ADDATTRIBUTEDIALOG_H
#define ADDATTRIBUTEDIALOG_H

#include "addattributedialogbase.h"
#include <codemodel.h>

class CppSupportPart;

class AddAttributeDialog : public AddAttributeDialogBase
{
	Q_OBJECT
public:
	AddAttributeDialog( CppSupportPart* cppSupport, ClassDom klass,
	                    QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~AddAttributeDialog();

	virtual void addAttribute();
	virtual void deleteCurrentAttribute();
	virtual void currentChanged( QListViewItem* item );
	virtual void updateGUI();

protected:
	virtual void reject();
	virtual void accept();

private:
	QString accessID( VariableDom var ) const;
	QStringList newAccessList( const QStringList& accessList ) const;
	QString variableDeclaration( QListViewItem* item ) const;

private:
	CppSupportPart* m_cppSupport;
	ClassDom m_klass;
	int m_count;
};

#endif 
// kate: indent-mode csands; tab-width 4;


