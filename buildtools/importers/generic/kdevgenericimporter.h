/* This file is part of KDevelop
    Copyright (C) 2004,2005 Roberto Raggi <roberto@kdevelop.org>

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
#ifndef KDEVGENERICIMPORTER_H
#define KDEVGENERICIMPORTER_H

#include <kdevfilemanager.h>
#include <qstringlist.h>

class QFileInfo;

class KDevGenericImporter: public Koncrete::FileManager
{
    Q_OBJECT
public:
    KDevGenericImporter(QObject *parent = 0,
                        const QStringList &args = QStringList());
    virtual ~KDevGenericImporter();

//
// KDevFileManager interface
//
    virtual Features features() const
    { return Features(Folders | Files); }

    virtual Koncrete::ProjectFolderItem* addFolder(const KUrl& folder, Koncrete::ProjectFolderItem *parent);
    virtual Koncrete::ProjectFileItem* addFile(const KUrl& file, Koncrete::ProjectFolderItem *parent);
    virtual bool removeFolder(Koncrete::ProjectFolderItem *folder);
    virtual bool removeFile(Koncrete::ProjectFileItem *file);

    virtual Koncrete::Project *project() const;
    virtual QList<Koncrete::ProjectFolderItem*> parse(Koncrete::ProjectFolderItem *item);
    virtual Koncrete::ProjectItem *import(Koncrete::ProjectModel *model, const KUrl &fileName);

private:
    bool isValid(const QFileInfo &fileName) const;

private:
    Koncrete::Project *m_project;

    QStringList includes;
    QStringList excludes;
};

#endif // KDEVGENERICIMPORTER_H
