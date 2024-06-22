/*
    SPDX-FileCopyrightText: 2018 Amish K. Naidu <amhndu@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scratchpadview.h"
#include "scratchpad.h"

#include <debug.h>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocument.h>
#include <sublime/message.h>

#include <KLocalizedString>

#include <QAction>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QWidgetAction>
#include <QLineEdit>
#include <QInputDialog>
#include <QPainter>
#include <QAbstractItemView>

// Use a delegate because the dataChanged signal doesn't tell us the previous name
class FileRenameDelegate
    : public QStyledItemDelegate
{
    Q_OBJECT

public:
    FileRenameDelegate(QObject* parent, Scratchpad* scratchpad)
        : QStyledItemDelegate(parent)
        , m_scratchpad(scratchpad)
    {
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
    {
        const QString previousName = index.data().toString();
        QStyledItemDelegate::setModelData(editor, model, index);
        const auto* proxyModel = static_cast<QAbstractProxyModel*>(model);
        m_scratchpad->renameScratch(proxyModel->mapToSource(index), previousName);
    }

private:
    Scratchpad* m_scratchpad;
};

EmptyMessageListView::EmptyMessageListView(QWidget* parent)
    : QListView(parent)
{
}

void EmptyMessageListView::paintEvent(QPaintEvent* event)
{
    if (model() && model()->rowCount(rootIndex()) > 0) {
        QListView::paintEvent(event);
    } else {
        QPainter painter(viewport());
        const auto margin =
            QMargins(parentWidget()->style()->pixelMetric(QStyle::PM_LayoutLeftMargin), 0,
                     parentWidget()->style()->pixelMetric(QStyle::PM_LayoutRightMargin), 0);
        painter.drawText(rect() - margin, Qt::AlignCenter | Qt::TextWordWrap, m_message);
    }
}

void EmptyMessageListView::setEmptyMessage(const QString& message)
{
    m_message = message;
}


ScratchpadView::ScratchpadView(QWidget* parent, Scratchpad* scratchpad)
    : QWidget(parent)
    , m_scratchpad(scratchpad)
{
    setupUi(this);

    setupActions();

    setWindowTitle(i18nc("@title:window", "Scratchpad"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("note")));

    auto* const modelProxy = new QSortFilterProxyModel(this);
    modelProxy->setSourceModel(m_scratchpad->model());
    modelProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    modelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    modelProxy->setSortRole(Qt::DisplayRole);
    connect(m_filter, &QLineEdit::textEdited,
            modelProxy, &QSortFilterProxyModel::setFilterWildcard);

    scratchView->setModel(modelProxy);
    scratchView->setItemDelegate(new FileRenameDelegate(this, m_scratchpad));
    scratchView->setEmptyMessage(i18n("Scratchpad lets you quickly run and experiment with code without a full project, and even store todos. Create a new scratch to start."));

    connect(scratchView, &QListView::activated, this, &ScratchpadView::scratchActivated);

    connect(m_scratchpad, &Scratchpad::actionFailed, this, [](const QString& messageText) {
        // TODO: could be also messagewidget inside toolview?
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        KDevelop::ICore::self()->uiController()->postMessage(message);
    });

    connect(commandWidget, &QLineEdit::returnPressed, this, &ScratchpadView::runSelectedScratch);
    connect(commandWidget, &QLineEdit::returnPressed, this, [this] {
        m_scratchpad->setCommand(proxyModel()->mapToSource(currentIndex()), commandWidget->text());
    });
    commandWidget->setToolTip(i18nc("@info:tooltip", "Command to run this scratch. '$f' will expand to the scratch path."));
    commandWidget->setPlaceholderText(commandWidget->toolTip());

    // change active scratch when changing document
    connect(KDevelop::ICore::self()->documentController(), &KDevelop::IDocumentController::documentActivated, this,
        [this](const KDevelop::IDocument* document) {
        if (document->url().isLocalFile()) {
            const auto* model = scratchView->model();
            const auto index = model->match(model->index(0, 0), Scratchpad::FullPathRole,
                                            document->url().toLocalFile()).value({});
            if (index.isValid()) {
                scratchView->setCurrentIndex(index);
            }
        }
    });

    connect(scratchView, &QAbstractItemView::pressed, this, &ScratchpadView::validateItemActions);

    validateItemActions();
}

void ScratchpadView::setupActions()
{
    auto* action = new QAction(QIcon::fromTheme(QStringLiteral("list-add")), i18nc("@action", "New Scratch"), this);
    connect(action, &QAction::triggered, this, &ScratchpadView::createScratch);
    addAction(action);

    action = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18nc("@action", "Remove Scratch"), this);
    connect(action, &QAction::triggered, this, [this] {
        m_scratchpad->removeScratch(proxyModel()->mapToSource(currentIndex()));
        validateItemActions();
    });
    addAction(action);
    m_itemActions.push_back(action);

    action = new QAction(QIcon::fromTheme(QStringLiteral("edit-rename")), i18nc("@action", "Rename Scratch"), this);
    connect(action, &QAction::triggered, this, [this] {
        scratchView->edit(scratchView->currentIndex());
    });
    addAction(action);
    m_itemActions.push_back(action);

    action = m_scratchpad->runAction();
    action->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    action->setText(i18nc("@action", "Run Scratch"));
    connect(action, &QAction::triggered, this, &ScratchpadView::runSelectedScratch);
    addAction(action);
    m_itemActions.push_back(action);

    m_filter = new QLineEdit(this);
    m_filter->setPlaceholderText(i18nc("@info:placeholder", "Filter..."));
    auto filterAction = new QWidgetAction(this);
    filterAction->setDefaultWidget(m_filter);
    addAction(filterAction);
}

void ScratchpadView::validateItemActions()
{
    bool enable = currentIndex().isValid();

    for (auto* action : std::as_const(m_itemActions)) {
        action->setEnabled(enable);
    }

    commandWidget->setReadOnly(!enable);
    if (!enable) {
        commandWidget->clear();
    }
    commandWidget->setText(currentIndex().data(Scratchpad::RunCommandRole).toString());
}

void ScratchpadView::runSelectedScratch()
{
    const auto sourceIndex = proxyModel()->mapToSource(currentIndex());
    if (auto* document = KDevelop::ICore::self()->documentController()->documentForUrl(
        QUrl::fromLocalFile(sourceIndex.data(Scratchpad::FullPathRole).toString()))) {
            document->save();
    }
    m_scratchpad->setCommand(sourceIndex, commandWidget->text());
    m_scratchpad->runScratch(sourceIndex);
}

void ScratchpadView::scratchActivated(const QModelIndex& index)
{
    validateItemActions();
    m_scratchpad->openScratch(proxyModel()->mapToSource(index));
}

void ScratchpadView::createScratch()
{
    QString name = QInputDialog::getText(this, i18nc("@title:window", "Create New Scratch"),
                                         i18nc("@label:textbox", "Name for scratch file:"),
                                         QLineEdit::Normal,
                                         QStringLiteral("example.cpp"));
    if (!name.isEmpty()) {
        m_scratchpad->createScratch(name);
    }
}

QAbstractProxyModel* ScratchpadView::proxyModel() const
{
    return static_cast<QAbstractProxyModel*>(scratchView->model());
}

QModelIndex ScratchpadView::currentIndex() const
{
    return scratchView->currentIndex();
}

#include "scratchpadview.moc"
#include "moc_emptymessagelistview.cpp"
#include "moc_scratchpadview.cpp"
