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
#include "qtdesignerintegration.h"

#include <qpair.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <ktexteditor/editinterface.h>

#include <kdevpartcontroller.h>
#include <kdevcreatefile.h>

#include "backgroundparser.h"
#include "cppsupportpart.h"
#include "implementationwidget.h"

QtDesignerIntegration::QtDesignerIntegration(CppSupportPart *part, const char* name)
    :KDevDesignerIntegration(part, name), m_part(part)
{
}

QtDesignerIntegration::~QtDesignerIntegration()
{
}

void QtDesignerIntegration::addFunction(const QString& formName, KInterfaceDesigner::Function function)
{
    kdDebug() << "QtDesignerIntegration::addFunction: form: " << formName << ", function: " << function.function << endl;
    
    if (!m_implementations[formName])
        if (!selectImplementation(formName))
            return;
    
    ClassDom klass = m_implementations[formName];
    if (!klass)
        KMessageBox::error(0, i18n("Can not find implementation class for form: %1").arg(formName));

    addFunctionToClass(function, klass);
}

void QtDesignerIntegration::editFunction(const QString& formName, KInterfaceDesigner::Function oldFunction, KInterfaceDesigner::Function function)
{
    kdDebug() << "QtDesignerIntegration::editFunction: form: " << formName 
        << ", old function: " << oldFunction.function
        << ", function: " << function.function << endl;
}

void QtDesignerIntegration::removeFunction(const QString& formName, KInterfaceDesigner::Function function)
{
    kdDebug() << "QtDesignerIntegration::removeFunction: form: " << formName << ", function: " << function.function << endl;
}

bool QtDesignerIntegration::selectImplementation(const QString &formName)
{
    QFileInfo fi(formName);
    if (!fi.exists())
        return false;
    
    ImplementationWidget selectImpl(m_part, formName);
    if (selectImpl.exec())
    {
        m_implementations[formName] = selectImpl.selectedClass();
        return true;
    }
    return false;
}

void QtDesignerIntegration::addFunctionToClass(KInterfaceDesigner::Function function, ClassDom klass)
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
    QStringList accessList;

    const FunctionList functionList = klass->functionList();
    for( FunctionList::ConstIterator it=functionList.begin(); it!=functionList.end(); ++it )
    {
        int funEndLine, funEndColumn;
        (*it)->getEndPosition( &funEndLine, &funEndColumn );
        QString access = accessID( *it );
        QPair<int, int> funEndPoint = qMakePair( funEndLine, funEndColumn );

        if( !points.contains(access) || points[access] < funEndPoint ){
            accessList.remove( access );
            accessList.push_back( access ); // move 'access' at the end of the list

            points[ access ] = funEndPoint;
        }
    }

    int insertedLine = 0;

    QString newAccess = function.access + ( function.type == "slot" ? " slots" : "" );
    if( !(accessList.contains(newAccess)) )
        accessList.push_back( newAccess );
//    accessList += newAccessList( accessList );

    for( QStringList::iterator it=accessList.begin(); it!=accessList.end(); ++it )
    {
        QString access = (*it).lower();

        QString str = function.returnType + " " + function.function;
        if (function.specifier == "virtual")
            str = "virtual " + str;
        else if (function.specifier == "pure virtual")
            str = "virtual " + str + " = 0";
        else if (function.specifier == "static")
            str = "static " + str;
        str += ";\n";

        QPair<int, int> pt;
        if( points.contains(*it) ) {
            pt = points[ *it ];
        } else {
            str.prepend( access + ":\n" );
            points[ *it ] = qMakePair( line-1, 0 );
            pt = points[ *it ]; // end of class declaration
        }

        editIface->insertText( pt.first + insertedLine + 1, 0 /*pt.second*/, str );
        insertedLine += str.contains( QChar('\n') );
    }

    m_part->backgroundParser()->addFile( klass->fileName() );

    if (function.specifier == "pure virtual")
        return;

    QString str = function.returnType + " " + klass->name() + "::" + function.function;
    if (function.specifier == "static")
        str = "static " + str;
    str += "\n{\n}\n";
        
    QFileInfo fi(klass->fileName());
    QString implementationFile = fi.baseName() + ".cpp";

    QFileInfo fileInfo( implementationFile );
    if( !QFile::exists(fileInfo.absFilePath()) ){
        m_part->createFileSupport()->createNewFile( fileInfo.extension(), fileInfo.dirPath(true), fileInfo.baseName() );
    }

    m_part->partController()->editDocument( KURL( implementationFile ) );
    editIface = dynamic_cast<KTextEditor::EditInterface*>( m_part->partController()->activePart() );
    if( !editIface )
        return;

    editIface->insertLine( editIface->numLines(), QString::fromLatin1("") );
    editIface->insertText( editIface->numLines()-1, 0, str );
    m_part->backgroundParser()->addFile( implementationFile );
}

QString QtDesignerIntegration::accessID(FunctionDom fun) const
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


#include "qtdesignerintegration.moc"
