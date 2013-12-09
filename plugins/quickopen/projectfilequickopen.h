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
#include <language/duchain/indexedstring.h>

namespace KDevelop {
class IProject;
}

class QIcon;

/**
 * Internal data class for the BaseFileDataProvider and ProjectFileData.
 */
struct ProjectFile
{
    // string representation of the url
    QString pathOrUrl;
    // project root folder url
    // NOTE: this is using a QUrl to save memory
    //       if you want a string representation of
    //       this url, use the non-broken KUrl api via:
    //       KUrl(QUrl).pathOrUrl() or similar.
    QUrl projectUrl;
    // indexed url - only set for project files
    // currently open documents don't use this!
    KDevelop::IndexedString indexedUrl;
};

inline bool operator<(const ProjectFile& left, const ProjectFile& right)
{
    return left.pathOrUrl < right.pathOrUrl;
}

Q_DECLARE_TYPEINFO(ProjectFile, Q_MOVABLE_TYPE);

/**
 * The shared data class that is used by the quick open model.
 */
class ProjectFileData : public KDevelop::QuickOpenDataBase
{
public:
    ProjectFileData( const ProjectFile& file );

    virtual QString text() const;
    virtual QString htmlDescription() const;

    bool execute( QString& filterText );

    virtual bool isExpandable() const;
    virtual QWidget* expandingWidget() const;

    virtual QIcon icon() const;

    QList<QVariant> highlighting() const;

    QString project() const;

private:
    ProjectFile m_file;
};

typedef KDevelop::FilterWithSeparator<ProjectFile> Base;

class BaseFileDataProvider : public KDevelop::QuickOpenDataProviderBase, public Base, public KDevelop::QuickOpenFileSetInterface
{
    Q_OBJECT
public:
    BaseFileDataProvider();
    virtual void setFilterText( const QString& text );
    virtual uint itemCount() const;
    virtual uint unfilteredItemCount() const;
    virtual KDevelop::QuickOpenDataPointer data( uint row ) const;

    //Reimplemented from Base<..>
    virtual QString itemText( const ProjectFile& data ) const;
};

/**
 * QuickOpen data provider for file-completion using project-files.
 *
 * It provides all files from all open projects except currently opened ones.
 */
class ProjectFileDataProvider : public BaseFileDataProvider
{
    Q_OBJECT
public:
    ProjectFileDataProvider();
    virtual void reset();
    virtual QSet<KDevelop::IndexedString> files() const;

private slots:
    void projectClosing( KDevelop::IProject* );
    void projectOpened( KDevelop::IProject* );
    void fileAddedToSet( KDevelop::IProject*, const KDevelop::IndexedString& );
    void fileRemovedFromSet( KDevelop::IProject*, const KDevelop::IndexedString& );

private:
    // project files sorted by their url
    // this is done so we can limit ourselves to a relatively fast
    // filtering without any expensive sorting in reset().
    QList<ProjectFile> m_projectFiles;
};

/**
 * Quick open data provider for currently opened documents.
 */
class OpenFilesDataProvider : public BaseFileDataProvider
{
    Q_OBJECT
public:
    virtual void reset();
    virtual QSet<KDevelop::IndexedString> files() const;
};

#endif

