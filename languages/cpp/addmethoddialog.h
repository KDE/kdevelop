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

#ifndef ADDMETHODDIALOG_H
#define ADDMETHODDIALOG_H

#include "addmethoddialogbase.h"
#include <codemodel.h>

class CppSupportPart;

class AddMethodDialog : public AddMethodDialogBase
{
	Q_OBJECT
public:
	AddMethodDialog( CppSupportPart* cppSupport, ClassDom klass,
	                 QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~AddMethodDialog();

	virtual void addMethod();
	virtual void deleteCurrentMethod();
	virtual void currentChanged( QListViewItem* item );
	virtual void updateGUI();
	virtual void browseImplementationFile();

protected:
	virtual void reject();
	virtual void accept();

private:
	QString accessID( FunctionDom fun ) const;
	QString functionDeclaration( QListViewItem* item ) const;
	QString functionDefinition( QListViewItem* item ) const;
	QStringList newAccessList( const QStringList& accessList ) const;

private:
	CppSupportPart* m_cppSupport;
	ClassDom m_klass;
	int m_count;
};

#endif 
// kate: indent-mode csands; tab-width 4;


