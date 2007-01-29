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

#include <kdevcore.h>
#include <kdevproject.h>
#include <domutil.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>

#include <QDir>
#include <QFile>
#include <qfileinfo.h>
#include <QRegExp>

typedef KGenericFactory<KDevGenericImporter> GenericSupportFactory;
K_EXPORT_COMPONENT_FACTORY(kdevgenericimporter, GenericSupportFactory("kdevgenericimporter"))

KDevGenericImporter::KDevGenericImporter(QObject *parent, const QStringList &)
    : Koncrete::FileManager( GenericSupportFactory::componentData(), parent)
{
    if (includes.isEmpty())
        includes << "*.h" << "*.cpp" << "*.c" << "*.ui" << "*.cs" << "*.java";   // ### remove me

    excludes << ".svn" << "CVS" << "moc_*.cpp"; // ### remove me
}

KDevGenericImporter::~KDevGenericImporter()
{
}

Koncrete::Project *KDevGenericImporter::project() const
{
    return Koncrete::Core::activeProject();
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

QList<Koncrete::ProjectFolderItem*> KDevGenericImporter::parse(Koncrete::ProjectFolderItem *item)
{
    QDir dir( item->url().toLocalFile() );

#if 0 // ### port me
    KDevProjectTargetItem *target = new KDevProjectTargetItem("files");
    item->add(target);
#endif

    QList<Koncrete::ProjectFolderItem*> folder_list;
    QFileInfoList entries = dir.entryInfoList();

    for (int i=0; i<entries.count(); ++i) {
        QFileInfo fileInfo = entries.at(i);

        if (!isValid(fileInfo)) {
            //kDebug(9000) << "skip:" << fileInfo.absoluteFilePath() << endl;
        }
        else if (fileInfo.isDir() && fileInfo.fileName() != QLatin1String(".")
                   && fileInfo.fileName() != QLatin1String(".."))
        {
            Koncrete::ProjectFolderItem *folder = new Koncrete::ProjectFolderItem(KUrl(fileInfo.absoluteFilePath()), item);
            folder_list.append(folder);
        } else if (fileInfo.isFile())
        {
            Koncrete::ProjectFileItem *file = new Koncrete::ProjectFileItem(KUrl( fileInfo.absoluteFilePath() ), item);
        }
    }

    return folder_list;
}

Koncrete::ProjectItem *KDevGenericImporter::import(Koncrete::ProjectModel *model,
                                                   const KUrl &fileName)
{
    QFileInfo fileInfo( fileName.path() );
    if (fileInfo.isDir()) {
        Koncrete::ProjectFolderItem *folder = new Koncrete::ProjectFolderItem(fileName, 0);
        return folder;
    } else if (fileInfo.isFile()) {
        Koncrete::ProjectFileItem *file = new Koncrete::ProjectFileItem(fileName,0);
        return file;
    }

    return 0;
}

Koncrete::ProjectFolderItem* KDevGenericImporter::addFolder(const KUrl&,
                                                            Koncrete::ProjectFolderItem *)
{
    return 0;
}


Koncrete::ProjectFileItem* KDevGenericImporter::addFile(const KUrl&,
                                                        Koncrete::ProjectFolderItem *)
{
    return 0;
}

bool KDevGenericImporter::removeFolder(Koncrete::ProjectFolderItem *)
{
    return false;
}

bool KDevGenericImporter::removeFile(Koncrete::ProjectFileItem *)
{
    return false;
}

#include "kdevgenericimporter.moc"
