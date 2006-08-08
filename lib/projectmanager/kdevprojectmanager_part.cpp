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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kdevprojectmanager_part.h"
#include "kdevprojectmanagerdelegate.h"
#include "kdevprojectmodel.h"
#include "kdevprojectmanager.h"
#include "kdevcore.h"
#include "kdevconfig.h"
#include "kdevplugin.h"
#include "kdevfilemanager.h"
#include "kdevbuildmanager.h"
#include "kdevprojectbuilder.h"
#include "kdevprojectfilter.h"
#include "importprojectjob.h"

#include <kfiltermodel.h>
#include <kdevcore.h>
#include <kdevdocumentcontroller.h>
#include <kdevmainwindow.h>
#include <kservicetypetrader.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kaboutdata.h>

#include <kparts/componentfactory.h>

#include <QtGui/QVBoxLayout>
#include <QDir>
#include <qfileinfo.h>
#include <QTimer>
#include <QLineEdit>

typedef KGenericFactory<KDevProjectManagerPart> KDevProjectManagerFactory;
K_EXPORT_COMPONENT_FACTORY(kdevprojectmanager, KDevProjectManagerFactory("kdevprojectmanager"))

KDevProjectManagerPart::KDevProjectManagerPart(QObject *parent, const QStringList&)
  : KDevProject(KDevProjectManagerFactory::instance(), parent)
{
  m_workspace = 0;
  m_projectModel = new KDevProjectModel(this);
  m_dirty = false;

  setInstance(KDevProjectManagerFactory::instance());

  //load the importers
  KService::List managerList = KServiceTypeTrader::self()->query("KDevelop/FileManager");
  KService::List::Iterator it, itEnd = managerList.end();
  for (it = managerList.begin(); it != itEnd; ++it)
  {
      KService::Ptr ptr = *it;

      int error = 0;
      if (KDevFileManager *i = KService::createInstance<KDevFileManager>(ptr, this,
          QStringList(), &error))
      {
          m_importers.insert(ptr->name(), i);
      }
      else
          kDebug(9000) << "error:" << error << endl;
  }

  // load the builders
  KService::List builderList = KServiceTypeTrader::self()->query("KDevelop/ProjectBuilder");
  itEnd = builderList.end();
  for (it = builderList.begin(); it != itEnd; ++it)
  {
      KService::Ptr ptr = *it;

      int error = 0;
      if (KDevProjectBuilder *i = KService::createInstance<KDevProjectBuilder>(ptr, this,
          QStringList(), &error))
      {
          m_builders.insert(ptr->name(), i);
      }
      else
         kDebug(9000) << "error:" << error << endl;
  }

  m_widget = new QWidget(0);
  QVBoxLayout *vbox = new QVBoxLayout(m_widget);
  vbox->setMargin(0);

#if 0
  QLineEdit *editor = new QLineEdit(m_widget);
  vbox->addWidget(editor);
  editor->hide();
#endif

  KDevProjectManagerDelegate *delegate = new KDevProjectManagerDelegate(this);

  QAbstractItemModel *overviewModel = m_projectModel;
#ifdef USE_KFILTER_MODEL
  overviewModel = new KDevProjectOverviewFilter(m_projectModel, this);
#endif

  m_projectOverview = new KDevProjectManager(this, m_widget);
  m_projectOverview->setModel(overviewModel);
  m_projectOverview->setItemDelegate(delegate);
  m_projectOverview->setWhatsThis(i18n("Project Overview"));
  vbox->addWidget(m_projectOverview);

//   connect(m_projectOverview, SIGNAL(activateURL(KUrl)), this, SLOT(openURL(KUrl)));
  connect(m_projectOverview, SIGNAL(pressed(QModelIndex)),
          this, SLOT(pressed(QModelIndex)));



#ifdef WITH_PROJECT_DETAILS
  QAbstractItemModel *detailsModel = m_projectModel;
#ifdef USE_KFILTER_MODEL
  detailsModel = new KDevProjectDetailsFilter(m_projectModel, this);
#endif

  m_projectDetails = new KDevProjectManager(this, m_widget);
  m_projectDetails->setModel(detailsModel);
  m_projectDetails->setItemDelegate(delegate);
  m_projectDetails->setWhatsThis(i18n("Project Details"));
  vbox->add(m_projectDetails);

//   connect(m_projectDetails, SIGNAL(activateURL(KUrl)), this, SLOT(openURL(KUrl)));
  connect(m_projectDetails, SIGNAL(pressed(QModelIndex)),
          this, SLOT(pressed(QModelIndex)));
  connect(m_projectOverview->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
          m_projectDetails, SLOT(setRootIndex(QModelIndex)));
#endif

  KDevCore::mainWindow()->embedSelectViewRight(m_widget, tr("Project Manager"), tr("Project Manager"));

  setXMLFile("kdevprojectmanager.rc");

  m_updateProjectTimer = new QTimer(this);
  connect(m_updateProjectTimer, SIGNAL(timeout()), this, SLOT(updateProjectTimeout()));
}

