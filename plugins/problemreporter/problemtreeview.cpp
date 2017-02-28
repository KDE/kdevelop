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

#include <QAction>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QIcon>
#include <QItemDelegate>
#include <QMenu>
#include <QSortFilterProxyModel>

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iassistant.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <util/kdevstringhandler.h>

#include "problemreporterplugin.h"
#include <shell/problemmodel.h>
#include <shell/problem.h>
#include <shell/problemconstants.h>

//#include "modeltest.h"

using namespace KDevelop;

namespace KDevelop
{

class ProblemTreeViewItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    explicit ProblemTreeViewItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};
}

ProblemTreeViewItemDelegate::ProblemTreeViewItemDelegate(QObject* parent)
    : QItemDelegate(parent)
{
}

void ProblemTreeViewItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const
{
    QStyleOptionViewItem newOption(option);
    newOption.textElideMode = index.column() == ProblemModel::File ? Qt::ElideMiddle : Qt::ElideRight;

    QItemDelegate::paint(painter, newOption, index);
}

ProblemTreeView::ProblemTreeView(QWidget* parent, QAbstractItemModel* itemModel)
    : QTreeView(parent)
    , m_proxy(new QSortFilterProxyModel(this))
{
    setObjectName(QStringLiteral("Problem Reporter Tree"));
    setWhatsThis(i18n("Problems"));
    setItemDelegate(new ProblemTreeViewItemDelegate(this));
    setSelectionBehavior(QAbstractItemView::SelectRows);

    m_proxy->setSortRole(ProblemModel::SeverityRole);
    m_proxy->setDynamicSortFilter(true);
    m_proxy->sort(0, Qt::AscendingOrder);

    ProblemModel* problemModel = dynamic_cast<ProblemModel*>(itemModel);
    Q_ASSERT(problemModel);
    setModel(problemModel);

    header()->setStretchLastSection(false);
    if (!problemModel->features().testFlag(ProblemModel::ShowSource)) {
        hideColumn(ProblemModel::Source);
    }

    connect(this, &ProblemTreeView::clicked, this, &ProblemTreeView::itemActivated);

    connect(model(), &QAbstractItemModel::rowsInserted, this, &ProblemTreeView::changed);
    connect(model(), &QAbstractItemModel::rowsRemoved, this, &ProblemTreeView::changed);
    connect(model(), &QAbstractItemModel::modelReset, this, &ProblemTreeView::changed);

    m_proxy->setFilterKeyColumn(-1);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

ProblemTreeView::~ProblemTreeView()
{
}

void ProblemTreeView::openDocumentForCurrentProblem()
{
    itemActivated(currentIndex());
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
        const auto problem = index.data(ProblemModel::ProblemRole).value<IProblem::Ptr>();
        if (!problem)
            return;

        url = problem->finalLocation().document.toUrl();
        start = problem->finalLocation().start();
    }

    if (QFile::exists(url.toLocalFile())) {
        ICore::self()->documentController()->openDocument(url, start);
    }
}

void ProblemTreeView::resizeColumns()
{
    for (int i = 0; i < model()->columnCount(); ++i)
        resizeColumnToContents(i);
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

int ProblemTreeView::setFilter(const QString& filterText)
{
    m_proxy->setFilterFixedString(filterText);

    return m_proxy->rowCount();
}

ProblemModel* ProblemTreeView::model() const
{
    return static_cast<ProblemModel*>(m_proxy->sourceModel());
}

void ProblemTreeView::setModel(QAbstractItemModel* model)
{
    Q_ASSERT(qobject_cast<ProblemModel*>(model));
    m_proxy->setSourceModel(model);
    QTreeView::setModel(m_proxy);
}

void ProblemTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
        return;

    const auto problem = index.data(ProblemModel::ProblemRole).value<IProblem::Ptr>();
    if (!problem) {
        return;
    }

    QExplicitlySharedDataPointer<KDevelop::IAssistant> solution = problem->solutionAssistant();
    if (!solution) {
        return;
    }

    QList<QAction*> actions;
    foreach (KDevelop::IAssistantAction::Ptr action, solution->actions()) {
        actions << action->toKAction();
    }

    if (actions.isEmpty()) {
        return;
    }

    QString title = solution->title();
    title = KDevelop::htmlToPlainText(title);
    title.replace(QLatin1String("&apos;"), QLatin1String("\'"));

    QPointer<QMenu> m = new QMenu(this);
    m->addSection(title);
    m->addActions(actions);
    m->exec(event->globalPos());
    delete m;
}

void ProblemTreeView::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    resizeColumns();
}

#include "problemtreeview.moc"
