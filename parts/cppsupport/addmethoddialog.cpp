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

#include "addmethoddialog.h"
#include "cppsupportpart.h"
#include "backgroundparser.h"
#include "tree_parser.h"
#include "ast.h"

#include <kdevpartcontroller.h>

#include <classstore.h>
#include <parsedclass.h>
#include <parsedmethod.h>

#include <kparts/part.h>
#include <ktexteditor/editinterface.h>

#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtextstream.h>

class FindInsertionPoint: public TreeParser
{
public:
    FindInsertionPoint( const QString& className )
	: TreeParser(), m_line(0), m_column(0) 
    {
	m_className = className.stripWhiteSpace();	
    }
    
    void parseTranslationUnit( TranslationUnitAST* ast )
    {
	m_line = 0;
	m_column = 0;
	m_scope.clear();
	TreeParser::parseTranslationUnit( ast );
    }
    
    void parseNamespace( NamespaceAST* ast )
    {
	QString name = ast->namespaceName() ? ast->namespaceName()->text() : QString::null; // hmm
	name = name.stripWhiteSpace();
	
	m_scope.push_back( name );
	TreeParser::parseNamespace( ast );
	m_scope.pop_back();	
    }
    
    void parseClassSpecifier( ClassSpecifierAST* ast )
    {
	QString name = ast->name() ? ast->name()->text() : QString::null; // hmm
	name = name.stripWhiteSpace();
	
	m_scope.push_back( name );
	kdDebug(9007) << "----------------------------> current scope is " << m_scope.join(".") << endl;
	
	if( m_scope.join(".") == m_className ){
	    kdDebug(9007) << "----------------------------> found class" << endl;
	    ast->getEndPosition( &m_line, &m_column );
	}
	
	TreeParser::parseClassSpecifier( ast );
		
	m_scope.pop_back();
    }
    
    void parseSimpleDeclaration( SimpleDeclarationAST* ast )
    {
	TypeSpecifierAST* typeSpec = ast->typeSpec();
	
	if( typeSpec )
	    parseTypeSpecifier( typeSpec );
		
	TreeParser::parseSimpleDeclaration( ast );
    }
    
    void parseAccessDeclaration( AccessDeclarationAST* ast )
    {
	if( m_scope.join(".") == m_className ){
	    kdDebug(9007) << "------------------> found insertion point" << endl;
	}
	
	TreeParser::parseAccessDeclaration( ast );
    }
    
    int line() const { return m_line; }
    int column() const { return m_column-1; }
    
private:
    QString m_className;
    QStringList m_scope;
    int m_line;
    int m_column;
};

AddMethodDialog::AddMethodDialog(CppSupportPart* cppSupport, ParsedClass* klass,
				 QWidget* parent, const char* name, bool modal, WFlags fl)
    : AddMethodDialogBase(parent,name, modal,fl), m_cppSupport( cppSupport ), m_klass( klass ), m_count( 0 )
{
    QString fileName = m_klass->declaredInFile();
    m_cppSupport->partController()->editDocument( fileName );
    
    updateGUI();
    addMethod();
}

AddMethodDialog::~AddMethodDialog()
{
}

void AddMethodDialog::reject()
{
    QDialog::reject();
}

void AddMethodDialog::accept()
{
    QString fileName = m_klass->declaredInFile();
    kdDebug(9007) << "-------------> fileName = " << fileName << endl;
    
    m_cppSupport->backgroundParser()->addFile( fileName );
        
    // sync
    while( m_cppSupport->backgroundParser()->filesInQueue() > 0 )
	m_cppSupport->backgroundParser()->isEmpty().wait();
    
    m_cppSupport->backgroundParser()->lock();
 
    int line = 0, column = 0;
    
    TranslationUnitAST* translationUnit = m_cppSupport->backgroundParser()->translationUnit( fileName );	
    if( translationUnit ){
	FindInsertionPoint findInsertionPoint( m_klass->path() );
	findInsertionPoint.parseTranslationUnit( translationUnit );
	line = findInsertionPoint.line();
	column = findInsertionPoint.column();
    }    
    m_cppSupport->backgroundParser()->unlock();
    m_cppSupport->backgroundParser()->removeFile( fileName );
            
    QString str;
    QTextStream stream( &str, IO_WriteOnly );
    QListViewItem* item = methods->firstChild();
    stream << "\n";
    while( item ){
	// TODO: check item
	stream << "    " << item->text(1).lower() << ": ";
	if( storage->currentText() == "Virtual" || storage->currentText() == "Pure Virtual" )
	    stream << "virtual ";
	else if( storage->currentText() == "Friend" )
	    stream << "friend ";
	else if( storage->currentText() == "Static" )
	    stream << "static ";
	stream << item->text( 3 ) << " " << item->text( 4 );
	if( storage->currentText() == "Pure Virtual" )
	    stream << " = 0";
	stream << ";\n";
	item = item->nextSibling();
    }
    KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( m_cppSupport->partController()->activePart() );
    if( editIface )
	editIface->insertText( line, column, str );
    
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
    
    if( enable ){
	QListViewItem* item = methods->selectedItem();
	item->setText( 0, isInline->isChecked() ? "True" : "False" );
	item->setText( 1, access->currentText() );
	item->setText( 2, storage->currentText() );
	item->setText( 3, returnType->currentText() );
	item->setText( 4, declarator->text() );
	item->setText( 5, sourceFile->currentText() );
	
	if( isInline->isChecked() || storage->currentText() == "Friend" || storage->currentText() == "Pure Virtual" )
	    sourceFile->setEnabled( false );
    }
}

void AddMethodDialog::addMethod()
{
    QListViewItem* item = new QListViewItem( methods );
    methods->setCurrentItem( item );
    methods->setSelected( item, true );
    
    // default values
    isInline->setChecked( false );
    access->setCurrentText( "Public" );
    storage->setCurrentText( "Normal" );
    returnType->setCurrentText( "void" );
    declarator->setText( QString("method_%1()").arg(++m_count) );
    sourceFile->setCurrentText( "" );
    
    returnType->setFocus();
}

void AddMethodDialog::deleteCurrentMethod()
{
    delete( methods->currentItem() );
}

void AddMethodDialog::currentChanged( QListViewItem* item )
{
    if( item ){	
	isInline->setChecked( item->text(0) == "True" ? true : false );
	access->setCurrentText( item->text(1) );
	storage->setCurrentText( item->text(2) );
	returnType->setCurrentText( item->text(3) );
	declarator->setText( item->text(4) );
	sourceFile->setCurrentText( item->text(5) );
    }
    
    updateGUI();
}


