/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@mksat.net                                                       *
 *   Portions Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "qtdesignercppintegration.h"

#include <qpair.h>
#include <qregexp.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/view.h>
#include <ktexteditor/viewcursorinterface.h>

#include <domutil.h>
#include <kdevpartcontroller.h>
#include <kdevcreatefile.h>

#include "backgroundparser.h"
#include "cppsupportpart.h"
#include "codemodel_utils.h"
#include "implementationwidget.h"

QtDesignerCppIntegration::QtDesignerCppIntegration(KDevLanguageSupport *part,
    ImplementationWidget *impl)
    :QtDesignerIntegration(part, impl, true, 0)
{
}

void QtDesignerCppIntegration::addFunctionToClass(KInterfaceDesigner::Function function, ClassDom klass)
{
    m_part->partController()->editDocument( KURL( klass->fileName() ) );
    KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( m_part->partController()->activePart() );
    if( !editIface ){
        /// @todo show messagebox
//        QDialog::accept();
        return;
    }

    int line, column;
    klass->getEndPosition( &line, &column );

    // compute the insertion point map
    QMap<QString, QPair<int,int> > points;

    const FunctionList functionList = klass->functionList();
    for( FunctionList::ConstIterator it=functionList.begin(); it!=functionList.end(); ++it )
    {
        int funEndLine, funEndColumn;
        (*it)->getEndPosition( &funEndLine, &funEndColumn );
        QString access = accessID( *it );
        QPair<int, int> funEndPoint = qMakePair( funEndLine, funEndColumn );

        if( !points.contains(access) || points[access] < funEndPoint ){
            points[ access ] = funEndPoint;
        }
    }

    int insertedLine = 0;

    QString access = function.access + ( function.type == KInterfaceDesigner::ftQtSlot ? " slots" : "" );

    QString str = function.returnType + " " + function.function;
    if (function.specifier == "virtual")
        str = "virtual " + str;
    else if (function.specifier == "pure virtual")
        str = "virtual " + str + " = 0";
    else if (function.specifier == "static")
        str = "static " + str;
    str += ";\n";
    str = "    " + str;

    QPair<int, int> pt;
    if( points.contains(access) ) {
        pt = points[access];
    } else {
        str.prepend( access + ":\n" );
        points[access] = qMakePair( line-1, 0 );
        pt = points[access]; // end of class declaration
    }

    editIface->insertText( pt.first + insertedLine + 1, 0 /*pt.second*/, str );
    insertedLine += str.contains( QChar('\n') );

    CppSupportPart *cppPart = dynamic_cast<CppSupportPart *>(m_part);
    cppPart->backgroundParser()->addFile( klass->fileName() );

    if (function.specifier == "pure virtual")
        return;

    
    //implementation
    QString stri = function.returnType + " " + klass->name() + "::" + function.function;
    if (function.specifier == "static")
        stri = "static " + stri;
    stri += "\n{\n}\n";
    stri = "\n" + stri;
        
    QFileInfo fi(klass->fileName());
    QString implementationFile = fi.absFilePath();
    implementationFile.replace(".h", ".cpp");

    QFileInfo fileInfo( implementationFile );
    if( !QFile::exists(fileInfo.absFilePath()) ){
        m_part->createFileSupport()->createNewFile( fileInfo.extension(), fileInfo.dirPath(true), fileInfo.fileName() );
    }

    m_part->partController()->editDocument( KURL( implementationFile ) );
    editIface = dynamic_cast<KTextEditor::EditInterface*>( m_part->partController()->activePart() );
    if( !editIface )
        return;

    int atLine = 0, atColumn = 0;
    TranslationUnitAST *translationUnit = cppPart->backgroundParser()->translationUnit(implementationFile);
    if (translationUnit){
        translationUnit->getEndPosition( &atLine, &atColumn );
        kdDebug() << "atLine: " << atLine << endl;
        stri = "\n" + stri;
    } else {
        atLine = editIface->numLines();
        line = editIface->numLines();
        while (line > 0)
        {
            if (editIface->textLine(line).isEmpty())
            {
                --line;
                continue;
            }
            else
            {
                if (editIface->textLine(line).contains(QRegExp(".*#include .*\\.moc.*")))
                    atLine = line;
                break;
            }
        }
        kdDebug() << "atLine (2): " << atLine << endl;
        atColumn = 0;
    }
    
//    editIface->insertLine( atLine + 1, QString::fromLatin1("") );
         kdDebug() << "at line in intg: " << atLine  << " atCol: " << atColumn << endl;
         kdDebug() << "text: " << stri << endl;
    editIface->insertText( atLine, atColumn, stri );
    KTextEditor::View *activeView = dynamic_cast<KTextEditor::View*>( m_part->partController()->activePart()->widget() );
    if (activeView)
    {
        KTextEditor::ViewCursorInterface* cursor = dynamic_cast<KTextEditor::ViewCursorInterface*>(activeView );
        if (cursor)
            cursor->setCursorPositionReal( atLine+3, 1 );
    }
    
    cppPart->backgroundParser()->addFile( implementationFile );
}

QString QtDesignerCppIntegration::accessID(FunctionDom fun) const
{
    if( fun->isSignal() )
        return QString::fromLatin1( "signals" );

    switch( fun->access() )
    {
        case CodeModelItem::Public:
            if( fun->isSlot() )
                return QString::fromLatin1( "public slots" );
            return QString::fromLatin1( "public" );

        case CodeModelItem::Protected:
            if( fun->isSlot() )
                return QString::fromLatin1( "protected slots" );
            return QString::fromLatin1( "protected" );

        case CodeModelItem::Private:
            if( fun->isSlot() )
                return QString::fromLatin1( "private slots" );
            return QString::fromLatin1( "private" );
    }

    return QString::null;
}

void QtDesignerCppIntegration::processImplementationName(QString &name)
{
    name.replace(".h", ".cpp");
}

#include "qtdesignercppintegration.moc"
