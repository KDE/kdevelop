/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstringlist.h>
#include <qtimer.h>
#include <kdebug.h>

#include "cppsupport.h"
#include "parsedclass.h"
#include "parsedattribute.h"
#include "parsedmethod.h"
#include "classstore.h"
#include "classparser.h"
#include "caddclassmethoddlg.h"
#include "caddclassattributedlg.h"
#include "kdevnodes.h"
#include "projectspace.h"
#include "kdeveditormanager.h"
#include "main.h"


CppSupport::CppSupport(QObject *parent, const char *name)
    : KDevLanguageSupport(parent, name)
{
    setInstance(CppSupportFactory::instance());

    m_parser = 0;
}


CppSupport::~CppSupport()
{
    delete m_parser;
}


void CppSupport::projectSpaceOpened()
{
    kdDebug(9007) << "CppSupport::projectSpaceOpened()" << endl;

    ProjectSpace *ps = projectSpace();
    connect( ps, SIGNAL(sigAddedFileToProject(KDevFileNode*)),
             this, SLOT(addedFileToProject(KDevFileNode*)) );
    connect( ps, SIGNAL(sigRemovedFileFromProject(KDevFileNode*)),
             this, SLOT(removedFileFromProject(KDevFileNode*)) );

    KDevEditorManager *em = editorManager();
    connect( em, SIGNAL(sigSavedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );

    m_parser = new CClassParser(classStore());
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void CppSupport::initialParse()
{
    kdDebug(9007) << "CppSupport::initialParse()" << endl;
    
    // quick hack
    Project* pProject = projectSpace()->currentProject();
    if (pProject) {
        QStringList files = pProject->allAbsoluteFileNames();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
            m_parser->parse(*it);
        }

        emit sigUpdatedSourceInfo();
    }
}


void CppSupport::addedFileToProject(KDevFileNode* pNode)
{
    kdDebug(9007) << "CppSupport::addedFileToProject()" << endl;
    QString fileName = pNode->absoluteFileName();
    m_parser->parse(fileName);
  
    emit sigUpdatedSourceInfo();
}


void CppSupport::removedFileFromProject(KDevFileNode* pNode)
{
    kdDebug(9007) << "CppSupport::removedFileFromProject()" << endl;
    QString fileName = pNode->absoluteFileName();
    m_parser->removeWithReferences(fileName);
    emit sigUpdatedSourceInfo();
}


void CppSupport::savedFile(const QString &fileName)
{
    kdDebug(9007) << "CppSupport::savedFile()" << endl;

    m_parser->parse(fileName);
    emit sigUpdatedSourceInfo();
}


bool CppSupport::hasFeature(Features feature)
{
    return (feature==AddMethod) || (feature==AddAttribute);
}


void CppSupport::newClassRequested()
{
}


void CppSupport::addMethodRequested(const QString &className)
{
    CAddClassMethodDlg dlg(0, "methodDlg");
    if (!dlg.exec())
        return;
    
    ParsedMethod *pm = dlg.asSystemObj();
    pm->setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass *pc = classStore()->getClassByName(className);
    
    if (pm->isSignal) {
        for (pc->signalIterator.toFirst(); pc->signalIterator.current(); ++pc->signalIterator) {
            ParsedMethod *meth = pc->signalIterator.current();
            if (meth->exportScope == pm->exportScope && 
                atLine < meth->declarationEndsOnLine)
                atLine = meth->declarationEndsOnLine;
        }
    } else if (pm->isSlot) {
        for (pc->slotIterator.toFirst(); pc->slotIterator.current(); ++pc->slotIterator) {
            ParsedMethod *meth = pc->slotIterator.current();
            if (meth->exportScope == pm->exportScope && 
                atLine < meth->declarationEndsOnLine)
                atLine = meth->declarationEndsOnLine;
        }
    } else {
        for (pc->methodIterator.toFirst(); pc->methodIterator.current(); ++pc->methodIterator) {
            ParsedMethod *meth = pc->methodIterator.current();
            if (meth->exportScope == pm->exportScope && 
                atLine < meth->declarationEndsOnLine)
                atLine = meth->declarationEndsOnLine;
        }
    }

    QString headerCode = asHeaderCode(pm);
    
    if (atLine == -1) {
        if (pm->isSignal) 
            headerCode.prepend(QString("signals:\n"));
        else if (pm->exportScope == PIE_PUBLIC)
            headerCode.prepend(QString("public:%1\n").arg(pm->isSlot? " slots" :  ""));
        else if (pm->exportScope == PIE_PROTECTED)
            headerCode.prepend(QString("protected:\n").arg(pm->isSlot? " slots" :  ""));
        else if (pm->exportScope == PIE_PRIVATE) 
            headerCode.prepend(QString("private:\n").arg(pm->isSlot? " slots" :  ""));
        else
            kdDebug(9007) << "CppSupport::selectedAddMethod: Unknown exportScope "
                          << (int)pm->exportScope << endl;

        atLine = pc->declarationEndsOnLine;
    } else 
        atLine++;

    editorManager()->gotoSourceFile(pc->declaredInFile, atLine);
    kdDebug(9007) << "####################" << "Adding at line " << atLine << " " 
                  << headerCode << endl
                  << "####################";

    QString cppCode = asCppCode(pm);
    
    editorManager()->gotoSourceFile(pc->definedInFile, atLine);
    kdDebug(9007) << "####################" << "Adding at line " << atLine
                  << " " << cppCode
                  << "####################" << endl;
    
    delete pm;
}


void CppSupport::addAttributeRequested(const QString &className)
{
    CAddClassAttributeDlg dlg(0, "attrDlg");
    if( !dlg.exec() )
      return;

    ParsedAttribute *pa = dlg.asSystemObj();
    pa->setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass *pc = classStore()->getClassByName(className);
    
    for (pc->attributeIterator.toFirst(); pc->attributeIterator.current(); ++pc->attributeIterator) {
        ParsedAttribute *attr = pc->attributeIterator.current();
        if (attr->exportScope == pa->exportScope && 
            atLine < attr->declarationEndsOnLine)
            atLine = attr->declarationEndsOnLine;
    }
    
    QString headerCode = asHeaderCode(pa);
    
    if (atLine == -1) {
        if (pa->exportScope == PIE_PUBLIC)
            headerCode.prepend("public: // Public attributes\n");
        else if (pa->exportScope == PIE_PROTECTED)
            headerCode.prepend("protected: // Protected attributes\n");
        else if (pa->exportScope == PIE_PRIVATE) 
            headerCode.prepend("private: // Private attributes\n");
        else
            kdDebug(9007) << "CppSupport::selectedAddAttribute: Unknown exportScope "
                          << (int)pa->exportScope << endl;

        atLine = pc->declarationEndsOnLine;
    } else 
        atLine++;

    editorManager()->gotoSourceFile(pc->declaredInFile, atLine);
    kdDebug(9007) << "####################" << "Adding at line " << atLine
                  << " " << headerCode
                  << "####################" << endl;

    delete pa;
}


QString CppSupport::asHeaderCode(ParsedMethod *pm)
{
    QString str = "  ";
    str += pm->comment;
    str += "\n  ";

    if (pm->isVirtual)
        str += "virtual ";
    
    if (pm->isStatic)
        str += "static ";
    
    str += pm->type;
    str += " ";
    str += pm->name;
    
    if (pm->isConst)
        str += " const";
    
    if (pm->isPure)
        str += " = 0";
    
    str += ";\n";

    return str;
}


QString CppSupport::asCppCode(ParsedMethod *pm)
{
    if (pm->isPure || pm->isSignal)
        return QString();

    QString str = pm->comment;
    str += "\n";

    // Take the path and replace all . with ::
    QString path = pm->path();
    path.replace( QRegExp( "\\." ), "::" );

    str += pm->type;
    str += " ";
    str += pm->path();
    
    if (pm->isConst)
        str += " const";
    
    str += "{\n}\n";

    return str;
}


QString CppSupport::asHeaderCode(ParsedAttribute *pa)
{
    QString str = "  ";
    str += pa->comment;
    str += "\n  ";

    if (pa->isConst)
        str += "const ";

    if (pa->isStatic)
        str += "static ";

    str += pa->asString();
    str += ";\n";

    return str;
}


QStringList CppSupport::fileFilters(){
  QStringList list;
  list << "*.h;*.hxx;*.cpp;*.cc;*.C;*.cxx;*.idl;*.c";
  list << "*.cpp;*.cc;*.C;*.cxx;*.c";
  list << "*.h;*.hxx;*.idl";
  list << "*";
  return list;
}

#include "cppsupport.moc"
