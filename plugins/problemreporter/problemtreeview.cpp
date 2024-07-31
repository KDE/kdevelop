/*
    SPDX-FileCopyrightText: 2006-2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "problemtreeview.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QItemDelegate>
#include <QMenu>
#include <QSortFilterProxyModel>

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iassistant.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/editor/documentrange.h>
#include <util/kdevstringhandler.h>

#include <shell/problemmodel.h>
#include <shell/problem.h>
#include <shell/problemconstants.h>

#include <algorithm>
#include <array>

using namespace KDevelop;

namespace {
QString descriptionFromProblem(IProblem::Ptr problem)
{
    QString text;
    const auto location = problem->finalLocation();
    if (location.isValid()) {
        text += location.document.toUrl()
            .adjusted(QUrl::NormalizePathSegments)
            .toDisplayString(QUrl::PreferLocalFile);
        if (location.start().line() >= 0) {
            text += QLatin1Char(':') + QString::number(location.start().line() + 1);
            if (location.start().column() >= 0) {
                text += QLatin1Char(':') + QString::number(location.start().column() + 1);
            }
        }
        text += QLatin1String(": ");
    }
    text += problem->description();
    if (!problem->explanation().isEmpty()) {
        text += QLatin1Char('\n') + problem->explanation();
    }
    return text;
}
}

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
    setWhatsThis(i18nc("@info:whatsthis", "Problems"));
    setItemDelegate(new ProblemTreeViewItemDelegate(this));
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setUniformRowHeights(true);

    m_proxy->setSortRole(ProblemModel::SeverityRole);
    m_proxy->setDynamicSortFilter(true);
    m_proxy->sort(0, Qt::AscendingOrder);

    auto* problemModel = qobject_cast<ProblemModel*>(itemModel);
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

    resizeColumns();
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
    // Don't simply call QTreeView::resizeColumnToContents() for each column here,
    // because it is not useful enough to justify significant performance cost.
    // Instead, set column widths to heuristic values independent on the contents (the problem list).

    const int averageCharWidth = fontMetrics().averageCharWidth();
    const int headerWidth = header()->width();
    if (averageCharWidth == m_averageCharWidth && headerWidth == m_headerWidth) {
        // No reason to change column widths. This early return is not just an optimization: KDevelop should not
        // gratuitously reapply unchanged heuristic column widths, because the user may have fine-tuned them manually.
        return;
    }
    m_averageCharWidth = averageCharWidth;
    m_headerWidth = headerWidth;

    struct ColumnSizePolicy
    {
        int minWidthInCharacters;
        int stretchFactor;
    };
    static constexpr std::array<ColumnSizePolicy, 5> sizePolicy{
        ColumnSizePolicy{40, 20}, // Error
        ColumnSizePolicy{25, 1}, //  Source
        ColumnSizePolicy{30, 10}, // File
        ColumnSizePolicy{10, 1}, //  Line
        ColumnSizePolicy{10, 1}, //  Column
    };
    static_assert(sizePolicy.size() == ProblemModel::LastColumn);

    // Cannot use std::accumulate() here, because it is not constexpr in C++17.
    static constexpr ColumnSizePolicy totalAllColumns = [] {
        ColumnSizePolicy sum{};
        for (auto p : sizePolicy) {
            sum.minWidthInCharacters += p.minWidthInCharacters;
            sum.stretchFactor += p.stretchFactor;
        }
        return sum;
    }();

    ColumnSizePolicy total = totalAllColumns;
    if (!model()->features().testFlag(ProblemModel::ShowSource)) {
        // Disregard the size policy of the hidden Source column.
        static constexpr auto hiddenColumn = sizePolicy[ProblemModel::Source];
        total.minWidthInCharacters -= hiddenColumn.minWidthInCharacters;
        total.stretchFactor -= hiddenColumn.stretchFactor;
    }
    Q_ASSERT(total.stretchFactor > 0);

    const int remainingPixels = std::max(0, headerWidth - total.minWidthInCharacters * averageCharWidth);

    // Give each column its minimum needed width. If there is any horizontal space left,
    // distribute it among columns in proportion to their stretch factors.
    for (std::size_t i = 0; i < sizePolicy.size(); ++i) {
        int width = sizePolicy[i].minWidthInCharacters * averageCharWidth;
        width += remainingPixels * sizePolicy[i].stretchFactor / total.stretchFactor;
        setColumnWidth(i, width);
    }
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

    QPointer<QMenu> m = new QMenu(this);

    m->addSection(i18nc("@title:menu", "Problem"));
    auto copyDescriptionAction = m->addAction(QIcon::fromTheme(QStringLiteral("edit-copy")),
                                              i18nc("@action:inmenu", "&Copy Description"));
    connect(copyDescriptionAction, &QAction::triggered, this, [problem]() {
        QApplication::clipboard()->setText(descriptionFromProblem(problem), QClipboard::Clipboard);
    });

    QExplicitlySharedDataPointer<KDevelop::IAssistant> solution = problem->solutionAssistant();
    if (solution && !solution->actions().isEmpty()) {
        QList<QAction*> actions;
        const auto solutionActions = solution->actions();
        actions.reserve(solutionActions.size());
        for (auto assistantAction : solutionActions) {
            auto action = assistantAction->toQAction(m.data());
            action->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));
            actions << action;
        }

        QString title = solution->title();
        title = KDevelop::htmlToPlainText(title);
        title.replace(QLatin1String("&apos;"), QLatin1String("\'"));
        m->addSection(i18nc("@title:menu", "Solve: %1", title));
        m->addActions(actions);
    }

    m->exec(event->globalPos());
    delete m;

}

void ProblemTreeView::resizeEvent(QResizeEvent* event)
{
    QTreeView::resizeEvent(event);
    // resizeEvent() is invoked whenever this tree view is resized and also whenever the default system font
    // changes. So the resizeColumns() call below should cover all scenarios where heuristic column widths change.
    resizeColumns();
}

#include "problemtreeview.moc"
#include "moc_problemtreeview.cpp"
