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

#include "addmethoddialog.h"
#include "cppsupportpart.h"
#include "backgroundparser.h"
#include "cppsupport_utils.h"

#include <kdevpartcontroller.h>
#include <kdevcreatefile.h>

#include <klocale.h>
#include <kfiledialog.h>
#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <kdebug.h>

#include <qregexp.h>
#include <qfileinfo.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtextstream.h>

AddMethodDialog::AddMethodDialog( CppSupportPart* cppSupport, ClassDom klass,
                                  QWidget* parent, const char* name, bool modal, WFlags fl )
: AddMethodDialogBase( parent, name, modal, fl ), m_cppSupport( cppSupport ), m_klass( klass ), m_count( 0 )
{
	QString fileName = m_klass->fileName();
	
	access->insertStringList( QStringList() << "Public" << "Protected" << "Private" << "Signals" <<
	                          "Public Slots" << "Protected Slots" << "Private Slots" );
	
	storage->insertStringList( QStringList() << "Normal" << "Static" << "Virtual" << "Pure Virtual" << "Friend" );
	
	// setup sourceFile combo
	QMap<QString, bool> m;
#if 0 /// \FIXME ROBE
	
	FunctionList l = m_klass->functionList();
	{
		for ( FunctionList::Iterator it = l.begin(); it != l.end(); ++it )
		{
			if ( ( *it ) ->hasImplementation() )
				m.insert( ( *it ) ->implementedInFile(), true );
		}
	}
#endif
	
	{
		QStringList headers = QStringList::split( ",", "h,H,hh,hxx,hpp,inl,tlh,diff,ui.h" );
		QStringList fileList;
		QMap<QString, bool>::Iterator it = m.begin();
		while ( it != m.end() )
		{
			QString ext = QFileInfo( it.key() ).extension();
			if ( !headers.contains( ext ) )
				sourceFile->insertItem( it.key() );
			++it;
		}
		
		if ( sourceFile->count() == 0 )
		{
			QFileInfo info( fileName );
			sourceFile->insertItem( info.dirPath( true ) + "/" + info.baseName() + ".cpp" );
		}
	}
	
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
	addMethod();
}

AddMethodDialog::~AddMethodDialog()
{}

void AddMethodDialog::reject()
{
	QDialog::reject();
}

QString AddMethodDialog::accessID( FunctionDom fun ) const
{
	if ( fun->isSignal() )
		return QString::fromLatin1( "Signals" );
	
	switch ( fun->access() )
	{
	case CodeModelItem::Public:
		if ( fun->isSlot() )
			return QString::fromLatin1( "Public Slots" );
		return QString::fromLatin1( "Public" );
		
	case CodeModelItem::Protected:
		if ( fun->isSlot() )
			return QString::fromLatin1( "Protected Slots" );
		return QString::fromLatin1( "Protected" );
		
	case CodeModelItem::Private:
		if ( fun->isSlot() )
			return QString::fromLatin1( "Private Slots" );
		return QString::fromLatin1( "Private" );
	}
	
	return QString::null;
}

