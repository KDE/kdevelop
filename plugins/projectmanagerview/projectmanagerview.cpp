/* This file is part of KDevelop
    Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2008 Aleix Pol <aleixpol@gmail.com>

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

#include "projectmanagerview.h"

#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QStandardItem>
#include <QtGui/QToolButton>
#include <QtGui/QKeyEvent>

#include <kxmlguiwindow.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kfadewidgeteffect.h>
#include <kcombobox.h>
#include <kjob.h>
#include <KLineEdit>

#include <interfaces/iselectioncontroller.h>
#include <interfaces/context.h>
#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/projectmodel.h>

#include "../openwith/iopenwith.h"

#include <KParts/MainWindow>
#include <sublime/mainwindow.h>

#include "tests/modeltest.h"
#include <project/projectproxymodel.h>
#include "projectmanagerviewplugin.h"
#include "vcsoverlayproxymodel.h"
#include "ui_projectmanagerview.h"

using namespace KDevelop;

//BEGIN ProjectManagerFilterAction

ProjectManagerFilterAction::ProjectManagerFilterAction(ProjectManagerView* parent)
    : KAction( parent )
    , m_projectManagerView(parent)
{
    setIcon(KIcon("view-filter"));
    setText(i18n("Filter..."));
    setToolTip(i18n("Insert wildcard patterns to filter the project view"
                    " for files and targets for matching items."));
}

QWidget* ProjectManagerFilterAction::createWidget( QWidget* parent )
{
    KLineEdit* edit = new KLineEdit(parent);
    edit->setClickMessage(i18n("Filter..."));
    edit->setClearButtonShown(true);
    connect(edit, SIGNAL(textChanged(QString)), this, SIGNAL(filterChanged(QString)));

    const QString filterString = m_projectManagerView->filterString();
    edit->setText(filterString);

    return edit;
}

//END ProjectManagerFilterAction

ProjectManagerViewItemContext::ProjectManagerViewItemContext(const QList< ProjectBaseItem* >& items, ProjectManagerView* view)
    : ProjectItemContext(items), m_view(view)
{
}

ProjectManagerView *ProjectManagerViewItemContext::view() const
{
    return m_view;
}


static const char* sessionConfigGroup = "ProjectManagerView";
static const char* splitterStateConfigKey = "splitterState";
static const char* filterConfigKey = "filter";
static const int projectTreeViewStrechFactor = 75; // %
static const int projectBuildSetStrechFactor = 25; // %

ProjectManagerView::ProjectManagerView( ProjectManagerViewPlugin* plugin, QWidget *parent )
        : QWidget( parent ), m_ui(new Ui::ProjectManagerView), m_plugin(plugin)
{
    m_ui->setupUi( this );

    m_ui->projectTreeView->installEventFilter(this);

    setWindowIcon( SmallIcon( "project-development" ) );

    KConfigGroup pmviewConfig(ICore::self()->activeSession()->config(), sessionConfigGroup);
    if (pmviewConfig.hasKey(splitterStateConfigKey)) {
        QByteArray geometry = pmviewConfig.readEntry<QByteArray>(splitterStateConfigKey, QByteArray());
        m_ui->splitter->restoreState(geometry);
    } else {
        m_ui->splitter->setStretchFactor(0, projectTreeViewStrechFactor);
        m_ui->splitter->setStretchFactor(1, projectBuildSetStrechFactor);
    }

    m_syncAction = plugin->actionCollection()->action("locate_document");
    Q_ASSERT(m_syncAction);
    m_syncAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_syncAction->setText(i18n("Locate Current Document"));
    m_syncAction->setToolTip(i18n("Locates the current document in the project tree and selects it."));
    m_syncAction->setIcon(KIcon("dirsync"));
    m_syncAction->setShortcut(Qt::ControlModifier + Qt::Key_Less);
    connect(m_syncAction, SIGNAL(triggered(bool)), this, SLOT(locateCurrentDocument()));
    addAction(m_syncAction);
    updateSyncAction();

    addAction(plugin->actionCollection()->action("project_build"));
    addAction(plugin->actionCollection()->action("project_install"));
    addAction(plugin->actionCollection()->action("project_clean"));

    connect(m_ui->projectTreeView, SIGNAL(activateUrl(KUrl)), this, SLOT(openUrl(KUrl)));

    m_ui->buildSetView->setProjectView( this );

    m_modelFilter = new ProjectProxyModel( this );
    m_modelFilter->setDynamicSortFilter( true );
    m_modelFilter->setSourceModel(ICore::self()->projectController()->projectModel());
    m_overlayProxy = new VcsOverlayProxyModel( this );
    m_overlayProxy->setSourceModel(m_modelFilter);

    m_ui->projectTreeView->setModel( m_overlayProxy );

    QString filterText;
    if (pmviewConfig.hasKey(filterConfigKey)) {
        filterText = pmviewConfig.readEntry(filterConfigKey, QString());
    }
    setFilterString(filterText);

    ProjectManagerFilterAction* filterAction = new ProjectManagerFilterAction(this);
    connect(filterAction, SIGNAL(filterChanged(QString)),
            this, SLOT(setFilterString(QString)));
    addAction(filterAction);

    connect( m_ui->projectTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(selectionChanged()) );
    connect( KDevelop::ICore::self()->documentController(), SIGNAL(documentClosed(KDevelop::IDocument*)),
             SLOT(updateSyncAction()));
    connect( KDevelop::ICore::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)),
             SLOT(updateSyncAction()));
    connect( qobject_cast<Sublime::MainWindow*>(KDevelop::ICore::self()->uiController()->activeMainWindow()), SIGNAL(areaChanged(Sublime::Area*)),
             SLOT(updateSyncAction()));
    selectionChanged();

    //Update the "sync" button after the initialization has completed, to see whether there already is some open documents
    QMetaObject::invokeMethod(this, "updateSyncAction", Qt::QueuedConnection);

    // Need to set this to get horizontal scrollbar. Also needs to be done after
    // the setModel call
    m_ui->projectTreeView->header()->setResizeMode( QHeaderView::ResizeToContents );
}

bool ProjectManagerView::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_ui->projectTreeView) {
        if (event->type() == QEvent::KeyRelease) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Delete) {
                m_plugin->removeItems(selectedItems());
                return true;
            } else if (keyEvent->key() == Qt::Key_F2) {
                m_plugin->renameItems(selectedItems());
                return true;
            } else if (keyEvent->key() == Qt::Key_C && keyEvent->modifiers() == Qt::ControlModifier) {
                m_plugin->copyFromContextMenu();
                return true;
            } else if (keyEvent->key() == Qt::Key_V && keyEvent->modifiers() == Qt::ControlModifier) {
                m_plugin->pasteFromContextMenu();
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void ProjectManagerView::selectionChanged()
{
    m_ui->buildSetView->selectionChanged();
    QList<ProjectBaseItem*> selected;
    foreach( const QModelIndex& idx, m_ui->projectTreeView->selectionModel()->selectedRows() )
    {
        selected << ICore::self()->projectController()->projectModel()->itemFromIndex(indexFromView( idx ));
    }
    selected.removeAll(0);
    KDevelop::ICore::self()->selectionController()->updateSelection( new ProjectManagerViewItemContext( selected, this ) );
}

void ProjectManagerView::updateSyncAction()
{
    m_syncAction->setEnabled( KDevelop::ICore::self()->documentController()->activeDocument() );
}

ProjectManagerView::~ProjectManagerView()
{
    KConfigGroup pmviewConfig(ICore::self()->activeSession()->config(), sessionConfigGroup);
    pmviewConfig.writeEntry(splitterStateConfigKey, m_ui->splitter->saveState());
    pmviewConfig.writeEntry(filterConfigKey, m_filterString);
    pmviewConfig.sync();
}

QList<KDevelop::ProjectBaseItem*> ProjectManagerView::selectedItems() const
{
    QList<KDevelop::ProjectBaseItem*> items;
    foreach( const QModelIndex &idx, m_ui->projectTreeView->selectionModel()->selectedIndexes() )
    {
        KDevelop::ProjectBaseItem* item = ICore::self()->projectController()->projectModel()->itemFromIndex(indexFromView(idx));
        if( item )
            items << item;
        else
            kDebug(9511) << "adding an unknown item";
    }
    return items;
}

void ProjectManagerView::selectItems(const QList< ProjectBaseItem* >& items)
{
    QItemSelection selection;
    foreach (ProjectBaseItem *item, items) {
        QModelIndex indx = indexToView(item->index());
        selection.append(QItemSelectionRange(indx, indx));
        m_ui->projectTreeView->setCurrentIndex(indx);
    }
    m_ui->projectTreeView->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

void ProjectManagerView::expandItem(ProjectBaseItem* item)
{
    m_ui->projectTreeView->expand( indexToView(item->index()));
}

void ProjectManagerView::locateCurrentDocument()
{
    ICore::self()->uiController()->raiseToolView(this);

    KDevelop::IDocument *doc = ICore::self()->documentController()->activeDocument();

    if (!doc) {
        // in theory we should never get a null pointer as the action is only enabled
        // when there is an active document.
        // but: in practice it can happen that you close the last document and press
        // the shortcut to locate a doc or vice versa... so just do the failsafe thing here...
        return;
    }

    QModelIndex bestMatch;
    foreach (IProject* proj, ICore::self()->projectController()->projects()) {
        foreach (KDevelop::ProjectFileItem* item, proj->filesForUrl(doc->url())) {
            QModelIndex index = indexToView(item->index());
            if (index.isValid()) {
                if (!bestMatch.isValid()) {
                    bestMatch = index;
                } else if (KDevelop::ProjectBaseItem* parent = item->parent()) {
                    // prefer files in their real folders over the 'copies' in the target folders
                    if (!parent->target()) {
                        bestMatch = index;
                        break;
                    }
                }
            }
        }
    }
    if (bestMatch.isValid()) {
        m_ui->projectTreeView->clearSelection();
        m_ui->projectTreeView->setCurrentIndex(bestMatch);
        m_ui->projectTreeView->expand(bestMatch);
        m_ui->projectTreeView->scrollTo(bestMatch);
    }
}

void ProjectManagerView::openUrl( const KUrl& url )
{
    IOpenWith::openFiles(KUrl::List() << url);
}

QString ProjectManagerView::filterString() const
{
    return m_filterString;
}

void ProjectManagerView::setFilterString(const QString &text)
{
    m_filterString = text;
    m_modelFilter->setFilterString(text);
}

QModelIndex ProjectManagerView::indexFromView(const QModelIndex& index) const
{
    return m_modelFilter->mapToSource( m_overlayProxy->mapToSource(index) );
}

QModelIndex ProjectManagerView::indexToView(const QModelIndex& index) const
{
    return m_overlayProxy->mapFromSource( m_modelFilter->mapFromSource(index) );
}

#include "projectmanagerview.moc"
