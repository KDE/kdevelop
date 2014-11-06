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

#include <QHeaderView>
#include <QKeyEvent>
#include <QUrl>

#include <kiconloader.h>
#include <KLocalizedString>
#include <kactioncollection.h>
#include <QLineEdit>

#include <interfaces/iselectioncontroller.h>
#include <interfaces/context.h>
#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectproxymodel.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include "../openwith/iopenwith.h"

#include <sublime/mainwindow.h>
#include <sublime/area.h>

#include "projectmanagerviewplugin.h"
#include "vcsoverlayproxymodel.h"
#include "ui_projectmanagerview.h"
#include "debug.h"


using namespace KDevelop;

ProjectManagerViewItemContext::ProjectManagerViewItemContext(const QList< ProjectBaseItem* >& items, ProjectManagerView* view)
    : ProjectItemContext(items), m_view(view)
{
}

ProjectManagerView *ProjectManagerViewItemContext::view() const
{
    return m_view;
}


static const char sessionConfigGroup[] = "ProjectManagerView";
static const char splitterStateConfigKey[] = "splitterState";
static const int projectTreeViewStrechFactor = 75; // %
static const int projectBuildSetStrechFactor = 25; // %

ProjectManagerView::ProjectManagerView( ProjectManagerViewPlugin* plugin, QWidget *parent )
        : QWidget( parent ), m_ui(new Ui::ProjectManagerView), m_plugin(plugin)
{
    m_ui->setupUi( this );

    m_ui->projectTreeView->installEventFilter(this);

    setWindowIcon( QIcon::fromTheme( "project-development" ) );

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
    m_syncAction->setIcon(QIcon::fromTheme("dirsync"));
    m_syncAction->setShortcut(Qt::ControlModifier + Qt::Key_Less);
    connect(m_syncAction, &QAction::triggered, this, &ProjectManagerView::locateCurrentDocument);
    addAction(m_syncAction);
    updateSyncAction();

    addAction(plugin->actionCollection()->action("project_build"));
    addAction(plugin->actionCollection()->action("project_install"));
    addAction(plugin->actionCollection()->action("project_clean"));

    connect(m_ui->projectTreeView, &ProjectTreeView::activate, this, &ProjectManagerView::open);

    m_ui->buildSetView->setProjectView( this );

    m_modelFilter = new ProjectProxyModel( this );
    m_modelFilter->setSourceModel(ICore::self()->projectController()->projectModel());
    m_overlayProxy = new VcsOverlayProxyModel( this );
    m_overlayProxy->setSourceModel(m_modelFilter);

    m_ui->projectTreeView->setModel( m_overlayProxy );

    connect( m_ui->projectTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
             this, &ProjectManagerView::selectionChanged );
    connect( KDevelop::ICore::self()->documentController(), &IDocumentController::documentClosed,
             this, &ProjectManagerView::updateSyncAction);
    connect( KDevelop::ICore::self()->documentController(), &IDocumentController::documentActivated,
             this, &ProjectManagerView::updateSyncAction);
    connect( qobject_cast<Sublime::MainWindow*>(KDevelop::ICore::self()->uiController()->activeMainWindow()), &Sublime::MainWindow::areaChanged,
             this, &ProjectManagerView::updateSyncAction);
    selectionChanged();

    //Update the "sync" button after the initialization has completed, to see whether there already is some open documents
    QMetaObject::invokeMethod(this, "updateSyncAction", Qt::QueuedConnection);

    // Need to set this to get horizontal scrollbar. Also needs to be done after
    // the setModel call
    m_ui->projectTreeView->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
}

bool ProjectManagerView::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_ui->projectTreeView) {
        if (event->type() == QEvent::KeyRelease) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Delete && keyEvent->modifiers() == Qt::NoModifier) {
                m_plugin->removeItems(selectedItems());
                return true;
            } else if (keyEvent->key() == Qt::Key_F2 && keyEvent->modifiers() == Qt::NoModifier) {
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
    pmviewConfig.sync();

    delete m_ui;
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
            qCDebug(PLUGIN_PROJECTMANAGERVIEW) << "adding an unknown item";
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

void ProjectManagerView::open( const Path& path )
{
    IOpenWith::openFiles(QList<QUrl>() << path.toUrl());
}

QModelIndex ProjectManagerView::indexFromView(const QModelIndex& index) const
{
    return m_modelFilter->mapToSource( m_overlayProxy->mapToSource(index) );
}

QModelIndex ProjectManagerView::indexToView(const QModelIndex& index) const
{
    return m_overlayProxy->mapFromSource( m_modelFilter->mapFromSource(index) );
}