KDevProjectManagerPart::~KDevProjectManagerPart()
{
  if (m_projectOverview)
    {
      KDevCore::mainWindow()->removeView(m_widget);
      delete m_widget;
      m_widget = 0;
    }
}

void KDevProjectManagerPart::openURL(const KUrl &url)
{
  KDevCore::documentController()->editDocument(url);
}

KDevProjectFolderItem *KDevProjectManagerPart::activeFolder()
{
  return m_projectOverview->currentFolderItem();
}

KDevProjectTargetItem *KDevProjectManagerPart::activeTarget()
{
  return m_projectOverview->currentTargetItem();
}

KDevProjectFileItem * KDevProjectManagerPart::activeFile()
{
  return m_projectOverview->currentFileItem();
}

void KDevProjectManagerPart::updateProjectTimeout()
{
  import();
}

void KDevProjectManagerPart::openProject(const KUrl &dirName, const QString &projectName)
{
  m_projectDirectory = dirName;
  m_projectName = projectName;
  import(ForceRefresh);
}

void KDevProjectManagerPart::import(RefreshPolicy policy)
{

  QStringList oldFileList = fileList();

  if (m_workspace)
    m_projectModel->removeItem(m_workspace);
  KDevFileManager* manager = defaultImporter();
  setFileManager( manager );
  KDevProjectItem* item = manager->import(m_projectModel, m_projectDirectory);
  m_workspace = item ? item->folder() : 0;
  if ( m_workspace != 0 )
  {
      m_projectModel->appendItem(m_workspace);
  }

  Q_ASSERT(m_workspace != 0);

  ImportProjectJob *job = ImportProjectJob::importProjectJob(m_workspace, manager);
  connect(job, SIGNAL(result(KJob*)), this, SIGNAL(refresh()));
  job->start();

  QStringList newFileList = fileList();

  bool hasChanges = computeChanges(oldFileList, newFileList);

  if ((hasChanges && policy == Refresh) || policy == ForceRefresh)
    emit refresh();
}

void KDevProjectManagerPart::closeProject()
{
}

KUrl KDevProjectManagerPart::projectDirectory() const
{
  return m_projectDirectory;
}

QString KDevProjectManagerPart::projectName() const
{
  return m_projectName;
}

QList<KDevProjectFileItem*> KDevProjectManagerPart::allFiles()
{
  if (!m_workspace)
  return QList<KDevProjectFileItem*>();

  return recurseFiles(m_workspace);
}