void AddMethodDialog::accept()
{
	m_cppSupport->partController() ->editDocument( KURL( m_klass->fileName() ) );
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
	
	const FunctionList functionList = m_klass->functionList();
	for ( FunctionList::ConstIterator it = functionList.begin(); it != functionList.end(); ++it )
	{
		int funEndLine, funEndColumn;
		( *it ) ->getEndPosition( &funEndLine, &funEndColumn );
		QString access = accessID( *it );
		QPair<int, int> funEndPoint = qMakePair( funEndLine, funEndColumn );
		
		if ( !points.contains( access ) || points[ access ] < funEndPoint )
		{
			accessList.remove( access );
			accessList.push_back( access ); // move 'access' at the end of the list
			
			points[ access ] = funEndPoint;
		}
	}
	
	int insertedLine = 0;
	
	accessList += newAccessList( accessList );
	
	for ( QStringList::iterator it = accessList.begin(); it != accessList.end(); ++it )
	{
		QListViewItem* item = methods->firstChild();
		while ( item )
		{
			QListViewItem * currentItem = item;
			
			item = item->nextSibling();
			
			if ( currentItem->text( 1 ) != *it )
				continue;
			
			QString access = ( *it ).lower();
			
			bool isInline = currentItem->text( 0 ) == "True";
			QString str = isInline ? functionDefinition( currentItem ) : functionDeclaration( currentItem );
			
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
	
	QString str;
	QListViewItem* item = methods->firstChild();
	while ( item )
	{
		QListViewItem * currentItem = item;
		
		item = item->nextSibling();
		
		QString str = functionDefinition( currentItem );
		if ( str.isEmpty() )
			continue;
		
		QString implementationFile = currentItem->text( 5 );
		if ( currentItem->text( 0 ) == "True" )
			implementationFile = m_klass->fileName();
		
		QFileInfo fileInfo( implementationFile );
		if ( !QFile::exists( fileInfo.absFilePath() ) )
		{
			if ( KDevCreateFile * createFileSupp = m_cppSupport->extension<KDevCreateFile>( "KDevelop/CreateFile" ) )
				createFileSupp->createNewFile( fileInfo.extension(), fileInfo.dirPath( true ), fileInfo.baseName() );
		}
		
		m_cppSupport->partController() ->editDocument( KURL( implementationFile ) );
		editIface = dynamic_cast<KTextEditor::EditInterface*>( m_cppSupport->partController() ->activePart() );
		if ( !editIface )
			continue;
		
		bool isInline = currentItem->text( 0 ) == "True";
		if ( !isInline )
		{
			editIface->insertLine( editIface->numLines(), QString::fromLatin1( "" ) );
			editIface->insertText( editIface->numLines() - 1, 0, str );
			m_cppSupport->backgroundParser() ->addFile( implementationFile );
		}
	}
	
	QDialog::accept();
}

void AddMethodDialog::updateGUI()
{
	bool enable = methods->selectedItem() != 0;
	
	returnType->setEnabled( enable );
	declarator->setEnabled( enable );
	access->setEnabled( enable );
	storage->setEnabled( enable );
	isInline->setEnabled( enable );
	
	sourceFile->setEnabled( enable );
	browseButton->setEnabled( enable );
	
	deleteMethodButton->setEnabled( enable );
	
	if ( enable )
	{
		QListViewItem * item = methods->selectedItem();
		item->setText( 0, isInline->isChecked() ? "True" : "False" );
		item->setText( 1, access->currentText() );
		item->setText( 2, storage->currentText() );
		item->setText( 3, returnType->currentText() );
		item->setText( 4, declarator->text() );
		item->setText( 5, sourceFile->currentText() );
		
		if ( isInline->isChecked() || storage->currentText() == "Friend" || storage->currentText() == "Pure Virtual" )
		{
			sourceFile->setEnabled( false );
			browseButton->setEnabled( false );
		}
	}
}

void AddMethodDialog::addMethod()
{
	QListViewItem * item = new QListViewItem( methods, "False", "Public", "Normal",
	                                          "void", QString( "method_%1()" ).arg( ++m_count ),
	                                          sourceFile->currentText() );
	methods->setCurrentItem( item );
	methods->setSelected( item, true );
	
	returnType->setFocus();
}

void AddMethodDialog::deleteCurrentMethod()
{
	delete( methods->currentItem() );
	updateGUI();
}

void AddMethodDialog::currentChanged( QListViewItem* item )
{
	if ( item )
	{
		QString _isInline = item->text( 0 );
		QString _access = item->text( 1 );
		QString _storage = item->text( 2 );
		QString _returnType = item->text( 3 );
		QString _declarator = item->text( 4 );
		QString _sourceFile = item->text( 5 );
		
		isInline->setChecked( _isInline == "True" ? true : false );
		access->setCurrentText( _access );
		storage->setCurrentText( _storage );
		returnType->setCurrentText( _returnType );
		declarator->setText( _declarator );
		sourceFile->setCurrentText( _sourceFile );
	}
	
	updateGUI();
}

void AddMethodDialog::browseImplementationFile()
{
	QString fileName = KFileDialog::getOpenFileName();
	sourceFile->setCurrentText( fileName );
	updateGUI();
}

QString AddMethodDialog::functionDeclaration( QListViewItem * item ) const
{
	QString str;
	QTextStream stream( &str, IO_WriteOnly );
	
	QString access = item->text( 1 ).lower();
	
	stream << "    "; /// @todo use AStyle
	if ( item->text( 2 ) == "Virtual" || item->text( 2 ) == "Pure Virtual" )
		stream << "virtual ";
	else if ( item->text( 2 ) == "Friend" )
		stream << "friend ";
	else if ( item->text( 2 ) == "Static" )
		stream << "static ";
	stream << item->text( 3 ) << " " << item->text( 4 );
	if ( item->text( 2 ) == "Pure Virtual" )
		stream << " = 0";
	stream << ";\n";
	
	return str;
}

QString AddMethodDialog::functionDefinition( QListViewItem* item ) const
{
	if ( item->text( 1 ) == "Signals" || item->text( 2 ) == "Pure Virtual" ||
	     item->text( 2 ) == "Friend" )
	{
		return QString::null;
	}
	
	QString className = m_klass->name();
	QString fullName = m_klass->scope().join( "::" );
	if ( !fullName.isEmpty() )
		fullName += "::";
	fullName += className;
	
	QString str;
	QTextStream stream( &str, IO_WriteOnly );
	
	bool isInline = item->text( 0 ) == "True";
	
	QString ind;
	if ( isInline )
		ind.fill( QChar( ' ' ), 4 );
	
	stream << "\n"
		<< ind << "/*!\n"
		<< ind << "    \\fn " << fullName << "::" << item->text( 4 ) << "\n"
		<< ind << " */\n";
	
	stream
		<< ind << item->text( 3 ) << " " << ( isInline ? QString::fromLatin1( "" ) : fullName + "::" )
		<< item->text( 4 ) << "\n"
		<< ind << "{\n"
		<< ind << "    /// @todo implement me\n"
		<< ind << "}\n";
	
	return str;
}

QStringList AddMethodDialog::newAccessList( const QStringList& accessList ) const
{
	QStringList newAccessList;
	
	QListViewItem* item = methods->firstChild();
	while ( item )
	{
		QListViewItem * currentItem = item;
		
		item = item->nextSibling();
		
		QString access = currentItem->text( 1 );
		if ( !( accessList.contains( access ) || newAccessList.contains( access ) ) )
			newAccessList.push_back( access );
	}
	
	return newAccessList;
}

#include "addmethoddialog.moc" 
//kate: indent-mode csands; tab-width 4; space-indent off;

