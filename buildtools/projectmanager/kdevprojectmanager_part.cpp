/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include "kdevprojectmanager_widget.h"
#include "kdevprojectmanager_part.h"
#include "kdevprojectimporter.h"
#include "kdevprojectbuilder.h"
#include "kdevprojecteditor.h"
#include "importprojectjob.h"

#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <urlutil.h>

#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <ktrader.h>
#include <kaboutdata.h>

#include <kparts/componentfactory.h>

#include <qdir.h>
#include <qwhatsthis.h>
#include <qfileinfo.h>
#include <qtimer.h>

#include <kdevplugininfo.h>

typedef KDevGenericFactory<KDevProjectManagerPart> KDevProjectManagerFactory;
static const KDevPluginInfo data("kdevprojectmanager");
K_EXPORT_COMPONENT_FACTORY(libkdevprojectmanager, KDevProjectManagerFactory(data));

KDevProjectManagerPart::KDevProjectManagerPart(QObject *parent, const char *name, const QStringList&)
    : KDevProject(&data, parent, name ? name : "KDevProjectManagerPart")
{
    m_projectModel = new ProjectModel();
    m_dirty = false;
    
    setInstance(KDevProjectManagerFactory::instance());
        
    { // load the importers
        KTrader::OfferList lst = KTrader::self()->query("KDevelop/ProjectImporter");
        
        for (KTrader::OfferList::Iterator it = lst.begin(); it != lst.end(); ++it) {
            KService::Ptr ptr = *it;
    
            int error = 0;
            if (KDevProjectImporter *i = KParts::ComponentFactory::createInstanceFromService<KDevProjectImporter>(ptr, this, 
                                                ptr->name().latin1(), QStringList(), &error)) 
            {
                m_importers.insert(ptr->name(), i);            
            } else {
                kdDebug(9000) << "error:" << error << endl;
            }
        }    
    }
    
    { // load the builders
        KTrader::OfferList lst = KTrader::self()->query("KDevelop/ProjectBuilder");
        
        for (KTrader::OfferList::Iterator it = lst.begin(); it != lst.end(); ++it) {
            KService::Ptr ptr = *it;
    
            int error = 0;
            if (KDevProjectBuilder *i = KParts::ComponentFactory::createInstanceFromService<KDevProjectBuilder>(ptr, this, 
                                                ptr->name().latin1(), QStringList(), &error)) 
            {
                m_builders.insert(ptr->name(), i);            
            } else {
                kdDebug(9000) << "error:" << error << endl;
            }
        }    
    }
    
    m_widget = new KDevProjectManagerWidget(this);

    QWhatsThis::add(m_widget, i18n("Project Manager"));

    mainWindow()->embedSelectViewRight(m_widget, tr("Project Manager"), tr("Project Manager"));

    setXMLFile("kdevprojectmanager.rc");    
    
    m_updateProjectTimer = new QTimer(this);
    connect(m_updateProjectTimer, SIGNAL(timeout()), this, SLOT(updateProjectTimeout()));
}

KDevProjectManagerPart::~KDevProjectManagerPart()
{
    if (m_widget) {
        mainWindow()->removeView(m_widget);
        delete m_widget;
    }
}

ProjectFolderDom KDevProjectManagerPart::activeFolder()
{
    return m_widget->activeFolder();
}

ProjectTargetDom KDevProjectManagerPart::activeTarget()
{
    return m_widget->activeTarget();
}

ProjectFileDom KDevProjectManagerPart::activeFile()
{
    return m_widget->activeFile();
}

void KDevProjectManagerPart::updateProjectTimeout()
{
    import();
}

void KDevProjectManagerPart::openProject(const QString &dirName, const QString &projectName)
{
    m_projectDirectory = dirName;
    m_projectName = projectName;    

    import(ForceRefresh);
    
    KDevProject::openProject(dirName, projectName);    
}

void KDevProjectManagerPart::import(RefreshPolicy policy)
{
    QStringList oldFileList = allFiles();
    
    if (m_workspace)
        m_projectModel->removeItem(m_workspace->toItem());
        
    if (m_workspace = defaultImporter()->import(projectModel(), projectDirectory())->toFolder())
        m_projectModel->addItem(m_workspace->toItem());
    
    ImportProjectJob *job = ImportProjectJob::importProjectJob(m_workspace, defaultImporter());
    connect(job, SIGNAL(result(KIO::Job*)), this, SIGNAL(refresh()));
    job->start();
        
    QStringList newFileList = allFiles();

    bool hasChanges = computeChanges(oldFileList, newFileList);
    
    if ((hasChanges && policy == Refresh) || policy == ForceRefresh)
        emit refresh();

}

void KDevProjectManagerPart::closeProject()
{
}

KDevProjectManagerPart::Options KDevProjectManagerPart::options() const
{
    return (Options)0;
}

QString KDevProjectManagerPart::projectDirectory() const
{
    return m_projectDirectory;
}

QString KDevProjectManagerPart::projectName() const
{
    return m_projectName;
}

DomUtil::PairList KDevProjectManagerPart::runEnvironmentVars() const
{
    return DomUtil::PairList();
}

QString KDevProjectManagerPart::mainProgram(bool relative) const
{
    Q_UNUSED(relative);
    return QString();
}

QString KDevProjectManagerPart::runDirectory() const
{
    return m_projectDirectory;
}

QString KDevProjectManagerPart::runArguments() const
{
    return QString();
}

QString KDevProjectManagerPart::activeDirectory() const
{
    if (ProjectFolderDom folder = m_widget->activeFolder())
        return URLUtil::relativePath(projectDirectory(), folder->name());
    
    return QString::null;
}

