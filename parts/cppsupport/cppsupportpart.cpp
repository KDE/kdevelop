/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#define GIDEON

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kregexp.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "classstore.h"

#include "cppsupportpart.h"
#include "cppsupportfactory.h"
#include "parsedclass.h"
#include "parsedattribute.h"
#include "parsedmethod.h"
#include "classparser.h"
#include "addclassmethoddlg.h"
#include "addclassattributedlg.h"


CppSupportPart::CppSupportPart(bool cpp, KDevApi *api, QObject *parent, const char *name)
    : KDevLanguageSupport(api, parent, name)
{
#ifndef GIDEON
    setInstance(CppSupportFactory::instance());
#endif
    
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( core(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

    m_parser = 0;
    withcpp = cpp;
}


CppSupportPart::~CppSupportPart()
{
    delete m_parser;
}


void CppSupportPart::projectOpened()
{
    kdDebug(9007) << "CppSupportPart::projectSpaceOpened()" << endl;

    connect( project(), SIGNAL(addedFileToProject(const QString &)),
             this, SLOT(addedFileToProject(const QString &)) );
    connect( project(), SIGNAL(removedFileFromProject(const QString &)),
             this, SLOT(removedFileFromProject(const QString &)) );

    // We want to parse only after all components have been
    // properly initialized
    m_parser = new CClassParser(classStore());
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void CppSupportPart::projectClosed()
{
    delete m_parser;
    m_parser = 0;
}


static QString findHeader(const QStringList &list, const QString &header)
{
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        QString s = *it;
        int pos = s.findRev('.');
        if (pos != -1)
            s = s.left(pos) + ".h";
        if (s.right(header.length()) == header)
            return s;
    }

    return QString::null;
}

void CppSupportPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (context->hasType("editor")) {
        const EditorContext *econtext = static_cast<const EditorContext*>(context);
        QString str = econtext->linestr();
        //        int col = econtext->col();
        //        KRegExp re("[ \t]*#include[ \t]*[<\"](.*)[>\"][ \t]*");
        KRegExp re(" *#include *[<\"](.*)[>\"] *");
        if (re.match(str) &&
            !findHeader(project()->allSourceFiles(), re.group(1)).isEmpty()) {
            popupstr = re.group(1);
            popup->insertItem( i18n("Goto include file: %1").arg(popupstr),
                               this, SLOT(slotGotoIncludeFile()) );
        }
    }
}


void CppSupportPart::maybeParse(const QString fileName)
{
    QFileInfo fi(fileName);
    QString path = fi.filePath();
    QString ext = fi.extension();
    if (ext == "cpp" || ext == "cc" || ext == "cxx") {
        m_parser->parse(path.left(path.length()-ext.length()) + "h");
        m_parser->parse(fileName);
    } else if (ext == "h") {
        m_parser->parse(fileName);
    }
}


void CppSupportPart::initialParse()
{
    kdDebug(9007) << "CppSupportPart::initialParse()" << endl;
    
    if (project()) {
        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allSourceFiles();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it)
            maybeParse(*it);
        
        emit updatedSourceInfo();
        kapp->restoreOverrideCursor();
    } else {
        kdDebug(9007) << "No project" << endl;
    }
}


void CppSupportPart::addedFileToProject(const QString &fileName)
{
    kdDebug(9007) << "CppSupportPart::addedFileToProject()" << endl;
    maybeParse(fileName);
    emit updatedSourceInfo();
}


void CppSupportPart::removedFileFromProject(const QString &fileName)
{
    kdDebug(9007) << "CppSupportPart::removedFileFromProject()" << endl;
    m_parser->removeWithReferences(fileName);
    emit updatedSourceInfo();
}


void CppSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9007) << "CppSupportPart::savedFile()" << endl;

    if (project()->allSourceFiles().contains(fileName)) {
        maybeParse(fileName);
        emit updatedSourceInfo();
    }
}


void CppSupportPart::slotGotoIncludeFile()
{
    QString fileName = findHeader(project()->allSourceFiles(), popupstr);
    if (!fileName.isEmpty())
        core()->gotoSourceFile(fileName, 0);
    
}


bool CppSupportPart::hasFeature(Features feature)
{
    if (!withcpp)
        return false;
    
    return
        (feature == Signals)
        || (feature == Slots)
        || (feature == Namespaces)
        || (feature == AddMethod)
        || (feature == AddAttribute);
}


void CppSupportPart::newClass()
{
}


