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

#include "javasupport.h"
#include "parsedclass.h"
#include "parsedattribute.h"
#include "parsedmethod.h"
#include "classstore.h"
#include "classparser.h"
#include "javaaddclassmethoddlg.h"
#include "javaaddclassattributedlg.h"
#include "kdevnodes.h"
#include "projectspace.h"
#include "kdeveditormanager.h"
#include "main.h"
#include "KDevComponentManager.h"


JavaSupport::JavaSupport(QObject *parent, const char *name)
    : KDevLanguageSupport(parent, name)
{
    setInstance(JavaSupportFactory::instance());

    m_parser = 0;
}


JavaSupport::~JavaSupport()
{
    delete m_parser;
}

ClassStore* JavaSupport::classStore(){
	return static_cast<ClassStore*>(componentManager()->component("KDevClassStore"));
}

ProjectSpace* JavaSupport::projectSpace(){
	return static_cast<ProjectSpace*>(componentManager()->component("ProjectSpace"));
}

KDevEditorManager* JavaSupport::editorManager(){
	return static_cast<KDevEditorManager*>(componentManager()->component("KDevEditorManager"));
}

void JavaSupport::slotProjectSpaceOpened()
{
    kdDebug(9007) << "JavaSupport::projectSpaceOpened()" << endl;

    ProjectSpace *ps = projectSpace();
    connect( ps, SIGNAL(sigAddedFileToProject(KDevFileNode*)),
             this, SLOT(addedFileToProject(KDevFileNode*)) );
    connect( ps, SIGNAL(sigRemovedFileFromProject(KDevFileNode*)),
             this, SLOT(removedFileFromProject(KDevFileNode*)) );

    KDevEditorManager *em = editorManager();
    connect( em, SIGNAL(sigSavedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );

    m_parser = new JavaClassParser(classStore());
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void JavaSupport::initialParse()
{
    kdDebug(9007) << "JavaSupport::initialParse()" << endl;

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


void JavaSupport::addedFileToProject(KDevFileNode* pNode)
{
    kdDebug(9007) << "JavaSupport::addedFileToProject()" << endl;
    QString fileName = pNode->absoluteFileName();
    m_parser->parse(fileName);

    emit sigUpdatedSourceInfo();
}


void JavaSupport::removedFileFromProject(KDevFileNode* pNode)
{
    kdDebug(9007) << "JavaSupport::removedFileFromProject()" << endl;
    QString fileName = pNode->absoluteFileName();
    m_parser->removeWithReferences(fileName);
    emit sigUpdatedSourceInfo();
}


void JavaSupport::savedFile(const QString &fileName)
{
    kdDebug(9007) << "JavaSupport::savedFile()" << endl;

    m_parser->parse(fileName);
    emit sigUpdatedSourceInfo();
}


bool JavaSupport::hasFeature(Features feature)
{
    return (feature==AddMethod) || (feature==AddAttribute);
}


void JavaSupport::newClassRequested()
{
}


void JavaSupport::addMethodRequested(const QString &className)
{
    JavaAddClassMethodDlg dlg(0, "methodDlg");
    if (!dlg.exec())
        return;

    ParsedMethod *pm = dlg.asSystemObj();
    pm->setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass *pc = classStore()->getClassByName(className);

    for (pc->methodIterator.toFirst(); pc->methodIterator.current(); ++pc->methodIterator) {
        ParsedMethod *meth = pc->methodIterator.current();
        if (meth->exportScope == pm->exportScope &&
            atLine < meth->declarationEndsOnLine)
            atLine = meth->declarationEndsOnLine;
    }

    QString javaCode = asJavaCode(pm);

    atLine++;

    editorManager()->gotoSourceFile(pc->definedInFile, atLine);
    kdDebug(9007) << "####################" << "Adding at line " << atLine << " "
                  << javaCode << endl
                  << "####################";

    delete pm;
}


void JavaSupport::addAttributeRequested(const QString &className)
{
    JavaAddClassAttributeDlg dlg(0, "attrDlg");
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

    QString javaCode = asJavaCode(pa);

    atLine++;

    editorManager()->gotoSourceFile(pc->definedInFile, atLine);
    kdDebug(9007) << "####################" << "Adding at line " << atLine
                  << " " << javaCode
                  << "####################" << endl;

    delete pa;
}



QString JavaSupport::asJavaCode(ParsedMethod *pm)
{

    QString str = pm->comment;
    str += "\n";

    QString path = pm->path();

    str += pm->type;
    str += " ";
    str += pm->path();

    str += "{\n}\n";

    return str;
}


QString JavaSupport::asJavaCode(ParsedAttribute *pa)
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


QStringList JavaSupport::fileFilters(){
  QStringList list;
  list << "*.java";
  list << "*";
  return list;
}

#include "javasupport.moc"
