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

K_EXPORT_COMPONENT_FACTORY(kdevgenericimporter, KGenericFactory<KDevGenericImporter>("kdevgenericimporter"))

KDevGenericImporter::KDevGenericImporter(QObject *parent, const char *name, const QStringList &)
    : KDevProjectEditor(parent)
{
    setObjectName(QString::fromUtf8(name));

    m_project = qobject_cast<KDevProject*>(parent);
    Q_ASSERT(m_project);

    QDomDocument &dom = *project()->projectDom();
    includes = DomUtil::readListEntry(dom, QLatin1String("/kdevprojectmanager/importer/generic"), "include");
    excludes = DomUtil::readListEntry(dom, QLatin1String("/kdevprojectmanager/importer/generic"), "exclude");

    if (includes.isEmpty())
        includes << "*.h" << "*.cpp" << "*.c" << "*.ui";   // ### remove me

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
        QRegExp rx(*it, Qt::CaseSensitive, QRegExp::Wildcard);
        if (rx.exactMatch(fileName)) {
            ok = true;
        }
    }

    if (!ok)
        return false;

    for (QStringList::ConstIterator it = excludes.begin(); it != excludes.end(); ++it) {
        QRegExp rx(*it, Qt::CaseSensitive, QRegExp::Wildcard);
        if (rx.exactMatch(fileName)) {
            return false;
        }
    }

    return true;
}

QList<KDevProjectFolderItem*> KDevGenericImporter::parse(KDevProjectFolderItem *item)
{
    QDir dir = item->directory();

#if 0 // ### port me
    KDevProjectTargetItem *target = new KDevProjectTargetItem("files");
    item->add(target);
#endif

    QList<KDevProjectFolderItem*> folder_list;
    QFileInfoList entries = dir.entryInfoList();

    for (int i=0; i<entries.count(); ++i) {
        QFileInfo fileInfo = entries.at(i);

        if (!isValid(fileInfo)) {
            //kDebug(9000) << "skip:" << fileInfo.absoluteFilePath() << endl;
        } else if (fileInfo.isDir() && fileInfo.fileName() != QLatin1String(".")
                   && fileInfo.fileName() != QLatin1String("..")) {
            KDevProjectFolderItem *folder = new KDevProjectFolderItem(fileInfo.absoluteFilePath());
            item->add(folder);
            folder_list.append(folder);
        } else if (fileInfo.isFile()) {
            KDevProjectFileItem *file = new KDevProjectFileItem(fileInfo);
            item->add(file);
        }
    }

    return folder_list;
}

KDevProjectItem *KDevGenericImporter::import(KDevProjectModel *model, const QString &fileName)
{
//     kDebug(9000) << "ROBE: ========================================= import filename: " << fileName << endl;

    QFileInfo fileInfo(fileName);
    if (fileInfo.isDir()) {
        KDevProjectFolderItem *folder = new KDevProjectFolderItem(fileInfo.absoluteFilePath());
//         kDebug(9000) << "ROBE: create a directory ================================== " << fileInfo.absoluteFilePath() << endl;
        return folder;
    } else if (fileInfo.isFile()) {
//         kDebug(9000) << "ROBE: create a file ================================== " << fileInfo.absoluteFilePath() << endl;

        KDevProjectFileItem *file = new KDevProjectFileItem(fileInfo.absoluteFilePath());
        return file;
    } else {
//         kDebug(9000) << "ROBE: skip ================================== " << fileInfo.absoluteFilePath() << endl;
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