void CppSupportPart::addMethod(const QString &className)
{
    AddClassMethodDialog dlg(0, "methodDlg");
    if (!dlg.exec())
        return;
    
    ParsedMethod *pm = dlg.asSystemObj();
    pm->setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass *pc = classStore()->getClassByName(className);
    
    if (pm->isSignal) {
        for (pc->signalIterator.toFirst(); pc->signalIterator.current(); ++pc->signalIterator) {
            ParsedMethod *meth = pc->signalIterator.current();
            if (meth->access == pm->access && 
                atLine < meth->declarationEndsOnLine)
                atLine = meth->declarationEndsOnLine;
        }
    } else if (pm->isSlot) {
        for (pc->slotIterator.toFirst(); pc->slotIterator.current(); ++pc->slotIterator) {
            ParsedMethod *meth = pc->slotIterator.current();
            if (meth->access == pm->access && 
                atLine < meth->declarationEndsOnLine)
                atLine = meth->declarationEndsOnLine;
        }
    } else {
        for (pc->methodIterator.toFirst(); pc->methodIterator.current(); ++pc->methodIterator) {
            ParsedMethod *meth = pc->methodIterator.current();
            if (meth->access == pm->access && 
                atLine < meth->declarationEndsOnLine)
                atLine = meth->declarationEndsOnLine;
        }
    }

    QString headerCode = asHeaderCode(pm);
    
    if (atLine == -1) {
        if (pm->isSignal) 
            headerCode.prepend(QString("signals:\n"));
        else if (pm->access == PIE_PUBLIC)
            headerCode.prepend(QString("public:%1\n").arg(pm->isSlot? " slots" :  ""));
        else if (pm->access == PIE_PROTECTED)
            headerCode.prepend(QString("protected:\n").arg(pm->isSlot? " slots" :  ""));
        else if (pm->access == PIE_PRIVATE) 
            headerCode.prepend(QString("private:\n").arg(pm->isSlot? " slots" :  ""));
        else
            kdDebug(9007) << "CppSupportPart::selectedAddMethod: Unknown access "
                          << (int)pm->access << endl;

        atLine = pc->declarationEndsOnLine;
    } else 
        atLine++;

    core()->gotoSourceFile(pc->declaredInFile, atLine);
    kdDebug(9007) << "####################" << "Adding at line " << atLine << " " 
                  << headerCode << endl
                  << "####################";

    QString cppCode = asCppCode(pm);
    
    core()->gotoSourceFile(pc->definedInFile, atLine);
    kdDebug(9007) << "####################" << "Adding at line " << atLine
                  << " " << cppCode
                  << "####################" << endl;
    
    delete pm;
}


void CppSupportPart::addAttribute(const QString &className)
{
    AddClassAttributeDialog dlg(0, "attrDlg");
    if( !dlg.exec() )
      return;

    ParsedAttribute *pa = dlg.asSystemObj();
    pa->setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass *pc = classStore()->getClassByName(className);
    
    for (pc->attributeIterator.toFirst(); pc->attributeIterator.current(); ++pc->attributeIterator) {
        ParsedAttribute *attr = pc->attributeIterator.current();
        if (attr->access == pa->access && 
            atLine < attr->declarationEndsOnLine)
            atLine = attr->declarationEndsOnLine;
    }
    
    QString headerCode = asHeaderCode(pa);
    
    if (atLine == -1) {
        if (pa->access == PIE_PUBLIC)
            headerCode.prepend("public: // Public attributes\n");
        else if (pa->access == PIE_PROTECTED)
            headerCode.prepend("protected: // Protected attributes\n");
        else if (pa->access == PIE_PRIVATE) 
            headerCode.prepend("private: // Private attributes\n");
        else
            kdDebug(9007) << "CppSupportPart::selectedAddAttribute: Unknown access "
                          << (int)pa->access << endl;

        atLine = pc->declarationEndsOnLine;
    } else 
        atLine++;

    core()->gotoSourceFile(pc->declaredInFile, atLine);
    kdDebug(9007) << "####################" << "Adding at line " << atLine
                  << " " << headerCode
                  << "####################" << endl;

    delete pa;
}


QString CppSupportPart::asHeaderCode(ParsedMethod *pm)
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


QString CppSupportPart::asCppCode(ParsedMethod *pm)
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


QString CppSupportPart::asHeaderCode(ParsedAttribute *pa)
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

#include "cppsupportpart.moc"
