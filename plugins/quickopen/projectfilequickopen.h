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

#include <KIcon>
#include <KUrl>
#include <language/interfaces/quickopendataprovider.h>
#include <language/interfaces/quickopenfilter.h>
#include <language/duchain/indexedstring.h>

namespace KDevelop {
class IProject;
}

struct ProjectFile {
  KDevelop::IndexedString m_url;
  KDevelop::IndexedString m_projectUrl;
  KDevelop::IndexedString m_project;
  KIcon m_icon;
};

Q_DECLARE_TYPEINFO(ProjectFile, Q_PRIMITIVE_TYPE);

class ProjectFileData : public KDevelop::QuickOpenDataBase {
  public:
    ProjectFileData( const ProjectFile& file );
    
    virtual QString text() const;
    virtual QString htmlDescription() const;

    bool execute( QString& filterText );

    virtual bool isExpandable() const;
    virtual QWidget* expandingWidget() const;

    virtual QIcon icon() const;
    
    QList<QVariant> highlighting() const;
    
  private:
    KUrl totalUrl() const;
    
    ProjectFile m_file;
};

/**
 * A QuickOpenDataProvider for file-completion using project-files.
 * It provides all files from all open projects.
 * */

typedef KDevelop::FilterWithSeparator<ProjectFile> Base;

class BaseFileDataProvider : public KDevelop::QuickOpenDataProviderBase, public Base, public KDevelop::QuickOpenFileSetInterface
{
    Q_OBJECT
  public:
    BaseFileDataProvider();
    virtual void setFilterText( const QString& text );
    virtual uint itemCount() const;
    virtual uint unfilteredItemCount() const;
    virtual QList<KDevelop::QuickOpenDataPointer> data( uint start, uint end ) const;

  private:
    //Reimplemented from Base<..>
    virtual QString itemText( const ProjectFile& data ) const;
};

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
    QMap<QByteArray, ProjectFile> m_projectFiles;
};

class OpenFilesDataProvider : public BaseFileDataProvider
{
  Q_OBJECT
public:
    virtual void reset();
    virtual QSet<KDevelop::IndexedString> files() const;
};

#endif

