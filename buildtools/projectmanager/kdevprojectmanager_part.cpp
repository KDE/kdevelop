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

#include <kdevcore.h>
#include <kdevmainwindow.h>

#include <kdirwatch.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <ktrader.h>
#include <kaboutdata.h>

#include <kparts/componentfactory.h>

#include <qwhatsthis.h>
#include <qfileinfo.h>
#include <qtimer.h>

typedef KGenericFactory<KDevProjectManagerPart> KDevProjectManagerFactory;
static const KAboutData data("kdevprojectmanager", I18N_NOOP("KDevelop Project Manager"), "1.0");
K_EXPORT_COMPONENT_FACTORY(libkdevprojectmanager, KDevProjectManagerFactory("kdevprojectmanager"));

KDevProjectManagerPart::KDevProjectManagerPart(QObject *parent, const char *name, const QStringList&)
    : KDevProject("KDevProjectManagerPart", "kdevprojectmanager", parent, name ? name : "KDevProjectManagerPart")
{
    m_projectModel = new ProjectModel();

    
    m_dirWatch = new KDirWatch(this);
    
    setInstance(KDevProjectManagerFactory::instance());
    setXMLFile("kdevpart_kdevprojectmanager.rc");

    m_widget = new KDevProjectManagerWidget(this);

    QWhatsThis::add(m_widget, i18n("Project Manager"));

    mainWindow()->embedSelectViewRight(m_widget, tr("Project Manager"), tr("Project Manager"));

    m_workspace = m_projectModel->create<ProjectWorkspaceModel>();
    m_workspace->setName("Workspace (kdevelop)");
    m_projectModel->addItem(m_workspace->toItem());    
    
    KTrader::OfferList lst = KTrader::self()->query("KDevelop/ProjectImporter");
    
    for (KTrader::OfferList::Iterator it = lst.begin(); it != lst.end(); ++it){
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
    
    connect(m_dirWatch, SIGNAL(dirty(const QString&)),
        this, SLOT(fileDirty(const QString&)));
}

KDevProjectManagerPart::~KDevProjectManagerPart()
{
    if (m_widget) {
        mainWindow()->removeView(m_widget);
        delete m_widget;
    }
}

void KDevProjectManagerPart::openProject(const QString &dirName, const QString &projectName)
{
    m_projectDirectory = dirName;
    m_projectName = projectName;    

    import();
    
    KDevProject::openProject(dirName, projectName);    
}

void KDevProjectManagerPart::import()
{
    QDomDocument &dom = *projectDom();
    QString kind = DomUtil::readEntry(dom, "/general/importer");
    Q_ASSERT(!kind.isEmpty());
    
    if (!m_importers.contains(kind)) {
        kdDebug(9000) << "error: importer not found!" << endl;
        return;
    }
    
    ProjectItemDom projectDom = m_importers[kind]->import(m_workspace->toFolder(), projectDirectory());
    if (ProjectFolderDom folder = projectDom->toFolder()) {
        m_workspace->addFolder(folder);
        QStringList makefileList = m_importers[kind]->findMakefiles(folder);
        for (QStringList::Iterator it = makefileList.begin(); it != makefileList.end(); ++it) {
            kdDebug(9000) << "watch file: " << *it << endl;
            m_dirWatch->addDir(QFileInfo(*it).dirPath(true));
            m_dirWatch->addFile(*it);
        }
    }
    
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
    return m_projectDirectory;
}

QString KDevProjectManagerPart::buildDirectory() const
{
    return m_projectDirectory;
}

QStringList KDevProjectManagerPart::allFiles() const
{
    return const_cast<KDevProjectManagerPart*>(this)->allFiles();
}

QStringList KDevProjectManagerPart::allFiles()
{
    ProjectItemDom dom = m_workspace->toItem();
    return fileList(dom);
}

QStringList KDevProjectManagerPart::distFiles() const
{
    return allFiles();
}

void KDevProjectManagerPart::addFiles(const QStringList &fileList)
{
    kdDebug(9000) << "KDevProjectManagerPart::addFiles:" << fileList << endl;
}

void KDevProjectManagerPart::addFile(const QString &fileName)
{
    kdDebug(9000) << "KDevProjectManagerPart::addFile:" << fileName << endl;
}

void KDevProjectManagerPart::removeFiles(const QStringList &fileList)
{
    kdDebug(9000) << "KDevProjectManagerPart::removeFiles" << fileList << endl;
}

void KDevProjectManagerPart::removeFile(const QString &fileName)
{
    kdDebug(9000) << "KDevProjectManagerPart::removeFile" << fileName << endl;
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
    kdDebug(9000) << "KDevProjectManagerPart::fileDirty:" << fileName << endl;
    import();
}

void KDevProjectManagerPart::fileDeleted(const QString &fileName)
{
    kdDebug(9000) << "KDevProjectManagerPart::fileDeleted:" << fileName << endl;
    
    QFileInfo fileInfo(fileName);
    if (fileInfo.extension() == "am")
        import();
}

void KDevProjectManagerPart::fileCreated(const QString &fileName)
{
    kdDebug(9000) << "KDevProjectManagerPart::fileCreated:" << fileName << endl;
    
    QFileInfo fileInfo(fileName);
    if (fileInfo.extension() == "am")
        import();
}


#include "kdevprojectmanager_part.moc"
