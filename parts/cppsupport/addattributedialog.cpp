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
#include "tree_parser.h"
#include "ast.h"

#include <kdevpartcontroller.h>

#include <classstore.h>
#include <parsedclass.h>
#include <parsedattribute.h>

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

AddAttributeDialog::AddAttributeDialog(CppSupportPart* cppSupport, ParsedClass* klass,
				 QWidget* parent, const char* name, bool modal, WFlags fl)
    : AddAttributeDialogBase(parent,name, modal,fl), m_cppSupport( cppSupport ), m_klass( klass ), m_count( 0 )
{
    QString fileName = m_klass->declaredInFile();
    m_cppSupport->partController()->editDocument( fileName );

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

    returnType->insertStringList( m_cppSupport->classStore()->getSortedClassNameList() );
    returnType->insertStringList( m_cppSupport->classStore()->getSortedStructNameList() );

    updateGUI();
    addAttribute();
}

AddAttributeDialog::~AddAttributeDialog()
{
}

void AddAttributeDialog::reject()
{
    QDialog::reject();
}

void AddAttributeDialog::accept()
{
    QString fileName = m_klass->declaredInFile();
    kdDebug(9007) << "-------------> fileName = " << fileName << endl;

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

    {
	QString str;
	QTextStream stream( &str, IO_WriteOnly );
	QListViewItem* item = attributes->firstChild();
	stream << "\n";
	while( item ){
	    // TODO: check item
	    stream << "    " << item->text(0).lower() << ": ";
	    if( item->text(1) == "Static" )
		stream << "static ";
	    stream << item->text( 2 ) << " " << item->text( 3 );
	    stream << ";\n";
	    item = item->nextSibling();
	}


	m_cppSupport->partController()->editDocument( m_klass->declaredInFile() );
	KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( m_cppSupport->partController()->activePart() );
	if( editIface )
	    editIface->insertText( line, column, str );
    }

    QDialog::accept();
}

void AddAttributeDialog::updateGUI()
{
    bool enable = attributes->selectedItem() != 0;

    returnType->setEnabled( enable );
    declarator->setEnabled( enable );
    access->setEnabled( enable );
    storage->setEnabled( enable );

    deleteAttributeButton->setEnabled( enable );

    if( enable ){
	QListViewItem* item = attributes->selectedItem();
	item->setText( 0, access->currentText() );
	item->setText( 1, storage->currentText() );
	item->setText( 2, returnType->currentText() );
	item->setText( 3, declarator->text() );
    }
}

void AddAttributeDialog::addAttribute()
{
    QListViewItem* item = new QListViewItem( attributes, "Protected", "Normal", "int", QString("attribute_%1").arg(++m_count) );
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
    if( item ){
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

