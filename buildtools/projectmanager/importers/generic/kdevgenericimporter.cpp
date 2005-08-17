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
#include <kdevprojectmodel.h>

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
    : KDevProjectEditor(parent)
{
    setObjectName(QString::fromUtf8(name));

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

QList<KDevProjectFolderItem*> KDevGenericImporter::parse(KDevProjectFolderItem *item)
{
    static const QString &dot = KGlobal::staticQString(".");
    static const QString &dotdot = KGlobal::staticQString("..");

    QDir dir = item->directory();

    KDevProjectTargetItem *target = new KDevProjectTargetItem("files");
    item->add(target);

    QList<KDevProjectFolderItem*> folder_list;
    QFileInfoList entries = dir.entryInfoList();

    for (int i=0; i<entries.count(); ++i) {
        QFileInfo fileInfo = entries.at(i);

        if (!isValid(fileInfo)) {
            //kdDebug(9000) << "skip:" << fileInfo.absFilePath() << endl;
        } else if (fileInfo.isDir() && fileInfo.fileName() != dot && fileInfo.fileName() != dotdot) {
            KDevProjectFolderItem *folder = new KDevProjectFolderItem(fileInfo.absFilePath());
            item->add(folder);
            folder_list.append(folder);
        } else if (fileInfo.isFile()) {
            KDevProjectFileItem *file = new KDevProjectFileItem(fileInfo);
            target->add(file);
        }
    }

    return folder_list;
}

KDevProjectItem *KDevGenericImporter::import(KDevProjectModel *model, const QString &fileName)
{
    kdDebug(9000) << "ROBE: ========================================= import filename: " << fileName << endl;

    QFileInfo fileInfo(fileName);
    if (fileInfo.isDir()) {
        KDevProjectFolderItem *folder = new KDevProjectFolderItem(fileInfo.absFilePath());
        kdDebug(9000) << "ROBE: create a directory ================================== " << fileInfo.absFilePath() << endl;
        return folder;
    } else if (fileInfo.isFile()) {
        kdDebug(9000) << "ROBE: create a file ================================== " << fileInfo.absFilePath() << endl;

        KDevProjectFileItem *file = new KDevProjectFileItem(fileInfo.absFilePath());
        return file;
    } else {
        kdDebug(9000) << "ROBE: skip ================================== " << fileInfo.absFilePath() << endl;
    }

    return 0;
}

QString KDevGenericImporter::findMakefile(KDevProjectFolderItem *dom) const
{
    Q_UNUSED(dom);
    return QString::null;
}

QStringList KDevGenericImporter::findMakefiles(KDevProjectFolderItem *dom) const
{
    Q_UNUSED(dom);
    return QStringList();
}

bool KDevGenericImporter::addFolder(KDevProjectFolderItem *// folder
                                   , KDevProjectFolderItem *// parent
                                   )
{
    return false;
}

bool KDevGenericImporter::addTarget(KDevProjectTargetItem *// target
                                   , KDevProjectFolderItem *// parent
                                   )
{
    return false;
}

bool KDevGenericImporter::addFile(KDevProjectFileItem *// file
                                 , KDevProjectFolderItem *// parent
                                 )
{
    return false;
}

bool KDevGenericImporter::addFile(KDevProjectFileItem *// file
                                 , KDevProjectTargetItem *// parent
                                 )
{
    return false;
}

bool KDevGenericImporter::removeFolder(KDevProjectFolderItem *// folder
                                      , KDevProjectFolderItem *// parent
                                      )
{
    return false;
}

bool KDevGenericImporter::removeTarget(KDevProjectTargetItem *// target
                                      , KDevProjectFolderItem *// parent
                                      )
{
    return false;
}

bool KDevGenericImporter::removeFile(KDevProjectFileItem *// file
                                    , KDevProjectFolderItem *// parent
                                    )
{
    return false;
}

bool KDevGenericImporter::removeFile(KDevProjectFileItem *// file
                                    , KDevProjectTargetItem *// parent
                                    )
{
    return false;
}

#include "kdevgenericimporter.moc"
