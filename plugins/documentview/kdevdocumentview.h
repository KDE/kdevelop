/*
    SPDX-FileCopyrightText: 2005 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVDOCUMENTVIEW_H
#define KDEVPLATFORM_PLUGIN_KDEVDOCUMENTVIEW_H

#include <QTreeView>
#include <QUrl>

class QSortFilterProxyModel;
class QAction;
class KDevDocumentViewPlugin;
class KDevDocumentModel;
class KDevDocumentViewDelegate;
class KDevDocumentSelection;
class KDevFileItem;
class KDevCategoryItem;
namespace KDevelop
{
    class IDocument;
    class IProject;
}


class KDevDocumentView: public QTreeView
{
    Q_OBJECT
public:
    explicit KDevDocumentView( KDevDocumentViewPlugin *plugin, QWidget *parent );
    ~KDevDocumentView() override;

    KDevDocumentViewPlugin *plugin() const;

Q_SIGNALS:
    void activateURL( const QUrl &url );

public Q_SLOTS:
    void opened( KDevelop::IDocument* document );

private Q_SLOTS:
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
    void mousePressEvent( QMouseEvent * event ) override;
    void contextMenuEvent( QContextMenuEvent * event ) override;
    void drawBranches(QPainter* painter, const QRect& rect,
                      const QModelIndex& index) const override;

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
    QList<QUrl> m_selectedDocs; // used for ctx menu
    QList<QUrl> m_unselectedDocs; // used for ctx menu

    friend class KDevDocumentViewPluginFactory; // to connect to the private slots stateChanged and documentUrlChanged
};

#endif // KDEVPLATFORM_PLUGIN_KDEVDOCUMENTVIEW_H

