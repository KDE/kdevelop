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
#include "kdevprojectmanager_part.h"
#include "kdevprojectmanagerdelegate.h"
#include "kdevprojectmodel.h"
#include "kdevprojectmanager.h"
#include "kdevprojectimporter.h"
#include "kdevprojectbuilder.h"
#include "kdevprojecteditor.h"
#include "importprojectjob.h"

#include <kfiltermodel.h>
#include <kdevcore.h>
#include <kdevpartcontroller.h>
#include <kdevmainwindow.h>
#include <urlutil.h>

#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <ktrader.h>
#include <kaboutdata.h>

#include <kparts/componentfactory.h>

#include <QtGui/QVBoxLayout>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qlineedit.h>

#include <kdevplugininfo.h>

typedef KDevGenericFactory<KDevProjectManagerPart> KDevProjectManagerFactory;
static const KDevPluginInfo data("kdevprojectmanager");
K_EXPORT_COMPONENT_FACTORY(libkdevprojectmanager, KDevProjectManagerFactory(data));

KDevProjectManagerPart::KDevProjectManagerPart(QObject *parent, const char *name, const QStringList&)
    : KDevProject(&data, parent)
{
    setObjectName(QString::fromUtf8(name));

    m_workspace = 0;
    m_projectModel = new KDevProjectModel(this);
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

    m_widget = new QWidget(0);
    new QVBoxLayout(m_widget);

    QLineEdit *editor = new QLineEdit(m_widget);
    m_widget->layout()->addWidget(editor);

    //editor->hide();

    KDevProjectManagerDelegate *delegate = new KDevProjectManagerDelegate(this);

    m_projectManager = new KDevProjectManager(this, m_widget);
    m_projectManager->setModel(m_projectModel);
    m_projectManager->setItemDelegate(delegate);
    m_projectManager->setWhatsThis(i18n("Project overview"));
    m_widget->layout()->add(m_projectManager);

    KFilterModel *filterModel = new KFilterModel(m_projectModel, m_projectModel);
    connect(editor, SIGNAL(textChanged(QString)), filterModel, SLOT(setFilter(QString)));
    m_projectManager->setModel(filterModel);


    connect(m_projectManager, SIGNAL(activateURL(KURL)), this, SLOT(openURL(KURL)));

    mainWindow()->embedSelectViewRight(m_widget, tr("Project Manager"), tr("Project Manager"));

    setXMLFile("kdevprojectmanager.rc");

    m_updateProjectTimer = new QTimer(this);
    connect(m_updateProjectTimer, SIGNAL(timeout()), this, SLOT(updateProjectTimeout()));
}

KDevProjectManagerPart::~KDevProjectManagerPart()
{
    if (m_projectManager) {
        mainWindow()->removeView(m_widget);
        delete m_widget;
    }
}

void KDevProjectManagerPart::openURL(const KURL &url)
{
  kdDebug(9000) << "========> openURL:" << url << endl;
  partController()->editDocument(url);
}

KDevProjectFolderItem *KDevProjectManagerPart::activeFolder()
{
    return m_projectManager->currentFolderItem();
}

KDevProjectTargetItem *KDevProjectManagerPart::activeTarget()
{
    return m_projectManager->currentTargetItem();
}

KDevProjectFileItem * KDevProjectManagerPart::activeFile()
{
    return m_projectManager->currentFileItem();
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
        m_projectModel->removeItem(m_workspace);

    if ((m_workspace = defaultImporter()->import(projectModel(), projectDirectory())->folder()) != 0)
      {
        m_projectModel->appendItem(m_workspace);
      }

    Q_ASSERT(m_workspace != 0);

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

QString KDevProjectManagerPart::activeDirectory() const // ### do we really need it?
{
    if (KDevProjectFolderItem *folder = m_projectManager->currentFolderItem())
        return URLUtil::relativePath(projectDirectory(), folder->name());

    return QString();
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

    KDevProjectItem *dom = m_workspace;
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

QStringList KDevProjectManagerPart::fileList(KDevProjectItem *item)
{
    QStringList files;

    if (KDevProjectFolderItem *folder = item->folder()) {
        QList<KDevProjectFolderItem*> folder_list = folder->folderList();
        for (QList<KDevProjectFolderItem*>::Iterator it = folder_list.begin(); it != folder_list.end(); ++it)
            files += fileList((*it));

        QList<KDevProjectTargetItem*> target_list = folder->targetList();
        for (QList<KDevProjectTargetItem*>::Iterator it = target_list.begin(); it != target_list.end(); ++it)
            files += fileList((*it));

        QList<KDevProjectFileItem*> file_list = folder->fileList();
        for (QList<KDevProjectFileItem*>::Iterator it = file_list.begin(); it != file_list.end(); ++it)
            files += fileList((*it));
    } else if (KDevProjectTargetItem *target = item->target()) {
        QList<KDevProjectFileItem*> file_list = target->fileList();
        for (QList<KDevProjectFileItem*>::Iterator it = file_list.begin(); it != file_list.end(); ++it)
            files += fileList((*it));
    } else if (KDevProjectFileItem *file = item->file()) {
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
