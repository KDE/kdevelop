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
#include <qregexp.h>
#include <qvaluestack.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <kaction.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"
#include "kdevlanguagesupport.h"
#include "scriptoptionswidget.h"
#include "scriptnewfiledlg.h"


typedef KGenericFactory<ScriptProjectPart> ScriptProjectFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevscriptproject, ScriptProjectFactory( "kdevscriptproject" ) );

ScriptProjectPart::ScriptProjectPart(QObject *parent, const char *name, const QStringList &)
    : KDevProject(parent, name)
{
    setInstance(ScriptProjectFactory::instance());

    setXMLFile("kdevscriptproject.rc");

    KAction *action;
    action = new KAction( i18n("New File..."), 0,
                          this, SLOT(slotNewFile()),
                          actionCollection(), "file_newfile" );
    
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
}


ScriptProjectPart::~ScriptProjectPart()
{}


void ScriptProjectPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Script project options"));
    ScriptOptionsWidget *w = new ScriptOptionsWidget(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}


/**
 * This should really be merged with FileTreeWidget::matchesHidePattern()
 * and put in its own class.
 */
static bool matchesPattern(const QString &fileName, const QStringList &patternList)
{
    QStringList::ConstIterator it;
    for (it = patternList.begin(); it != patternList.end(); ++it) {
        QRegExp re(*it, true, true);
        if (re.search(fileName) == 0 && re.matchedLength() == (int)fileName.length())
            return true;
    }

    return false;
}


void ScriptProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    m_projectDirectory = dirName;
    m_projectName = projectName;

    QDomDocument &dom = *projectDom();

    QString includepatterns
        = DomUtil::readEntry(dom, "/kdevscriptproject/general/includepatterns");
    QStringList includepatternList;
    if ( includepatterns.isNull() ) {
        if ( languageSupport() )
            includepatternList = languageSupport()->fileFilters();
    } else {
        includepatternList = QStringList::split(",", includepatterns);
    }

    QString excludepatterns
        = DomUtil::readEntry(dom, "/kdevscriptproject/general/excludepatterns");
    if (excludepatterns.isNull())
        excludepatterns = "*~";
    QStringList excludepatternList = QStringList::split(",", excludepatterns);

    // Put all files from all subdirectories into file list
    QValueStack<QString> s;
    int prefixlen = m_projectDirectory.length()+1;
    s.push(m_projectDirectory);
    
    QDir dir;
    do {
        dir.setPath(s.pop());
        kdDebug(9015) << "Examining: " << dir.path() << endl;
        const QFileInfoList *dirEntries = dir.entryInfoList();
        QListIterator<QFileInfo> it(*dirEntries);
        for (; it.current(); ++it) {
            QString fileName = it.current()->fileName();
            if (fileName == "." || fileName == "..")
                continue;
            QString path = it.current()->absFilePath();
            if (it.current()->isDir()) {
                kdDebug(9015) << "Pushing: " << path << endl;
                s.push(path);
            }
            else {
                if (matchesPattern(path, includepatternList)
                    && !matchesPattern(path, excludepatternList)) {
                    kdDebug(9015) << "Adding: " << path << endl;
                    m_sourceFiles.append(path.mid(prefixlen));
                } else {
                    kdDebug(9015) << "Ignoring: " << path << endl;
                }
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


QStringList ScriptProjectPart::allFiles()
{
    QStringList res;

    QStringList::ConstIterator it;
    for (it = m_sourceFiles.begin(); it != m_sourceFiles.end(); ++it)
        res += (m_projectDirectory + "/" + (*it));

    return res;
}

void ScriptProjectPart::addFile(const QString &fileName)
{
    kdDebug(9015) << "AddFile2" << fileName << endl;
    m_sourceFiles.append(fileName);
    emit addedFileToProject(fileName);
}


void ScriptProjectPart::removeFile(const QString &fileName)
{
    m_sourceFiles.remove(fileName);
    emit removedFileFromProject(fileName);
}


void ScriptProjectPart::slotNewFile()
{
    ScriptNewFileDialog dlg(this);
    dlg.exec();
}

#include "scriptprojectpart.moc"
