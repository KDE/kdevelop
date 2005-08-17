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
#include "kdevgenericimporter.h"

#include <kdevproject.h>
#include <domutil.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>

const QString &KDevGenericImporter::genericImporter =
    KGlobal::staticQString("/kdevprojectmanager/importer/generic");

K_EXPORT_COMPONENT_FACTORY(libkdevgenericimporter, KGenericFactory<KDevGenericImporter>("kdevgenericimporter"))

KDevGenericImporter::KDevGenericImporter(QObject *parent, const char *name, const QStringList &)
    : KDevProjectEditor(parent, name)
{
    m_project = qobject_cast<KDevProject*>(parent);
    Q_ASSERT(m_project);

    QDomDocument &dom = *project()->projectDom();
    includes = DomUtil::readListEntry(dom, genericImporter, "include");
    excludes = DomUtil::readListEntry(dom, genericImporter, "exclude");

    if (includes.isEmpty())
        includes << "*.h" << "*.cpp" << "*.c";   // ### remove me

    excludes << "CVS" << "moc_*.cpp"; // ### remove me
}

KDevGenericImporter::~KDevGenericImporter()
{
}

KDevProject *KDevGenericImporter::project() const
{
    return m_project;
}

bool KDevGenericImporter::isValid(const QFileInfo &fileInfo) const
{
    QString fileName = fileInfo.fileName();

    bool ok = fileInfo.isDir();
    for (QStringList::ConstIterator it = includes.begin(); !ok && it != includes.end(); ++it) {
        QRegExp rx(*it, true, true);
        if (rx.exactMatch(fileName)) {
            ok = true;
        }
    }

    if (!ok)
        return false;

    for (QStringList::ConstIterator it = excludes.begin(); it != excludes.end(); ++it) {
        QRegExp rx(*it, true, true);
        if (rx.exactMatch(fileName)) {
            return false;
        }
    }

    return true;
}

ProjectFolderList KDevGenericImporter::parse(ProjectFolderDom item)
{
    static const QString &dot = KGlobal::staticQString(".");
    static const QString &dotdot = KGlobal::staticQString("..");

    QDir dir(item->name());

    ProjectTargetDom target = item->projectModel()->create<ProjectTargetModel>();
    target->setName("files");
    item->addTarget(target);

    ProjectFolderList folder_list;
    QFileInfoList entries = dir.entryInfoList();

    for (int i=0; i<entries.count(); ++i) {    
        QFileInfo fileInfo = entries.at(i);

        if (!isValid(fileInfo)) {
            //kdDebug(9000) << "skip:" << fileInfo.absFilePath() << endl;
        } else if (fileInfo.isDir() && fileInfo.fileName() != dot && fileInfo.fileName() != dotdot) {
            ProjectFolderDom folder = item->projectModel()->create<ProjectFolderModel>();
            folder->setName(fileInfo.absFilePath());
            item->addFolder(folder);
            folder_list.append(folder);
        } else if (fileInfo.isFile()) {
            ProjectFileDom file = item->projectModel()->create<ProjectFileModel>();
            file->setName(fileInfo.absFilePath());
            target->addFile(file);
        }
    }

    return folder_list;
}

ProjectItemDom KDevGenericImporter::import(ProjectModel *model, const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    if (fileInfo.isDir()) {
        ProjectFolderDom folder = model->create<ProjectFolderModel>();
        folder->setName(fileName);
        return folder->toItem();
    } else if (fileInfo.isFile()) {
        ProjectFileDom file = model->create<ProjectFileModel>();
        file->setName(fileName);
        return file->toItem();
    }

    return ProjectItemDom();
}

QString KDevGenericImporter::findMakefile(ProjectFolderDom dom) const
{
    Q_UNUSED(dom);
    return QString::null;
}

QStringList KDevGenericImporter::findMakefiles(ProjectFolderDom dom) const
{
    Q_UNUSED(dom);
    return QStringList();
}

bool KDevGenericImporter::addFolder(ProjectFolderDom // folder
                                   , ProjectFolderDom // parent
                                   )
{
    return false;
}

bool KDevGenericImporter::addTarget(ProjectTargetDom // target
                                   , ProjectFolderDom // parent
                                   )
{
    return false;
}

bool KDevGenericImporter::addFile(ProjectFileDom // file
                                 , ProjectFolderDom // parent
                                 )
{
    return false;
}

bool KDevGenericImporter::addFile(ProjectFileDom // file
                                 , ProjectTargetDom // parent
                                 )
{
    return false;
}

bool KDevGenericImporter::removeFolder(ProjectFolderDom // folder
                                      , ProjectFolderDom // parent
                                      )
{
    return false;
}

bool KDevGenericImporter::removeTarget(ProjectTargetDom // target
                                      , ProjectFolderDom // parent
                                      )
{
    return false;
}

bool KDevGenericImporter::removeFile(ProjectFileDom // file
                                    , ProjectFolderDom // parent
                                    )
{
    return false;
}

bool KDevGenericImporter::removeFile(ProjectFileDom // file
                                    , ProjectTargetDom // parent
                                    )
{
    return false;
}

#include "kdevgenericimporter.moc"