QString KDevProjectManagerPart::buildDirectory() const
{
    // ### atm we can handle only srcdir == builddir :(
    return m_projectDirectory;
}

QStringList KDevProjectManagerPart::allFiles() const
{
    if (!m_workspace)
        return QStringList();
    else if (!(isDirty() || m_cachedFileList.isEmpty()))
        return m_cachedFileList;
    
    return const_cast<KDevProjectManagerPart*>(this)->allFiles();
}

QStringList KDevProjectManagerPart::allFiles()
{
    if (!m_workspace)
        return QStringList();
        
    ProjectItemDom dom = m_workspace->toItem();
    m_cachedFileList = fileList(dom);
    
    return m_cachedFileList;
}

QStringList KDevProjectManagerPart::distFiles() const
{
    return allFiles();
}

KDevProjectImporter *KDevProjectManagerPart::defaultImporter() const
{
    QDomDocument &dom = *projectDom();
    QString kind = DomUtil::readEntry(dom, "/general/importer");
    Q_ASSERT(!kind.isEmpty());
    
    if (m_importers.contains(kind))
        return m_importers[kind];
        
    kdDebug(9000) << "error: no default importer!" << endl;
    return 0;
}

KDevProjectBuilder *KDevProjectManagerPart::defaultBuilder() const
{
    QDomDocument &dom = *projectDom();
    QString kind = DomUtil::readEntry(dom, "/general/builder");
    Q_ASSERT(!kind.isEmpty());
    
    if (m_builders.contains(kind))
        return m_builders[kind];
        
    kdDebug(9000) << "error: no default builder!" << endl;
    return 0;
}

void KDevProjectManagerPart::addFiles(const QStringList &fileList)
{
    kdDebug(9000) << "KDevProjectManagerPart::addFiles:" << fileList << endl;
    
    //m_updateProjectTimer->stop();
    //m_updateProjectTimer->start(0, true);
}

void KDevProjectManagerPart::addFile(const QString &fileName)
{
    kdDebug(9000) << "KDevProjectManagerPart::addFile:" << fileName << endl;
    
    addFiles(QStringList() << fileName);
}

void KDevProjectManagerPart::removeFiles(const QStringList &fileList)
{
    kdDebug(9000) << "KDevProjectManagerPart::removeFiles" << fileList << endl;
    
    //m_updateProjectTimer->stop();
    //m_updateProjectTimer->start(0, true);
}

void KDevProjectManagerPart::removeFile(const QString &fileName)
{
    kdDebug(9000) << "KDevProjectManagerPart::removeFile" << fileName << endl;
    
    removeFiles(QStringList() << fileName);
}

QStringList KDevProjectManagerPart::fileList(ProjectItemDom item)
{
    QStringList files;
    
    if (ProjectFolderDom folder = item->toFolder()) {
        ProjectFolderList folder_list = folder->folderList();
        for (ProjectFolderList::Iterator it = folder_list.begin(); it != folder_list.end(); ++it)
            files += fileList((*it)->toItem());
            
        ProjectTargetList target_list = folder->targetList();
        for (ProjectTargetList::Iterator it = target_list.begin(); it != target_list.end(); ++it)
            files += fileList((*it)->toItem());
            
        ProjectFileList file_list = folder->fileList();
        for (ProjectFileList::Iterator it = file_list.begin(); it != file_list.end(); ++it)
            files += fileList((*it)->toItem());
    } else if (ProjectTargetDom target = item->toTarget()) {
        ProjectFileList file_list = target->fileList();
        for (ProjectFileList::Iterator it = file_list.begin(); it != file_list.end(); ++it)
            files += fileList((*it)->toItem());
    } else if (ProjectFileDom file = item->toFile()) {
        QString fileName = file->name();
        if (fileName.startsWith(m_projectDirectory))
            fileName = fileName.mid(m_projectDirectory.length());
            
        while (!fileName.isEmpty() && fileName.at(0) == '/')
            fileName = fileName.mid(1);
            
        files.append(fileName);
    }
    
    return files;
}

void KDevProjectManagerPart::fileDirty(const QString &fileName)
{
    Q_UNUSED(fileName);
}

void KDevProjectManagerPart::fileDeleted(const QString &fileName)
{
    Q_UNUSED(fileName);
}

void KDevProjectManagerPart::fileCreated(const QString &fileName)
{
    Q_UNUSED(fileName);
}

bool KDevProjectManagerPart::computeChanges(const QStringList &oldFileList, const QStringList &newFileList)
{
    QMap<QString, bool> oldFiles, newFiles;
    
    for (QStringList::ConstIterator it = oldFileList.begin(); it != oldFileList.end(); ++it)
        oldFiles.insert(*it, true);
        
    for (QStringList::ConstIterator it = newFileList.begin(); it != newFileList.end(); ++it)
        newFiles.insert(*it, true);
        
    // created files: oldFiles - newFiles
    for (QStringList::ConstIterator it = oldFileList.begin(); it != oldFileList.end(); ++it)
        newFiles.remove(*it);        
        
    // removed files: newFiles - oldFiles
    for (QStringList::ConstIterator it = newFileList.begin(); it != newFileList.end(); ++it)
        oldFiles.remove(*it); 
        

    if (!newFiles.isEmpty())
        emit addedFilesToProject(newFiles.keys());
        
    if (!oldFiles.isEmpty())
        emit removedFilesFromProject(oldFiles.keys());       

    m_dirty = !(newFiles.isEmpty() && oldFiles.isEmpty());
    return m_dirty;
}

#include "kdevprojectmanager_part.moc"
