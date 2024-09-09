/*
    SPDX-FileCopyrightText: 2009, 2013 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2013 Jarosław Sierant <jaroslaw.sierant@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "documentswitcherplugin.h"

#include <QApplication>
#include <QListView>
#include <QStandardItemModel>
#include <QScrollBar>

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <sublime/mainwindow.h>

#include "documentswitchertreeview.h"
#include "documentswitcheritem.h"
#include "debug.h"

#include <algorithm>

K_PLUGIN_FACTORY_WITH_JSON(DocumentSwitcherFactory, "kdevdocumentswitcher.json", registerPlugin<DocumentSwitcherPlugin>();)

//TODO: Show frame around view's widget while walking through
//TODO: Make the widget transparent

DocumentSwitcherPlugin::DocumentSwitcherPlugin(QObject* parent, const KPluginMetaData& metaData,
                                               const QVariantList& /*args*/)
    : KDevelop::IPlugin(QStringLiteral("kdevdocumentswitcher"), parent, metaData)
    , view(nullptr)
{
    setXMLFile(QStringLiteral("kdevdocumentswitcher.rc"));
    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "Adding active mainwindow from constructor";

    KDevelop::IDocumentController *documentController = KDevelop::ICore::self()->documentController();
    for (KDevelop::IDocument *doc : documentController->openDocuments()) {
        documentOpened(doc);
    }

    // Signals to track last used documents.
    connect(documentController, &KDevelop::IDocumentController::documentOpened, this, &DocumentSwitcherPlugin::documentOpened);
    connect(documentController, &KDevelop::IDocumentController::documentActivated, this, &DocumentSwitcherPlugin::documentActivated);
    connect(documentController, &KDevelop::IDocumentController::documentClosed, this, &DocumentSwitcherPlugin::documentClosed);
    connect(documentController, &KDevelop::IDocumentController::documentUrlChanged, this,
            &DocumentSwitcherPlugin::documentUrlChanged);

#ifdef Q_OS_MACOS
    // Qt/Mac swaps the Ctrl and Meta (Command) keys by default, so that shortcuts defined as Ctrl+X
    // become the platform-standard Command+X . Ideally we would map the document switcher shortcut to
    // Control+Tab (and thus Qt::META|Qt::Key_Tab) everywhere because Command+Tab and Command+Shift+Tab
    // are reserved system shortcuts that bring up the application switcher. The Control+Tab shortcut is 
    // inoperable on Mac, so we resort to the Alt (Option) key, unless the AA_MacDontSwapCtrlAndMeta
    // attribute is set.
    const Qt::Modifier shortcutAccelerator = QCoreApplication::testAttribute(Qt::AA_MacDontSwapCtrlAndMeta) ? Qt::CTRL : Qt::ALT;
#else
    const Qt::Modifier shortcutAccelerator = Qt::CTRL;
#endif

    forwardAction = actionCollection()->addAction ( QStringLiteral( "last_used_views_forward" ) );
    forwardAction->setText(i18nc("@action", "Last Used Views"));
    forwardAction->setIcon( QIcon::fromTheme( QStringLiteral( "go-next-view-page") ) );
    actionCollection()->setDefaultShortcut( forwardAction, shortcutAccelerator | Qt::Key_Tab );
    forwardAction->setWhatsThis(i18nc("@info:whatsthis", "Opens a list to walk through the list of last used views."));
    forwardAction->setToolTip( i18nc("@info:tooltip", "Walk through the list of last used views"));
    connect( forwardAction, &QAction::triggered, this, &DocumentSwitcherPlugin::walkForward );

    backwardAction = actionCollection()->addAction ( QStringLiteral( "last_used_views_backward" ) );
    backwardAction->setText(i18nc("@action", "Last Used Views (Reverse)"));
    backwardAction->setIcon( QIcon::fromTheme( QStringLiteral( "go-previous-view-page") ) );
    actionCollection()->setDefaultShortcut( backwardAction, shortcutAccelerator | Qt::SHIFT | Qt::Key_Tab );
    backwardAction->setWhatsThis(i18nc("@info:whatsthis", "Opens a list to walk through the list of last used views in reverse."));
    backwardAction->setToolTip(i18nc("@info:tooltip", "Walk through the list of last used views"));
    connect( backwardAction, &QAction::triggered, this, &DocumentSwitcherPlugin::walkBackward );

    view = new DocumentSwitcherTreeView( this );
    view->setSelectionBehavior( QAbstractItemView::SelectRows );
    view->setSelectionMode( QAbstractItemView::SingleSelection );
    view->setUniformRowHeights( true );
    view->setTextElideMode( Qt::ElideMiddle );
    view->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    view->addAction( forwardAction );
    view->addAction( backwardAction );
    view->setHeaderHidden( true );
    view->setIndentation( 10 );
    connect( view, &QListView::pressed, this, &DocumentSwitcherPlugin::switchToClicked );
    connect( view, &QListView::activated, this, &DocumentSwitcherPlugin::itemActivated );

    model = new QStandardItemModel( view );
    view->setModel( model );
}

