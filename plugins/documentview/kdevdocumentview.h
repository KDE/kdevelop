/* This file is part of KDevelop
   Copyright 2005 Adam Treat <treat@kde.org>
   Copyright 2013 Sebastian Kügler <sebas@kde.org>

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

#ifndef KDEVDOCUMENTVIEW_H
#define KDEVDOCUMENTVIEW_H

#include <QTreeView>

class QSortFilterProxyModel;
class KAction;
class KUrl;
class KDevDocumentViewPlugin;
class KDevDocumentModel;
class KDevDocumentViewDelegate;
class KDevDocumentSelection;
class KDevFileItem;
class KMenu;
namespace KDevelop
{
    class IDocument;
    class IProject;
}

class KDevCategoryItem;
class KDevDocumentModel;
class KDevDocumentItem;

class KDevDocumentView: public QTreeView
{
    Q_OBJECT
public:
    explicit KDevDocumentView( KDevDocumentViewPlugin *plugin, QWidget *parent );
    virtual ~KDevDocumentView();

    KDevDocumentViewPlugin *plugin() const;

signals:
    void activateURL( const KUrl &url );

public slots:
    void opened( KDevelop::IDocument* document );

private slots:
    void activated( KDevelop::IDocument* document );
    void saved( KDevelop::IDocument* document );
    void closed( KDevelop::IDocument* document );
    void contentChanged( KDevelop::IDocument* document );
    void stateChanged( KDevelop::IDocument* document );
    void documentUrlChanged( KDevelop::IDocument* document );
    void updateCategoryItem( KDevCategoryItem *item );
    void updateProjectPaths();

    void saveSelected();
    void reloadSelected();
    void closeSelected();
    void closeUnselected();

protected:
    virtual void mousePressEvent( QMouseEvent * event );
    virtual void contextMenuEvent( QContextMenuEvent * event );

private:
    template<typename F> void visitItems(F, bool selectedItems);
    bool selectedDocHasChanges();
    void updateSelectedDocs();
    void appendActions(QMenu* menu, const QList< QAction* >& actions);

private:
    KDevDocumentViewPlugin *m_plugin;
    KDevDocumentModel *m_documentModel;
    KDevDocumentSelection* m_selectionModel;
    QSortFilterProxyModel* m_proxy;
    KDevDocumentViewDelegate* m_delegate;
    QHash< KDevelop::IDocument*, KDevFileItem* > m_doc2index;
    QList<KUrl> m_selectedDocs; // used for ctx menu
    QList<KUrl> m_unselectedDocs; // used for ctx menu
    QStringList m_projectFolders;
};

#endif // KDEVDOCUMENTVIEW_H

