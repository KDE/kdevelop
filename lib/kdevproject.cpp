/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>

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

#include <QFileInfo>
#include <QSet>
#include <QTimer>

#include <kdebug.h>
#include <kinstance.h>

#include "kdevproject.h"
#include <QtDBus/QtDBus>
#include "kdevfilemanager.h"
#include "kdevprojectmodel.h"
#include "filetemplate.h"

class KDevProjectPrivate
{
public:
    KDevFileManager* manager;
};

KDevProject::KDevProject(KInstance* instance, QObject *parent)
    : KDevPlugin(instance, parent)
    , d(new KDevProjectPrivate())
{
    QDBus::sessionBus().registerObject("/org/kdevelop/Project",
                                       this, QDBusConnection::ExportSlots);

    d->manager = 0;
}

KDevProject::~KDevProject()
{
    delete d;
}

bool KDevProject::inProject( const KUrl& url ) const
{
  KDevProjectFolderItem *top = d->manager->top();
  KUrl u = top->url();
  if (u.protocol() != url.protocol() || u.host() != url.host())
    return false;
  QTime time;
  time.start();
  bool found = false;
  bool exit = false;
  while (top && !found && !exit)
  {
    u = top->url();
    if (u.isParentOf(url))
    {
      KDevProjectFolderItem *parent = 0L;
      QList<KDevProjectFolderItem*> folder_list = top->folderList();
      foreach (KDevProjectFolderItem *folder, folder_list)
      {
        if (folder->url().isParentOf(url))
        {
          parent = folder;
          break;
        }
      }
      if (!parent) //the subfolders are not parent of url
      {
        QList<KDevProjectFileItem*> file_list= top->fileList();
        foreach (KDevProjectFileItem *file, file_list)
        {
          if (file->url() == url)
          {
            //return true; //we found it -- commented out due to benchmarking
            found = true;
            break;
          }
        }
        exit = true;
        //return false; //not in the project -- commented out due to benchmarking
      }
      top = parent;
    }
  }
  kDebug(9000) << "Search in model for url: " << time.elapsed() << endl;

  return found;

#if 0
  /** Benchmark the search using a QHash. */  

//store the items in a QHash
  QHash<QString, KDevProjectFileItem*> items;
  QList<KDevProjectFileItem*> files = const_cast<KDevProject*>(this)->allFiles();
  KDevProjectFileItem *file;
  foreach (file, files)
  {
    items.insert(file->url(), file);
  }
  
  time.restart();
  if (items.contains(url.url()))
  {
    kDebug(9000) << "Search in model for url using QHash: " << time.elapsed() << endl;
    return true;
  }
  kDebug(9000) << "Search in model for url using QHash: " << time.elapsed() << endl;
  return false;
#endif  
}

KUrl KDevProject::relativeUrl( const KUrl& absolute ) const
{
    return KUrl::relativeUrl(projectDirectory(), absolute);
}

KUrl KDevProject::absoluteUrl( const KUrl & relativeUrl ) const
{
    if (KUrl::isRelativeUrl(relativeUrl.path()))
        return KUrl(projectDirectory(), relativeUrl.path());

    return relativeUrl;
}

KDevFileManager* KDevProject::fileManager() const
{
	return d->manager;
}

void KDevProject::setFileManager( KDevFileManager* newManager )
{
	d->manager = newManager;
}

#include "kdevproject.moc"
