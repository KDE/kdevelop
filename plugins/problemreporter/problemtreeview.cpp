/*
 * KDevelop Problem Reporter
 *
 * Copyright (c) 2006-2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2006 Adam Treat <treat@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "problemtreeview.h"

#include <QMenu>
#include <QCursor>
#include <QContextMenuEvent>
#include <QSignalMapper>
#include <QHeaderView>

#include <QAction>
#include <kactionmenu.h>
#include <KLocalizedString>
#include <kicon.h>
#include <KMenu>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iassistant.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <util/kdevstringhandler.h>

#include "problemreporterplugin.h"
#include "problemmodel.h"

//#include "modeltest.h"

using namespace KDevelop;

ProblemTreeView::ProblemTreeView(QWidget* parent, ProblemReporterPlugin* plugin)
    : QTreeView(parent)
    , m_plugin(plugin)
{
    setObjectName("Problem Reporter Tree");
    setWindowTitle(i18n("Problems"));
    setWindowIcon( QIcon::fromTheme("dialog-information") ); ///@todo Use a proper icon
    setRootIsDecorated(false);
    setWhatsThis( i18n( "Problems" ) );

    setModel(m_plugin->getModel());
    header()->setStretchLastSection(false);

    QAction* fullUpdateAction = new QAction(this);
    fullUpdateAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    fullUpdateAction->setText(i18n("Force Full Update"));
    fullUpdateAction->setToolTip(i18nc("@info:tooltip", "Re-parse all watched documents"));
    fullUpdateAction->setIcon(QIcon::fromTheme("view-refresh"));
    connect(fullUpdateAction, SIGNAL(triggered(bool)), model(), SLOT(forceFullUpdate()));
    addAction(fullUpdateAction);

    QAction* showImportsAction = new QAction(this);
    addAction(showImportsAction);
    showImportsAction->setCheckable(true);
    showImportsAction->setChecked(false);
    showImportsAction->setText(i18n("Show Imports"));
    showImportsAction->setToolTip(i18nc("@info:tooltip", "Display problems in imported files"));
    this->model()->setShowImports(false);
    connect(showImportsAction, SIGNAL(triggered(bool)), this->model(), SLOT(setShowImports(bool)));

    KActionMenu* scopeMenu = new KActionMenu(this);
    scopeMenu->setDelayed(false);
    scopeMenu->setText(i18n("Scope"));
    scopeMenu->setToolTip(i18nc("@info:tooltip", "Which files to display the problems for"));

    QActionGroup* scopeActions = new QActionGroup(this);

    QAction* currentDocumentAction = new QAction(this);
    currentDocumentAction->setText(i18n("Current Document"));
    currentDocumentAction->setToolTip(i18nc("@info:tooltip", "Display problems in current document"));

    QAction* openDocumentsAction = new QAction(this);
    openDocumentsAction->setText(i18n("Open Documents"));
    openDocumentsAction->setToolTip(i18nc("@info:tooltip", "Display problems in all open documents"));

    QAction* currentProjectAction = new QAction(this);
    currentProjectAction->setText(i18n("Current Project"));
    currentProjectAction->setToolTip(i18nc("@info:tooltip", "Display problems in current project"));

    QAction* allProjectAction = new QAction(this);
    allProjectAction->setText(i18n("All Projects"));
    allProjectAction->setToolTip(i18nc("@info:tooltip", "Display problems in all projects"));

    QAction* scopeActionArray[] = {currentDocumentAction, openDocumentsAction, currentProjectAction, allProjectAction};
    for (int i = 0; i < 4; ++i) {
        scopeActionArray[i]->setCheckable(true);
        scopeActions->addAction(scopeActionArray[i]);
        scopeMenu->addAction(scopeActionArray[i]);
    }
    addAction(scopeMenu);

    currentDocumentAction->setChecked(true);
    model()->setScope(ProblemModel::CurrentDocument);
    QSignalMapper * scopeMapper = new QSignalMapper(this);
    scopeMapper->setMapping(currentDocumentAction, ProblemModel::CurrentDocument);
    scopeMapper->setMapping(openDocumentsAction, ProblemModel::OpenDocuments);
    scopeMapper->setMapping(currentProjectAction, ProblemModel::CurrentProject);
    scopeMapper->setMapping(allProjectAction, ProblemModel::AllProjects);
    connect(currentDocumentAction, SIGNAL(triggered()), scopeMapper, SLOT(map()));
    connect(openDocumentsAction, SIGNAL(triggered()), scopeMapper, SLOT(map()));
    connect(currentProjectAction, SIGNAL(triggered()), scopeMapper, SLOT(map()));
    connect(allProjectAction, SIGNAL(triggered()), scopeMapper, SLOT(map()));
    connect(scopeMapper, SIGNAL(mapped(int)), model(), SLOT(setScope(int)));

    KActionMenu* severityMenu = new KActionMenu(i18n("Severity"), this);
    severityMenu->setDelayed(false);
    severityMenu->setToolTip(i18nc("@info:tooltip", "Select the lowest level of problem severity to be displayed"));
    QActionGroup* severityActions = new QActionGroup(this);

    QAction* errorSeverityAction = new QAction(i18n("Error"), this);
    errorSeverityAction->setToolTip(i18nc("@info:tooltip", "Display only errors"));

    QAction* warningSeverityAction = new QAction(i18n("Warning"), this);
    warningSeverityAction->setToolTip(i18nc("@info:tooltip", "Display errors and warnings"));

    QAction* hintSeverityAction = new QAction(i18n("Hint"), this);
    hintSeverityAction->setToolTip(i18nc("@info:tooltip", "Display errors, warnings and hints"));

    QAction* severityActionArray[] = {errorSeverityAction, warningSeverityAction, hintSeverityAction};
    for (int i = 0; i < 3; ++i) {
        severityActionArray[i]->setCheckable(true);
        severityActions->addAction(severityActionArray[i]);
        severityMenu->addAction(severityActionArray[i]);
    }
    addAction(severityMenu);

    hintSeverityAction->setChecked(true);
    model()->setSeverity(ProblemData::Hint);
    QSignalMapper * severityMapper = new QSignalMapper(this);
    severityMapper->setMapping(errorSeverityAction, ProblemData::Error);
    severityMapper->setMapping(warningSeverityAction, ProblemData::Warning);
    severityMapper->setMapping(hintSeverityAction, ProblemData::Hint);
    connect(errorSeverityAction, SIGNAL(triggered()), severityMapper, SLOT(map()));
    connect(warningSeverityAction, SIGNAL(triggered()), severityMapper, SLOT(map()));
    connect(hintSeverityAction, SIGNAL(triggered()), severityMapper, SLOT(map()));
    connect(severityMapper, SIGNAL(mapped(int)), model(), SLOT(setSeverity(int)));

    connect(this, SIGNAL(activated(QModelIndex)), SLOT(itemActivated(QModelIndex)));
}

ProblemTreeView::~ProblemTreeView()
{
}

void ProblemTreeView::itemActivated(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    KTextEditor::Cursor start;
    QUrl url;

    {
        // TODO: is this really necessary?
        DUChainReadLocker lock(DUChain::lock());
        ProblemPointer problem = model()->problemForIndex(index);
        url = problem->finalLocation().document.toUrl();
        start = problem->finalLocation().start();
    }

    m_plugin->core()->documentController()->openDocument(url, start);
}

void ProblemTreeView::resizeColumns()
{
    // Do actual resizing only if the widget is visible and there are not too many items
    const int ResizeRowLimit = 15;
    if (isVisible() && model()->rowCount() > 0 && model()->rowCount() < ResizeRowLimit) {
        const int columnCount = model()->columnCount();
        QVector<int> widthArray(columnCount);
        int totalWidth = 0;
        for (int i = 0; i < columnCount; ++i) {
            widthArray[i] = columnWidth(i);
            totalWidth += widthArray[i];
        }
        for (int i = 0; i < columnCount; ++i) {
            int columnWidthHint = qMax(sizeHintForColumn(i), header()->sectionSizeHint(i));
            if (columnWidthHint - widthArray[i] > 0) {
                if (columnWidthHint - widthArray[i] < width() - totalWidth) { // enough space to resize
                    setColumnWidth(i, columnWidthHint);
                    totalWidth += (columnWidthHint - widthArray[i]);
                } else {
                    setColumnWidth(i, widthArray[i] + width() - totalWidth);
                    break;
                }
            }
        }
    }
}

void ProblemTreeView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    QTreeView::dataChanged(topLeft, bottomRight, roles);
    resizeColumns();
}

void ProblemTreeView::reset()
{
    QTreeView::reset();
    resizeColumns();
}

ProblemModel * ProblemTreeView::model() const
{
    return static_cast<ProblemModel*>(QTreeView::model());
}

void ProblemTreeView::setModel(QAbstractItemModel* model)
{
    Q_ASSERT(qobject_cast<ProblemModel*>(model));
    QTreeView::setModel(model);
}

void ProblemTreeView::contextMenuEvent(QContextMenuEvent* event) {
    QModelIndex index = indexAt(event->pos());
    if(index.isValid()) {
        KDevelop::ProblemPointer problem = model()->problemForIndex(index);
        if(problem) {
            QExplicitlySharedDataPointer<KDevelop::IAssistant> solution = problem->solutionAssistant();
            if(solution) {
                QList<QAction*> actions;
                foreach(KDevelop::IAssistantAction::Ptr action, solution->actions()) {
                    actions << action->toKAction();
                }
                if(!actions.isEmpty()) {
                    QString title = solution->title();
                    title = KDevelop::htmlToPlainText(title);
                    title.replace("&apos;", "\'");

                    QPointer<KMenu> m = new KMenu(this);
                    m->addTitle(title);
                    m->addActions(actions);
                    m->exec(event->globalPos());
                    delete m;
                }
            }
        }
    }
}

void ProblemTreeView::showEvent(QShowEvent * event)
{
    Q_UNUSED(event)

    for (int i = 0; i < model()->columnCount(); ++i)
        resizeColumnToContents(i);
}

