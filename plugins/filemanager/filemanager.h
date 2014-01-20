/***************************************************************************
 *   Copyright 2006 Alexander Dymo <adymo@kdevelop.org>                    *
 *   Copyright 2006 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef KDEVPLATFORM_PLUGIN_FILEMANAGER_H
#define KDEVPLATFORM_PLUGIN_FILEMANAGER_H

#include <QWidget>
#include <KFileItem>

class KActionCollection;
class KDirOperator;
class KUrlNavigator;
class QAction;
class QString;
class QMenu;
class KUrl;
class KDevFileManagerPlugin;
class BookmarkHandler;

class FileManager: public QWidget {
    Q_OBJECT
public:
    FileManager(KDevFileManagerPlugin *plugin, QWidget* parent);
    QList<QAction*> toolBarActions() const;
    KActionCollection* actionCollection() const;
    KDirOperator* dirOperator() const;
    KDevFileManagerPlugin* plugin() const;

private slots:
    void openFile(const KFileItem&);
    void gotoUrl(const KUrl&);
    void updateNav( const KUrl& url );
    void syncCurrentDocumentDirectory();
    void fillContextMenu(KFileItem item, QMenu *menu);
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
    KDevFileManagerPlugin *m_plugin;
};

#endif
