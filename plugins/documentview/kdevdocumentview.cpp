/*
    SPDX-FileCopyrightText: 2005 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdevdocumentview.h"
#include "kdevdocumentviewplugin.h"
#include "kdevdocumentmodel.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QMenu>
#include <QSortFilterProxyModel>

#include <KLocalizedString>
#include <KStandardAction>

#include "kdevdocumentselection.h"
#include "kdevdocumentviewdelegate.h"

#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/context.h>
#include <interfaces/idocument.h>

#include <widgetcolorizer.h>
#include <path.h>

using namespace KDevelop;

KDevDocumentView::KDevDocumentView( KDevDocumentViewPlugin *plugin, QWidget *parent )
    : QTreeView( parent ),
        m_plugin( plugin )
{
    connect(ICore::self()->projectController(), &IProjectController::projectOpened,
            this, &KDevDocumentView::updateProjectPaths);
    connect(ICore::self()->projectController(), &IProjectController::projectClosed,
            this, &KDevDocumentView::updateProjectPaths);

    m_documentModel = new KDevDocumentModel(this);

    m_delegate = new KDevDocumentViewDelegate( this );

    m_proxy = new QSortFilterProxyModel( this );
    m_proxy->setSourceModel( m_documentModel );
    m_proxy->setDynamicSortFilter( true );
    m_proxy->setSortCaseSensitivity( Qt::CaseInsensitive );
    m_proxy->sort(0);

    m_selectionModel = new KDevDocumentSelection( m_proxy );

    setModel( m_proxy );
    setSelectionModel( m_selectionModel );
    setItemDelegate( m_delegate );

    setObjectName( i18n( "Documents" ) );

    setWindowIcon( QIcon::fromTheme( QStringLiteral( "document-multiple" ), windowIcon() ) );
    setWindowTitle(i18nc("@title:window", "Documents"));

    setFocusPolicy( Qt::NoFocus );
    setIndentation(10);

    header()->hide();

    setSelectionBehavior( QAbstractItemView::SelectRows );
    setSelectionMode( QAbstractItemView::ExtendedSelection );

    updateProjectPaths();
}

KDevDocumentView::~KDevDocumentView()
{}

KDevDocumentViewPlugin *KDevDocumentView::plugin() const
{
    return m_plugin;
}

void KDevDocumentView::mousePressEvent( QMouseEvent * event )
{
    QModelIndex proxyIndex = indexAt( event->pos() );
    QModelIndex index = m_proxy->mapToSource( proxyIndex );

    if (event->modifiers() == Qt::NoModifier) {
        const bool actionOpen = event->button() == Qt::LeftButton;
        const bool actionClose = event->button() == Qt::MiddleButton;
        const bool action = actionOpen || actionClose;

        if (action) {
            if (proxyIndex.parent().isValid()) {
                // this is a document item
                KDevelop::IDocumentController* dc = m_plugin->core()->documentController();
                QUrl documentUrl = static_cast<KDevDocumentItem*>(m_documentModel->itemFromIndex(index))->fileItem()->url();
                KDevelop::IDocument *doc = dc->documentForUrl(documentUrl);

                // Try to open a document by url.
                if (actionOpen) {
                    if (doc != dc->activeDocument()) {
                        dc->openDocument(documentUrl);
                        return;
                    }
                }
                // Try to close a document.
                else if (actionClose) {
                    if (doc) {
                        doc->close();
                        return;
                    }
                }
            } else if (actionOpen) {
                // this is a folder item
                setExpanded(proxyIndex, !isExpanded(proxyIndex));
                return;
            }
        }
    }

    QTreeView::mousePressEvent( event );
}

template<typename F> void KDevDocumentView::visitItems(F f, bool selectedItems)
{
    KDevelop::IDocumentController* dc = m_plugin->core()->documentController();
    const QList<QUrl> docs = selectedItems ? m_selectedDocs : m_unselectedDocs;

    for (const QUrl& url : docs) {
       KDevelop::IDocument* doc = dc->documentForUrl(url);
       if (doc) f(doc);
    }
}

namespace
{
class DocSaver
{
public: void operator()(KDevelop::IDocument* doc) { doc->save(); }
};
class DocCloser
{
public: void operator()(KDevelop::IDocument* doc) { doc->close(); }
};
class DocReloader
{
public: void operator()(KDevelop::IDocument* doc) { doc->reload(); }
};
}

void KDevDocumentView::saveSelected()
{
    visitItems(DocSaver(), true);
}

void KDevDocumentView::closeSelected()
{
    visitItems(DocCloser(), true);
}

void KDevDocumentView::closeUnselected()
{
    visitItems(DocCloser(), false);
}

void KDevDocumentView::reloadSelected()
{
    visitItems(DocReloader(), true);
}

void KDevDocumentView::contextMenuEvent( QContextMenuEvent * event )
{
    QModelIndex proxyIndex = indexAt( event->pos() );
    // for now, ignore clicks on empty space or folder items
    if (!proxyIndex.isValid() || !proxyIndex.parent().isValid()) {
        return;
    }

    updateSelectedDocs();

    if (!m_selectedDocs.isEmpty())
    {
        auto* ctxMenu = new QMenu(this);

        KDevelop::FileContext context(m_selectedDocs);
        const QList<KDevelop::ContextMenuExtension> extensions =
            m_plugin->core()->pluginController()->queryPluginsForContextMenuExtensions(&context, ctxMenu);

        QList<QAction*> vcsActions;
        QList<QAction*> fileActions;
        QList<QAction*> editActions;
        QList<QAction*> extensionActions;
        for (const KDevelop::ContextMenuExtension& ext : extensions) {
            fileActions += ext.actions(KDevelop::ContextMenuExtension::FileGroup);
            vcsActions += ext.actions(KDevelop::ContextMenuExtension::VcsGroup);
            editActions += ext.actions(KDevelop::ContextMenuExtension::EditGroup);
            extensionActions += ext.actions(KDevelop::ContextMenuExtension::ExtensionGroup);
        }

        appendActions(ctxMenu, fileActions);

        QAction* save = KStandardAction::save(this, SLOT(saveSelected()), ctxMenu);
        save->setEnabled(selectedDocHasChanges());
        ctxMenu->addAction(save);
        ctxMenu->addAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18nc("@action:inmenu", "Reload"), this, SLOT(reloadSelected()));

        appendActions(ctxMenu, editActions);
        appendActions(ctxMenu, vcsActions);
        appendActions(ctxMenu, extensionActions);

        ctxMenu->addAction(QIcon::fromTheme(QStringLiteral("document-close")), i18nc("@action:inmenu", "Close"), this, SLOT(closeSelected()));
        QAction* closeUnselected = ctxMenu->addAction(QIcon::fromTheme(QStringLiteral("document-close")), i18nc("@action:inmenu", "Close All Other"), this, SLOT(closeUnselected()));
        closeUnselected->setEnabled(!m_unselectedDocs.isEmpty());

        ctxMenu->exec(event->globalPos());
        delete ctxMenu;
    }
}

void KDevDocumentView::appendActions(QMenu* menu, const QList<QAction*>& actions)
{
    for (QAction* act : actions) {
        menu->addAction(act);
    }
    menu->addSeparator();
}

bool KDevDocumentView::selectedDocHasChanges()
{
    KDevelop::IDocumentController* dc = m_plugin->core()->documentController();
    for (const QUrl& url : std::as_const(m_selectedDocs)) {
        KDevelop::IDocument* doc = dc->documentForUrl(url);
        if (!doc) continue;
        if (doc->state() != KDevelop::IDocument::Clean)
        {
            return true;
        }
    }
    return false;
}

void KDevDocumentView::updateSelectedDocs()
{
    m_selectedDocs.clear();
    m_unselectedDocs.clear();

    const QList<QStandardItem*> allItems = m_documentModel->findItems(QStringLiteral("*"), Qt::MatchWildcard | Qt::MatchRecursive);
    for (QStandardItem* item : allItems) {
        if (KDevFileItem* fileItem = static_cast<KDevDocumentItem*>(item)->fileItem()) {
            if (m_selectionModel->isSelected(m_proxy->mapFromSource(m_documentModel->indexFromItem(fileItem))))
                m_selectedDocs << fileItem->url();
            else
                m_unselectedDocs << fileItem->url();
        }
    }
}

void KDevDocumentView::activated( KDevelop::IDocument* document )
{
    setCurrentIndex( m_proxy->mapFromSource( m_documentModel->indexFromItem( m_doc2index[ document ] ) ) );
}

void KDevDocumentView::saved( KDevelop::IDocument* )
{
}

void KDevDocumentView::opened( KDevelop::IDocument* document )
{
    const QString path = QFileInfo( document->url().path() ).path();

    KDevCategoryItem *categoryItem = m_documentModel->category( path );
    if ( !categoryItem )
    {
        categoryItem = new KDevCategoryItem( path );
        categoryItem->setUrl( document->url() );
        m_documentModel->insertRow( m_documentModel->rowCount(), categoryItem );
        setExpanded( m_proxy->mapFromSource( m_documentModel->indexFromItem( categoryItem ) ), false);
        updateCategoryItem( categoryItem );
    }

    if ( !categoryItem->file( document->url() ) )
    {
        auto* fileItem = new KDevFileItem( document->url() );
        categoryItem->setChild( categoryItem->rowCount(), fileItem );
        setCurrentIndex( m_proxy->mapFromSource( m_documentModel->indexFromItem( fileItem ) ) );
        m_doc2index[ document ] = fileItem;
    }
}

void KDevDocumentView::closed( KDevelop::IDocument* document )
{
    KDevFileItem* file = m_doc2index[ document ];
    if ( !file )
        return;

    QStandardItem* categoryItem = file->parent();

    qDeleteAll(categoryItem->takeRow(m_documentModel->indexFromItem(file).row()));

    m_doc2index.remove(document);

    if ( categoryItem->hasChildren() )
        return;

    qDeleteAll(m_documentModel->takeRow(m_documentModel->indexFromItem(categoryItem).row()));

    doItemsLayout();
}

void KDevDocumentView::updateCategoryItem( KDevCategoryItem *item )
{
    QString text = KDevelop::ICore::self()->projectController()->prettyFilePath(item->url(), KDevelop::IProjectController::FormatPlain);
    // remove trailing slash
    if (text.length() > 1) {
        text.chop(1);
    }
    item->setText(text);
}

void KDevDocumentView::updateProjectPaths()
{
    const auto categoryList = m_documentModel->categoryList();
    for (KDevCategoryItem* it : categoryList) {
        updateCategoryItem( it );
    }
}

void KDevDocumentView::contentChanged( KDevelop::IDocument* )
{
}

void KDevDocumentView::stateChanged( KDevelop::IDocument* document )
{
    KDevDocumentItem * documentItem = m_doc2index[ document ];

    if ( documentItem && documentItem->documentState() != document->state() )
        documentItem->setDocumentState( document->state() );

    doItemsLayout();
}

void KDevDocumentView::documentUrlChanged( KDevelop::IDocument* document )
{
    closed(document);
    opened(document);
}

void KDevDocumentView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    if (WidgetColorizer::colorizeByProject()) {
        const auto url = index.data(KDevDocumentItem::UrlRole).toUrl();
        const auto project = ICore::self()->projectController()->findProjectForUrl(url);
        if (project) {
            const QColor color = WidgetColorizer::colorForId(qHash(project->path()), palette(), true);
            WidgetColorizer::drawBranches(this, painter, rect, index, color);
        }
    }

    QTreeView::drawBranches(painter, rect, index);
}

#include "moc_kdevdocumentview.cpp"
