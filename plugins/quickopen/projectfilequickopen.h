/* This file is part of the KDE libraries
   Copyright (C) 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef PROJECT_FILE_QUICKOPEN
#define PROJECT_FILE_QUICKOPEN

#include <language/interfaces/quickopendataprovider.h>
#include <language/interfaces/quickopenfilter.h>
#include <serialization/indexedstring.h>

#include <util/path.h>

namespace KDevelop {
class IProject;
class ProjectFileItem;
}

/**
 * Internal data class for the BaseFileDataProvider and ProjectFileData.
 */
struct ProjectFile
{
    ProjectFile()
    {}
    KDevelop::Path path;
    // project root folder url
    KDevelop::Path projectPath;
    // indexed url - only set for project files
    // currently open documents don't use this!
    KDevelop::IndexedString indexedPath;
    // true for files which reside outside of the project root
    // this happens e.g. for generated files in out-of-source build folders
    bool outsideOfProject = false;
};

inline bool operator<(const ProjectFile& left, const ProjectFile& right)
{
    if (left.outsideOfProject != right.outsideOfProject) {
        return !left.outsideOfProject;
    }
    return left.path < right.path;
}

Q_DECLARE_TYPEINFO(ProjectFile, Q_MOVABLE_TYPE);

/**
 * The shared data class that is used by the quick open model.
 */
class ProjectFileData
    : public KDevelop::QuickOpenDataBase
{
public:
    explicit ProjectFileData(const ProjectFile& file);

    QString text() const override;
    QString htmlDescription() const override;

    bool execute(QString& filterText) override;

    bool isExpandable() const override;
    QWidget* expandingWidget() const override;

    QIcon icon() const override;

    QList<QVariant> highlighting() const override;

    QString project() const;

    KDevelop::Path projectPath() const;
private:
    ProjectFile m_file;
};

class BaseFileDataProvider
    : public KDevelop::QuickOpenDataProviderBase
    , public KDevelop::PathFilter<ProjectFile, BaseFileDataProvider>
    , public KDevelop::QuickOpenFileSetInterface
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::QuickOpenFileSetInterface)

public:
    BaseFileDataProvider();
    void setFilterText(const QString& text) override;
    uint itemCount() const override;
    uint unfilteredItemCount() const override;
    KDevelop::QuickOpenDataPointer data(uint row) const override;

    inline KDevelop::Path itemPath(const ProjectFile& data) const
    {
        return data.path;
    }

    inline KDevelop::Path itemPrefixPath(const ProjectFile& data) const
    {
        return data.projectPath;
    }
};

/**
 * QuickOpen data provider for file-completion using project-files.
 *
 * It provides all files from all open projects except currently opened ones.
 */
class ProjectFileDataProvider
    : public BaseFileDataProvider
{
    Q_OBJECT
public:
    ProjectFileDataProvider();
    void reset() override;
    QSet<KDevelop::IndexedString> files() const override;
private Q_SLOTS:
    void projectClosing(KDevelop::IProject*);
    void projectOpened(KDevelop::IProject*);
    void fileAddedToSet(KDevelop::ProjectFileItem*);
    void fileRemovedFromSet(KDevelop::ProjectFileItem*);
private:
    // project files sorted by their url
    // this is done so we can limit ourselves to a relatively fast
    // filtering without any expensive sorting in reset().
    QVector<ProjectFile> m_projectFiles;
};

/**
 * Quick open data provider for currently opened documents.
 */
class OpenFilesDataProvider
    : public BaseFileDataProvider
{
    Q_OBJECT
public:
    void reset() override;
    QSet<KDevelop::IndexedString> files() const override;
};

#endif

