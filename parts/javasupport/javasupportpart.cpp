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

#include "javasupportpart.h"
#include "javasupportfactory.h"
#include "parsedclass.h"
#include "parsedattribute.h"
#include "parsedmethod.h"
#include "classparser.h"
#include "javaaddclassmethoddlg.h"
#include "javaaddclassattributedlg.h"


JavaSupportPart::JavaSupportPart(KDevApi *api, QObject *parent, const char *name)
    : KDevLanguageSupport(api, parent, name)
{
    setInstance(JavaSupportFactory::instance());
    
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( core(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

    m_parser = 0;
}


JavaSupportPart::~JavaSupportPart()
{
    delete m_parser;
}


void JavaSupportPart::projectOpened()
{
    kdDebug(9007) << "JavaSupportPart::projectSpaceOpened()" << endl;

    connect( project(), SIGNAL(addedFileToProject(const QString &)),
             this, SLOT(addedFileToProject(const QString &)) );
    connect( project(), SIGNAL(removedFileFromProject(const QString &)),
             this, SLOT(removedFileFromProject(const QString &)) );

    // We want to parse only after all components have been
    // properly initialized
    m_parser = new JavaClassParser(classStore());
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void JavaSupportPart::projectClosed()
{
    delete m_parser;
    m_parser = 0;
}


void JavaSupportPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (context->hasType("editor")) {
        const EditorContext *econtext = static_cast<const EditorContext*>(context);
        QString str = econtext->linestr();
        //        int col = econtext->col();
        //        KRegExp re("[ \t]*#include[ \t]*[<\"](.*)[>\"][ \t]*");
//        KRegExp re(" *#include *[<\"](.*)[>\"] *");
//        if (re.match(str) &&
//            !findHeader(project()->allSourceFiles(), re.group(1)).isEmpty()) {
//            popupstr = re.group(1);
//            popup->insertItem( i18n("Goto include file: %1").arg(popupstr),
//                               this, SLOT(slotGotoIncludeFile()) );
//        }
    }
}


void JavaSupportPart::maybeParse(const QString fileName)
{
    QFileInfo fi(fileName);
    QString path = fi.filePath();
    QString ext = fi.extension();
    if (ext == "java") {
        m_parser->parse(fileName);
    }
}


void JavaSupportPart::initialParse()
{
    kdDebug(9013) << "JavaSupportPart::initialParse()" << endl;
    
    if (project()) {
        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allSourceFiles();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it)
            maybeParse(*it);
        
        emit updatedSourceInfo();
        kapp->restoreOverrideCursor();
    } else {
        kdDebug(9013) << "No project" << endl;
    }
}


void JavaSupportPart::addedFileToProject(const QString &fileName)
{
    kdDebug(9013) << "JavaSupportPart::addedFileToProject()" << endl;
    maybeParse(fileName);
    emit updatedSourceInfo();
}


void JavaSupportPart::removedFileFromProject(const QString &fileName)
{
    kdDebug(9013) << "JavaSupportPart::removedFileFromProject()" << endl;
    m_parser->removeWithReferences(fileName);
    emit updatedSourceInfo();
}


void JavaSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9013) << "JavaSupportPart::savedFile()" << endl;

    if (project()->allSourceFiles().contains(fileName)) {
        maybeParse(fileName);
        emit updatedSourceInfo();
    }
}




KDevLanguageSupport::Features JavaSupportPart::features()
{
    return Features(Classes | Structs | Functions | Variables | Namespaces
                    | AddMethod | AddAttribute);
}


void JavaSupportPart::newClass()
{
}


void JavaSupportPart::addMethod(const QString &className)
{
    JavaAddClassMethodDialog dlg(0, "methodDlg");
    if (!dlg.exec())
        return;
    
    ParsedMethod *pm = dlg.asSystemObj();
    pm->setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass *pc = classStore()->getClassByName(className);
    
    for (pc->methodIterator.toFirst(); pc->methodIterator.current(); ++pc->methodIterator) {
    	ParsedMethod *meth = pc->methodIterator.current();
     	if (meth->access == pm->access &&
      		atLine < meth->declarationEndsOnLine)
                atLine = meth->declarationEndsOnLine;
    }

    QString javaCode = asJavaCode(pm);
    
    core()->gotoSourceFile(pc->definedInFile, atLine);
    kdDebug(9013) << "####################" << "Adding at line " << atLine
                  << " " << javaCode
                  << "####################" << endl;
    
    delete pm;
}


void JavaSupportPart::addAttribute(const QString &className)
{
    JavaAddClassAttributeDialog dlg(0, "attrDlg");
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


    core()->gotoSourceFile(pc->declaredInFile, atLine);

    delete pa;
}


QString JavaSupportPart::asJavaCode(ParsedMethod *pm)
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


QString JavaSupportPart::asJavaCode(ParsedAttribute *pa)
{
    QString str = "  ";
    str += pa->comment;
    str += "\n  ";

    if (pa->isStatic)
        str += "static ";

    str += pa->asString();
    str += ";\n";

    return str;
}

#include "javasupportpart.moc"
