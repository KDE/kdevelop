/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "scriptprojectpart.h"

#include <qdir.h>
#include <qvaluestack.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"


typedef KGenericFactory<ScriptProjectPart> ScriptProjectFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevscriptproject, ScriptProjectFactory( "kdevscriptproject" ) );

ScriptProjectPart::ScriptProjectPart(QObject *parent, const char *name, const QStringList &)
    : KDevProject(parent, name)
{
    setInstance(ScriptProjectFactory::instance());

    //    setXMLFile("kdevscriptproject.rc");
}


ScriptProjectPart::~ScriptProjectPart()
{}


void ScriptProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    m_projectDirectory = dirName;
    m_projectName = projectName;

    // Put all files from all subdirectories into file list
    QValueStack<QString> s;
    int prefixlen = m_projectDirectory.length()+1;
    s.push(m_projectDirectory);
    
    QDir dir;
    do {
        dir.setPath(s.pop());
        kdDebug(9025) << "Examining: " << dir.path() << endl;
        const QFileInfoList *dirEntries = dir.entryInfoList();
        QListIterator<QFileInfo> it(*dirEntries);
        for (; it.current(); ++it) {
            QString fileName = it.current()->fileName();
            if (fileName == "." || fileName == "..")
                continue;
            QString path = it.current()->absFilePath();
            if (it.current()->isDir()) {
                kdDebug(9025) << "Pushing: " << path << endl;
                s.push(path);
            }
            else {
                kdDebug(9025) << "Adding: " << path << endl;
                m_sourceFiles.append(path.mid(prefixlen));
            }
        }
    } while (!s.isEmpty());

}


void ScriptProjectPart::closeProject()
{
}


QString ScriptProjectPart::projectDirectory()
{
    return m_projectDirectory;
}


QString ScriptProjectPart::projectName()
{
    return m_projectName;
}


QString ScriptProjectPart::mainProgram()
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevscriptproject/run/mainprogram");
}


QString ScriptProjectPart::activeDirectory()
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevscriptproject/general/activedir");
}


QStringList ScriptProjectPart::allSourceFiles()
{
    QStringList res;

    QStringList::ConstIterator it;
    for (it = m_sourceFiles.begin(); it != m_sourceFiles.end(); ++it)
        res += (m_projectDirectory + "/" + (*it));

    return res;
}

void ScriptProjectPart::addFile(const QString &fileName)
{
    m_sourceFiles.append(fileName);
    emit addedFileToProject(fileName);
}


void ScriptProjectPart::removeFile(const QString &fileName)
{
    m_sourceFiles.remove(fileName);
    emit removedFileFromProject(fileName);
}


#include "scriptprojectpart.moc"
