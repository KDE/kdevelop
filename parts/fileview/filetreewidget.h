/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Mario Scalas (VCS Support)                      *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILETREEWIDGET_H_
#define _FILETREEWIDGET_H_

#include <qpointer.h>
#include <kfiletreeview.h>

#include <kdevversioncontrol.h>

class FileViewPart;
class FileTreeViewWidgetImpl;
class KDevVersionControl;

/**
* This is FileTree widget for listing files belonging to the project. It does feature:
*   - dynamic updates reflecting the state of the project dir and subdirs
*   - VCS support for showing VCS fields like state, working and repository revisions
*   - bolding the filenames belonging to project to distinguish them from the others
*   - dynamic filtering so the user has not to care about temporary files eating screen space ;-)
*
* Design notes
* The class uses two different implementations (referred by m_impl data member):
* - @see VCSFileTreeWidgetImpl for VCS support
*   VCS support is detencted by the constructor looking for the @see KDevPlugin::versionControl() member:
*   if the current VCS plug-in does offer a @see KDevVCSFileInfoProvider object than this will be used for
*   querying about files' data. If neither VCS plugin nor valid info provider is found then the filetreeview
*   will fallback to the standard implementation
* - @see StdFileTreeWidgetImpl for standard visualization, just like the KFileTreeView
*
* Each implementation must provide a branch item factory which the file filetree will delegate the creation
* of specific KFileTreeViewItem-derived objects: currently they are both defined in the same .h/.cpp files
* of the implementations listed above.
*
*/
class FileTreeWidget : public KFileTreeView
{
    Q_OBJECT
public:
    FileTreeWidget( FileViewPart *part, QWidget *parent, KDevVCSFileInfoProvider *infoProvider );
    virtual ~FileTreeWidget();

    void openDirectory(const QString &dirName);
    bool shouldBeShown( KFileTreeViewItem* item );

    QString projectDirectory();
    QStringList projectFiles();

    FileViewPart *part() const { return m_part; }

    //KDevVCSFileInfoProvider *vcsFileInfoProvider() const;
    void applyHidePatterns( const QString &hidePatterns );
    QString hidePatterns() const;

    bool showNonProjectFiles() const;

public slots:
    void hideOrShow();

private slots:
    void slotItemExecuted(Q3ListViewItem *item);
    void slotContextMenu(KListView *, Q3ListViewItem *item, const QPoint &p);

    void addProjectFiles( QStringList const & fileList, bool constructing = false );
    void removeProjectFiles( QStringList const & fileList );

    //! We must guard against unloading the used VCS plug-in when using it: we
    //! fall back to the implementation (a file view without VCS fields, only filenames)
    void slotImplementationInvalidated();

private:
    bool matchesHidePattern(const QString &fileName);
    KDevVersionControl *versionControl() const;

    QStringList m_hidePatterns;
    QStringList m_projectFiles;

    FileViewPart *m_part;
    KFileTreeBranch *m_rootBranch;
    QPointer<FileTreeViewWidgetImpl> m_impl;
};

#endif