void DocumentSwitcherPlugin::setViewGeometry(Sublime::MainWindow* window)
{
    const QSize centralSize = window->centralWidget()->size();

    // Maximum size of the view is 3/4th of the central widget (the editor area) so the view does not overlap the
    // mainwindow since that looks awkward.
    const QSize viewMaxSize( centralSize.width() * 3/4, centralSize.height() * 3/4 );

    // The actual view size should be as big as the columns/rows need it, but smaller than the max-size. This means
    // the view will get quite high with many open files but I think that is ok. Otherwise one can easily tweak the
    // max size to be only 1/2th of the central widget size
    const int rowHeight = view->sizeHintForRow(0);
    const int frameWidth = view->frameWidth();
    const QSize viewSize( std::min( view->sizeHintForColumn(0) + 2 * frameWidth + view->verticalScrollBar()->width(), viewMaxSize.width() ),
                          std::min( std::max( rowHeight * view->model()->rowCount() + 2 * frameWidth, rowHeight * 6 ), viewMaxSize.height() ) );

    // Position should be central over the editor area, so map to global from parent of central widget since
    // the view is positioned in global coords
    QPoint centralWidgetPos = window->mapToGlobal( window->centralWidget()->pos() );
    const int xPos = std::max(0, centralWidgetPos.x() + (centralSize.width()  - viewSize.width()  ) / 2);
    const int yPos = std::max(0, centralWidgetPos.y() + (centralSize.height() - viewSize.height() ) / 2);

    view->setFixedSize(viewSize);
    view->move(xPos, yPos);
}

void DocumentSwitcherPlugin::walk(const int from, const int to)
{
    auto* window = qobject_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() );

    QModelIndex idx;
    const int step = from < to ? 1 : -1;
    if(!view->isVisible())
    {
        fillModel();
        setViewGeometry(window);
        idx = model->index(from + step, 0);
        if(!idx.isValid()) { idx = model->index(0, 0); }
        view->show();
    } else {
        int newRow = view->selectionModel()->currentIndex().row() + step;
        if(newRow == to + step) { newRow = from; }
        idx = model->index(newRow, 0);
    }
    view->selectionModel()->select(idx, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}


void DocumentSwitcherPlugin::walkForward() { walk(0, model->rowCount()-1); }

void DocumentSwitcherPlugin::walkBackward() { walk(model->rowCount()-1, 0); }

void DocumentSwitcherPlugin::fillModel()
{
    model->clear();

    for (auto *doc : documentLists) {
        DocumentSwitcherItem* item = new DocumentSwitcherItem(doc, cachedMimeTypes);
        model->appendRow(item);
    }
}

DocumentSwitcherPlugin::~DocumentSwitcherPlugin()
{
}

void DocumentSwitcherPlugin::switchToClicked( const QModelIndex& idx )
{
    view->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
    itemActivated(idx);
}

void DocumentSwitcherPlugin::itemActivated( const QModelIndex& idx )
{
    Q_UNUSED( idx );
    if( view->selectionModel()->selectedRows().isEmpty() )
    {
        return;
    }
    const int row = view->selectionModel()->selectedRows().first().row();

    // Retrieve document from index
    auto* activatedDocument = documentLists.value(row);
    if( activatedDocument ) {
        // Close document
        if( QApplication::mouseButtons() & Qt::MiddleButton )
        {
            activatedDocument->close();
            fillModel();
            if ( model->rowCount() == 0 ) {
                view->hide();
            } else {
                view->selectionModel()->select( view->model()->index(0, 0), QItemSelectionModel::ClearAndSelect );
            }
        }
        // Activate document
        else
        {
            KDevelop::IDocumentController *documentController = KDevelop::ICore::self()->documentController();
            documentController->activateDocument(activatedDocument);
            view->hide();
        }
    }
}

void DocumentSwitcherPlugin::documentUrlChanged(KDevelop::IDocument* document, QUrl const&)
{
    cachedMimeTypes.erase(document);
}

void DocumentSwitcherPlugin::documentOpened(KDevelop::IDocument *document)
{
    if (!documentLists.contains(document)) {
        documentLists.prepend(document);
    }
}

void DocumentSwitcherPlugin::documentActivated(KDevelop::IDocument *document)
{
    documentLists.removeOne(document);
    documentLists.prepend(document);
}

void DocumentSwitcherPlugin::documentClosed(KDevelop::IDocument *document)
{
    documentLists.removeOne(document);
    cachedMimeTypes.erase(document);
}

void DocumentSwitcherPlugin::unload()
{
    delete forwardAction;
    delete backwardAction;
    view->deleteLater();
}

bool DocumentSwitcherPlugin::eventFilter( QObject* watched, QEvent* ev )
{
    auto* mw = qobject_cast<Sublime::MainWindow*>(watched);
    if( mw && ev->type() == QEvent::WindowActivate )
    {
        enableActions();
    }
    return QObject::eventFilter( watched, ev );
}

void DocumentSwitcherPlugin::enableActions()
{
    forwardAction->setEnabled(true);
    backwardAction->setEnabled(true);
}

#include "documentswitcherplugin.moc"
#include "moc_documentswitcherplugin.cpp"
