/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef PROJECT_FILE_QUICKOPEN
#define PROJECT_FILE_QUICKOPEN

#include <language/interfaces/quickopendataprovider.h>
#include <language/interfaces/quickopenfilter.h>
#include <serialization/indexedstring.h>

#include <util/path.h>

#include <utility>
#include <vector>

namespace KDevelop {
class IProject;
class ProjectFileItem;
}

/**
 * Internal data class for the BaseFileDataProvider and ProjectFileData.
 */
struct ProjectFile
{
    ProjectFile() = default;
    explicit ProjectFile(const KDevelop::ProjectFileItem*);

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

inline void swap(ProjectFile& a, ProjectFile& b) noexcept
{
    using std::swap;
    swap(a.path, b.path);
    swap(a.projectPath, b.projectPath);
    swap(a.indexedPath, b.indexedPath);
    swap(a.outsideOfProject, b.outsideOfProject);
}

inline bool operator<(const ProjectFile& left, const ProjectFile& right)
{
    if (left.outsideOfProject != right.outsideOfProject) {
        // place the less interesting generated files at the end
        return !left.outsideOfProject;
    }
    const int comparison = left.path.compare(right.path, Qt::CaseInsensitive);
    if (comparison != 0) {
        return comparison < 0;
    }
    // Only paths that are completely, case-sensitively equal are considered
    // duplicates. Comparing indexed paths here when the paths are
    // case-insensitively equal ensures that:
    // * the duplicates are adjacent and thus detected by std::unique;
    // * binary search algorithms find only case-sensitively equal elements.
    // OpenFilesDataProvider default-initializes all its indexed paths making
    // them all equal to each other. This is fine because OpenFilesDataProvider
    // doesn't use std::unique or binary search algorithms.
    return left.indexedPath < right.indexedPath;
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
    const ProjectFile m_file;
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
    std::vector<ProjectFile> m_projectFiles;
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

