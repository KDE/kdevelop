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
        KMessageBox::error(0, i18n("Cannot find implementation class for form: %1").arg(formName));

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

    m_part->backgroundParser()->addFile( klass->fileName() );

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
    TranslationUnitAST *translationUnit = m_part->backgroundParser()->translationUnit(implementationFile);
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

void QtDesignerIntegration::loadSettings(QDomDocument dom, QString path)
{
    QDomElement el = DomUtil::elementByPath(dom, path + "/qtdesigner");
    if (el.isNull())
        return;
    QDomNodeList impls = el.elementsByTagName("implementation");
    for (uint i = 0; i < impls.count(); ++i)
    {
        QDomElement el = impls.item(i).toElement();
        if (el.isNull())
            continue;
        FileDom file = m_part->codeModel()->fileByName(el.attribute("implementationpath"));
        if (!file)
            continue;
        ClassList cllist = file->classByName(el.attribute("class"));
        if (cllist.count() > 0)
            m_implementations[el.attribute("path")] = cllist.first();
    }
}

void QtDesignerIntegration::saveSettings(QDomDocument dom, QString path)
{
    kdDebug() << "QtDesignerIntegration::saveSettings" << endl;
    QDomElement el = DomUtil::createElementByPath(dom, path + "/qtdesigner");
    for (QMap<QString, ClassDom>::const_iterator it = m_implementations.begin(); 
        it != m_implementations.end(); ++it)
    {
        QDomElement il = dom.createElement("implementation");
        el.appendChild(il);
        il.setAttribute("path", it.key());
        il.setAttribute("implementationpath", it.data()->fileName());
        il.setAttribute("class", it.data()->name());
    }
}

struct MyPred{
    MyPred(const QString &functionName): m_functionName(functionName) {}
    bool operator () (const FunctionDefinitionDom& fun){
        kdDebug() << "    ==: " << fun->name() << " vs " << m_functionName << endl;
        if (fun->name() == m_functionName)
            return true;
        return false;
    }
    QString m_functionName;
};


void QtDesignerIntegration::openFunction(const QString &formName, const QString &functionName)
{
    kdDebug() << "QtDesignerIntegration::openFunction, formName = " << formName 
        << ", functionName = " << functionName << endl;
    QString fn = functionName;
    if (fn.find("(") > 0)
        fn.remove(fn.find("("), fn.length());
    
    if (!m_implementations[formName])
        return;

    int line = -1, col = -1;
    
/*    FunctionDefinitionList list;       
    MyPred mypred(fn);
    CodeModelUtils::findFunctionDefinitions<MyPred>(mypred, m_implementations[formName], list);
    if (list.count() == 0)
        return;
    
    list.first()->getStartPosition(&line, &col);*/

    QString impl = m_implementations[formName]->fileName();
    impl.replace(".h", ".cpp");
    
//    kdDebug() << "seeking for fn = " << fn << endl;
    if (m_part->codeModel()->hasFile(impl))
    {
        FunctionDefinitionList list = m_part->codeModel()->fileByName(impl)->functionDefinitionList();
        for (FunctionDefinitionList::const_iterator it = list.begin(); it != list.end(); ++it)
        {
//            kdDebug() << " <<: " << (*it)->name() << endl;
            if ((*it)->name() == fn)
                (*it)->getStartPosition(&line, &col);
        }
    }
    
    m_part->partController()->editDocument(KURL(impl), line, col);
}


#include "qtdesignerintegration.moc"
