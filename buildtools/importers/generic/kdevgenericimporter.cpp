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

#include <QDir>
#include <QFile>
#include <qfileinfo.h>
#include <QRegExp>

K_EXPORT_COMPONENT_FACTORY(kdevgenericimporter, KGenericFactory<KDevGenericImporter>("kdevgenericimporter"))

KDevGenericImporter::KDevGenericImporter(QObject *parent, const QStringList &)
    : KDevFileManager(parent)
{
    kDebug() << k_funcinfo << "generic importer loaded" << endl;

    if (includes.isEmpty())
        includes = "(*.h|*.cpp|*.c|*.ui)"; // ### remove me

    excludes = "(.svn|CVS|moc_.*.cpp)"; // ### remove me
}

KDevGenericImporter::~KDevGenericImporter()
{
}

KDevProject *KDevGenericImporter::project() const
{
    return KDevApi::self()->project();
}

bool KDevGenericImporter::isValid(const QFileInfo &fileInfo) const
{
    QString fileName = fileInfo.fileName();

    bool ok = fileInfo.isDir();
    QRegExp rx(includes, Qt::CaseSensitive, QRegExp::Wildcard);
    if (rx.exactMatch(fileName)) {
            ok = true;
    }

    if (!ok)
        return false;

    QRegExp rx2(excludes, Qt::CaseSensitive, QRegExp::Wildcard);
    if (rx2.exactMatch(fileName))
        return false;

    return true;
}

QList<KDevProjectFolderItem*> KDevGenericImporter::parse(KDevProjectFolderItem *item)
{
    QDir dir( item->url().toLocalFile() );

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
            KDevProjectFolderItem *folder = new KDevProjectFolderItem(KUrl(fileInfo.absoluteFilePath()), item);
            item->add(folder);
            folder_list.append(folder);
        } else if (fileInfo.isFile()) {
            KDevProjectFileItem *file = new KDevProjectFileItem(KUrl( fileInfo.absoluteFilePath() ), item);
            item->add(file);
        }
    }

    return folder_list;
}

KDevProjectItem *KDevGenericImporter::import(KDevProjectModel *model, const KUrl &fileName)
{
    kDebug(9000) << k_funcinfo << "import filename: " << fileName << endl;

    QFileInfo fileInfo( fileName.path() );
    if (fileInfo.isDir()) {
        KDevProjectFolderItem *folder = new KDevProjectFolderItem(fileName, 0);
        return folder;
    } else if (fileInfo.isFile()) {
        KDevProjectFileItem *file = new KDevProjectFileItem(fileName,0);
        return file;
    }

    return 0;
}

KDevProjectFolderItem* KDevGenericImporter::addFolder(const KUrl&// folder
                                   , KDevProjectFolderItem *// parent
                                   )
{
    return 0;
}


KDevProjectFileItem* KDevGenericImporter::addFile(const KUrl&// file
                                 , KDevProjectFolderItem *// parent
                                 )
{
    return 0;
}

bool KDevGenericImporter::removeFolder(KDevProjectFolderItem *)
{
    return false;
}

bool KDevGenericImporter::removeFile(KDevProjectFileItem *)
{
    return false;
}

#include "kdevgenericimporter.moc"
