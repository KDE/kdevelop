/*
    SPDX-FileCopyrightText: 2006 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_FILEMANAGER_H
#define KDEVPLATFORM_PLUGIN_FILEMANAGER_H

#include <QWidget>

#include <KFileItem>
#include <KDirOperator>
#include <KUrlNavigator>

class KActionCollection;
class QAction;
class QString;
class QMenu;
class BookmarkHandler;

class FileManager: public QWidget {
    Q_OBJECT
public:
    explicit FileManager(QWidget* parent);
    ~FileManager() override;

    QList<QAction*> toolBarActions() const;
    KActionCollection* actionCollection() const;
    KDirOperator* dirOperator() const;

private Q_SLOTS:
    void fileCreated(KJob *job);
    void openFile(const KFileItem&);
    void gotoUrl(const QUrl&);
    void updateNav( const QUrl& url );
    void syncCurrentDocumentDirectory();
    void fillContextMenu(const KFileItem& item, QMenu* menu);
    void createNewFile();

private:
    void setupActions();
    QList<QAction*> tbActions;
    QAction* newFileAction;
    QList<QAction*> contextActions;
    KDirOperator* dirop;
    KUrlNavigator* urlnav;
    BookmarkHandler *m_bookmarkHandler;
    KActionCollection *m_actionCollection;
};

#endif
