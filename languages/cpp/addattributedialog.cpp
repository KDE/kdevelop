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

#include "addattributedialog.h"
#include "cppsupportpart.h"
#include "backgroundparser.h"
#include "cppsupport_utils.h"

#include <kdevpartcontroller.h>

#include <codemodel.h>

#include <kfiledialog.h>
#include <kparts/part.h>
#include <ktexteditor/editinterface.h>

#include <qfileinfo.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtextstream.h>

AddAttributeDialog::AddAttributeDialog( CppSupportPart* cppSupport, ClassDom klass,
                                        QWidget* parent, const char* name, bool modal, WFlags fl )
: AddAttributeDialogBase( parent, name, modal, fl ), m_cppSupport( cppSupport ), m_klass( klass ), m_count( 0 )
{
	access->insertStringList( QStringList() << "Public" << "Protected" << "Private" );
	
	storage->insertStringList( QStringList() << "Normal" << "Static" );
	
	returnType->setAutoCompletion( true );
	returnType->insertStringList( QStringList()
	                              << "void"
	                              << "char"
	                              << "wchar_t"
	                              << "bool"
	                              << "short"
	                              << "int"
	                              << "long"
	                              << "signed"
	                              << "unsigned"
	                              << "float"
	                              << "double" );
	
	returnType->insertStringList( typeNameList( m_cppSupport->codeModel() ) );
	
	updateGUI();
	addAttribute();
}

AddAttributeDialog::~AddAttributeDialog()
{}

void AddAttributeDialog::reject()
{
	QDialog::reject();
}

void AddAttributeDialog::accept()
{
	m_cppSupport->partController()->editDocument( KURL( m_klass->fileName() ) );
	KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( m_cppSupport->partController() ->activePart() );
	if ( !editIface )
	{
		/// @todo show messagebox
		QDialog::accept();
		return ;
	}
	
	int line, column;
	m_klass->getEndPosition( &line, &column );
	
	// compute the insertion point map
	QMap<QString, QPair<int, int> > points;
	QStringList accessList;
	
	const VariableList variableList = m_klass->variableList();
	for ( VariableList::ConstIterator it = variableList.begin(); it != variableList.end(); ++it )
	{
		int varEndLine, varEndColumn;
		( *it ) ->getEndPosition( &varEndLine, &varEndColumn );
		QString access = accessID( *it );
		QPair<int, int> varEndPoint = qMakePair( varEndLine, varEndColumn );
		
		if ( !points.contains( access ) || points[ access ] < varEndPoint )
		{
			accessList.remove( access );
			accessList.push_back( access ); // move 'access' at the end of the list
			
			points[ access ] = varEndPoint;
		}
	}
	
	int insertedLine = 0;
	
	accessList += newAccessList( accessList );
	
	for ( QStringList::iterator it = accessList.begin(); it != accessList.end(); ++it )
	{
		QListViewItem* item = attributes->firstChild();
		while ( item )
		{
			QListViewItem * currentItem = item;
			
			item = item->nextSibling();
			
			if ( currentItem->text( 0 ) != *it )
				continue;
			
			QString access = ( *it ).lower();
			
			QString str = variableDeclaration( currentItem );
			
			QPair<int, int> pt;
			if ( points.contains( *it ) )
			{
				pt = points[ *it ];
			}
			else
			{
				str.prepend( access + ":\n" );
				points[ *it ] = qMakePair( line - 1, 0 );
				pt = points[ *it ]; // end of class declaration
			}
			
			editIface->insertText( pt.first + insertedLine + 1, 0 /*pt.second*/, str );
			insertedLine += str.contains( QChar( '\n' ) );
		}
	}
	
	m_cppSupport->backgroundParser() ->addFile( m_klass->fileName() );
	
	QDialog::accept();
}

QString AddAttributeDialog::variableDeclaration( QListViewItem* item ) const
{
	QString str;
	QTextStream stream( &str, IO_WriteOnly );
	QString ind;
	ind.fill( QChar( ' ' ), 4 );
	
	stream << ind;
	if ( item->text( 1 ) == "Static" )
		stream << "static ";
	stream << item->text( 2 ) << " " << item->text( 3 );
	stream << ";\n";
	
	return str;
}


void AddAttributeDialog::updateGUI()
{
	bool enable = attributes->selectedItem() != 0;
	
	returnType->setEnabled( enable );
	declarator->setEnabled( enable );
	access->setEnabled( enable );
	storage->setEnabled( enable );
	
	deleteAttributeButton->setEnabled( enable );
	
	if ( enable )
	{
		QListViewItem * item = attributes->selectedItem();
		item->setText( 0, access->currentText() );
		item->setText( 1, storage->currentText() );
		item->setText( 2, returnType->currentText() );
		item->setText( 3, declarator->text() );
	}
}

void AddAttributeDialog::addAttribute()
{
	QListViewItem * item = new QListViewItem( attributes, "Protected", "Normal",
	                                          "int", QString( "attribute_%1" ).arg( ++m_count ) );
	attributes->setCurrentItem( item );
	attributes->setSelected( item, true );
	
	returnType->setFocus();
}

void AddAttributeDialog::deleteCurrentAttribute()
{
	delete( attributes->currentItem() );
}

void AddAttributeDialog::currentChanged( QListViewItem* item )
{
	if ( item )
	{
		QString _access = item->text( 0 );
		QString _storage = item->text( 1 );
		QString _returnType = item->text( 2 );
		QString _declarator = item->text( 3 );
		
		access->setCurrentText( _access );
		storage->setCurrentText( _storage );
		returnType->setCurrentText( _returnType );
		declarator->setText( _declarator );
	}
	
	updateGUI();
}

QStringList AddAttributeDialog::newAccessList( const QStringList& accessList ) const
{
	QStringList newAccessList;
	
	QListViewItem* item = attributes->firstChild();
	while ( item )
	{
		QListViewItem * currentItem = item;
		
		item = item->nextSibling();
		
		QString access = currentItem->text( 0 );
		if ( !( accessList.contains( access ) || newAccessList.contains( access ) ) )
			newAccessList.push_back( access );
	}
	
	return newAccessList;
}

QString AddAttributeDialog::accessID( VariableDom var ) const
{
	switch ( var->access() )
	{
	case CodeModelItem::Public:
		return QString::fromLatin1( "Public" );
		
	case CodeModelItem::Protected:
		return QString::fromLatin1( "Protected" );
		
	case CodeModelItem::Private:
		return QString::fromLatin1( "Private" );
	}
	
	return QString::null;
}

#include "addattributedialog.moc" 
//kate: indent-mode csands; tab-width 4; space-indent off;