QList<KDevProjectFileItem*> KDevProjectManagerPart::recurseFiles(KDevProjectItem *item)
{
  QList<KDevProjectFileItem*> files;

  if (KDevProjectFolderItem *folder = item->folder())
  {
    QList<KDevProjectFolderItem*> folder_list = folder->folderList();
    for (QList<KDevProjectFolderItem*>::Iterator it = folder_list.begin(); it != folder_list.end(); ++it)
      files += recurseFiles((*it));

    QList<KDevProjectTargetItem*> target_list = folder->targetList();
    for (QList<KDevProjectTargetItem*>::Iterator it = target_list.begin(); it != target_list.end(); ++it)
      files += recurseFiles((*it));

    QList<KDevProjectFileItem*> file_list = folder->fileList();
    for (QList<KDevProjectFileItem*>::Iterator it = file_list.begin(); it != file_list.end(); ++it)
      files += recurseFiles((*it));
  }
  else if (KDevProjectTargetItem *target = item->target())
  {
    QList<KDevProjectFileItem*> file_list = target->fileList();
    for (QList<KDevProjectFileItem*>::Iterator it = file_list.begin(); it != file_list.end(); ++it)
      files += recurseFiles((*it));
  }
  else if (KDevProjectFileItem *file = item->file())
  {
    files.append(file);
  }
  return files;
}

QStringList KDevProjectManagerPart::fileList()
{
  if (!m_workspace)
    return QStringList();

  KDevProjectItem *dom = m_workspace;
  m_cachedFileList = fileList(dom);

  return m_cachedFileList;
}

KDevFileManager *KDevProjectManagerPart::defaultImporter() const
{
  KConfig * config = KDevConfig::standard();
  config->setGroup( "General Options" );

  QString importer = config->readPathEntry( "Importer", "KDevGenericImporter" );

  if ( m_importers.contains(importer) )
    return m_importers[importer];

  kWarning(9000) << k_funcinfo << "No default importer!" << endl;
  return 0;
}

KDevProjectBuilder *KDevProjectManagerPart::defaultBuilder() const
{
  KDevBuildManager *buildManager;
  buildManager = dynamic_cast<KDevBuildManager*>( fileManager() );
  if ( buildManager )
    return buildManager->builder();
  else
  {
    kDebug(9000) << "not a buildable project" << endl;
    return 0;
  }
}

QStringList KDevProjectManagerPart::fileList(KDevProjectItem *item)
{
  QStringList files;

  if (KDevProjectFolderItem *folder = item->folder())
    {
      QList<KDevProjectFolderItem*> folder_list = folder->folderList();
      for (QList<KDevProjectFolderItem*>::Iterator it = folder_list.begin(); it != folder_list.end(); ++it)
        files += fileList((*it));

      QList<KDevProjectTargetItem*> target_list = folder->targetList();
      for (QList<KDevProjectTargetItem*>::Iterator it = target_list.begin(); it != target_list.end(); ++it)
        files += fileList((*it));

      QList<KDevProjectFileItem*> file_list = folder->fileList();
      for (QList<KDevProjectFileItem*>::Iterator it = file_list.begin(); it != file_list.end(); ++it)
        files += fileList((*it));
    }
  else if (KDevProjectTargetItem *target = item->target())
    {
      QList<KDevProjectFileItem*> file_list = target->fileList();
      for (QList<KDevProjectFileItem*>::Iterator it = file_list.begin(); it != file_list.end(); ++it)
        files += fileList((*it));
    }
  else if (KDevProjectFileItem *file = item->file())
    {
      QString fileName = file->url().fileName();
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
/* FIXME port me!
  if (!newFiles.isEmpty())
    emit addedFilesToProject(newFiles.keys());

  if (!oldFiles.isEmpty())
    emit removedFilesFromProject(oldFiles.keys());
*/
  m_dirty = !(newFiles.isEmpty() && oldFiles.isEmpty());

  return m_dirty;
}

void KDevProjectManagerPart::updateDetails(KDevProjectItem *)
{
}

void KDevProjectManagerPart::pressed( const QModelIndex & index )
{
  if (KDevProjectFileItem *file = m_projectModel->item( index ) ->file())
    KDevCore::documentController() ->editDocument( file ->url() );
}

#include "kdevprojectmanager_part.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;
