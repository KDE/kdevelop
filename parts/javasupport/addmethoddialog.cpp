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
#include "javasupportpart.h"
#include "backgroundparser.h"
#include "JavaAST.hpp"

#include <kdevpartcontroller.h>

#include <classstore.h>
#include <parsedclass.h>
#include <parsedmethod.h>

#include <kfiledialog.h>
#include <kparts/part.h>
#include <ktexteditor/editinterface.h>

#include <qregexp.h>
#include <qfileinfo.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtextstream.h>

#if 0
namespace AddMethod
{

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
	//kdDebug(9007) << "----------------------------> current scope is " << m_scope.join(".") << endl;

	if( m_scope.join(".") == m_className ){
	    //kdDebug(9007) << "----------------------------> found class" << endl;
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
	    //kdDebug(9007) << "------------------> found insertion point" << endl;
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

}
#endif

AddMethodDialog::AddMethodDialog(JavaSupportPart* javaSupport, ParsedClass* klass,
				 QWidget* parent, const char* name, bool modal, WFlags fl)
    : AddMethodDialogBase(parent,name, modal,fl), m_javaSupport( javaSupport ), m_klass( klass ), m_count( 0 )
{
    QString fileName = m_klass->declaredInFile();
    m_javaSupport->partController()->editDocument( fileName );

    // setup sourceFile combo
    QValueList<ParsedMethod*> l = m_klass->getSortedMethodList();
    l += m_klass->getSortedSignalList();
    l += m_klass->getSortedSlotList();
    QMap<QString, bool> m;
    {
	QValueList<ParsedMethod*>::Iterator it = l.begin();
	while( it != l.end() ){
	    m.insert( (*it)->definedInFile(), true );
	    ++it;
	}
    }

    {
        QStringList headers = QStringList::split( ",", "h,H,hh,hxx,hpp,inl,tlh,diff,ui.h" );
        QStringList fileList;
	QMap<QString, bool>::Iterator it = m.begin();
	while( it != m.end() ){
            QString ext = QFileInfo(it.key()).extension();
            if( !headers.contains(ext) )
                sourceFile->insertItem( it.key() );
	    ++it;
	}

        if( sourceFile->count() == 0 ){
            QFileInfo info( fileName );
            sourceFile->insertItem( info.dirPath(true) + "/" + info.baseName() + ".java" );
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

    returnType->insertStringList( m_javaSupport->classStore()->getSortedClassNameList() );
    returnType->insertStringList( m_javaSupport->classStore()->getSortedStructNameList() );

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
    //kdDebug(9007) << "-------------> fileName = " << fileName << endl;

    // sync
    while( m_javaSupport->backgroundParser()->filesInQueue() > 0 )
	m_javaSupport->backgroundParser()->isEmpty().wait();

    m_javaSupport->backgroundParser()->lock();

    int line = 0, column = 0;

    RefJavaAST translationUnit = m_javaSupport->backgroundParser()->translationUnit( fileName );
    if( translationUnit ){
/*	AddMethod::FindInsertionPoint findInsertionPoint( m_klass->path() );
	findInsertionPoint.parseTranslationUnit( translationUnit );
	line = findInsertionPoint.line();
	column = findInsertionPoint.column();*/
    }
    m_javaSupport->backgroundParser()->unlock();

    {
	QString str;
	QTextStream stream( &str, IO_WriteOnly );
	QListViewItem* item = methods->firstChild();
	stream << "\n";
	while( item ){
	    /// @todo check item
	    stream << "    " << item->text(1).lower() << ": ";
	    if( item->text(2) == "Virtual" || storage->currentText() == "Pure Virtual" )
		stream << "virtual ";
	    else if( item->text(2) == "Friend" )
		stream << "friend ";
	    else if( item->text(2) == "Static" )
		stream << "static ";
	    stream << item->text( 3 ) << " " << item->text( 4 );
	    if( item->text(2) == "Pure Virtual" )
		stream << " = 0";
	    stream << ";\n";
	    item = item->nextSibling();
	}


	m_javaSupport->partController()->editDocument( m_klass->declaredInFile() );
	KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( m_javaSupport->partController()->activePart() );
	if( editIface )
	    editIface->insertText( line, column, str );
    }

    {
	    QListViewItem* item = methods->firstChild();
#if QT_VERSION >= 0x030100
	    QString className = m_klass->path().replace( QString("."), QString("::") );
#else
	    QString className = m_klass->path().replace( QRegExp("\\."), QString("::") );
#endif
	    while( item ){
		if( item->text(2) == "Friend" || item->text(2) == "Pure Virtual" ){
		    item = item->nextSibling();
                    continue;
		}

		QString implementationFile = item->text( 5 );
		if( item->text(0) == "True" )
		    implementationFile = m_klass->declaredInFile();

		m_javaSupport->partController()->editDocument( implementationFile );
                m_javaSupport->backgroundParser()->addFile( implementationFile ); // reparse

		KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( m_javaSupport->partController()->activePart() );
		if( !editIface ){
		    /// @todo report error
		    item = item->nextSibling();
		    continue;
		}

		// sync
		while( m_javaSupport->backgroundParser()->filesInQueue() > 0 )
		    m_javaSupport->backgroundParser()->isEmpty().wait();

		int line = editIface->numLines() - 1, column = 0;

                m_javaSupport->backgroundParser()->lock();
		RefJavaAST translationUnit = m_javaSupport->backgroundParser()->translationUnit( implementationFile );
		//kdDebug(9007) << "-----------> unit = " << translationUnit << endl;
		if( translationUnit ){
		    // translationUnit->getEndPosition( &line, &column );
                    //kdDebug(9007) << "------> line = " << line << " column = " << column << endl;
                }
		m_javaSupport->backgroundParser()->unlock();

		QString str;
		QTextStream stream( &str, IO_WriteOnly );
		stream << "\n\n"
                    << "/*!\n"
		    << "    \\fn " << className << "::" << item->text( 4 ) << "\n"
                    << " */\n";

                if( item->text(0) == "True" )
                    stream << "inline ";

                stream
		    << item->text( 3 ) << " " << className << "::" << item->text( 4 ) << "\n{\n"
                    << "#warning \"not implemented yet!!\"\n"
                    << "}\n";
		editIface->insertText( line, column, str );

		item = item->nextSibling();
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

    if( enable ){
	QListViewItem* item = methods->selectedItem();
	item->setText( 0, isInline->isChecked() ? "True" : "False" );
	item->setText( 1, access->currentText() );
	item->setText( 2, storage->currentText() );
	item->setText( 3, returnType->currentText() );
	item->setText( 4, declarator->text() );
	item->setText( 5, sourceFile->currentText() );

	if( isInline->isChecked() || storage->currentText() == "Friend" || storage->currentText() == "Pure Virtual" ){
	    sourceFile->setEnabled( false );
	    browseButton->setEnabled( false );
	}
    }
}

void AddMethodDialog::addMethod()
{
    QListViewItem* item = new QListViewItem( methods, "False", "Public", "Normal", "void", QString("method_%1()").arg(++m_count),
    		sourceFile->currentText() );
    methods->setCurrentItem( item );
    methods->setSelected( item, true );

    returnType->setFocus();
}

void AddMethodDialog::deleteCurrentMethod()
{
    delete( methods->currentItem() );
}

void AddMethodDialog::currentChanged( QListViewItem* item )
{
    if( item ){
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
#include "addmethoddialog.moc"
