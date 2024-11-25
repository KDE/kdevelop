/*
    SPDX-FileCopyrightText: 2010 Silvère Lestang <silvere.lestang@gmail.com>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grepoutputview.h"
#include "grepoutputmodel.h"
#include "grepoutputdelegate.h"
#include "ui_grepoutputview.h"
#include "grepviewplugin.h"
#include "grepdialog.h"
#include "greputil.h"
#include "grepjob.h"
#include "debug.h"

#include <interfaces/icore.h>
#include <interfaces/isession.h>

#include <KConfigGroup>
#include <KMessageBox>
#include <KColorScheme>
#include <KLocalizedString>

#include <QAction>
#include <QMenu>
#include <QWidgetAction>

using namespace KDevelop;

GrepOutputViewFactory::GrepOutputViewFactory(GrepViewPlugin* plugin)
: m_plugin(plugin)
{}

QWidget* GrepOutputViewFactory::create(QWidget* parent)
{
    return new GrepOutputView(parent, m_plugin);
}

Qt::DockWidgetArea GrepOutputViewFactory::defaultPosition() const
{
    return Qt::BottomDockWidgetArea;
}

QString GrepOutputViewFactory::id() const
{
    return QStringLiteral("org.kdevelop.GrepOutputView");
}


const int GrepOutputView::HISTORY_SIZE = 5;

namespace {
enum { GrepSettingsStorageItemCount = 10 };
}

GrepOutputView::GrepOutputView(QWidget* parent, GrepViewPlugin* plugin)
  : QWidget(parent)
  , m_next(nullptr)
  , m_prev(nullptr)
  , m_collapseAll(nullptr)
  , m_expandAll(nullptr)
  , m_refresh(nullptr)
  , m_clearSearchHistory(nullptr)
  , m_statusLabel(nullptr)
  , m_plugin(plugin)
{
    Ui::GrepOutputView::setupUi(this);

    setWindowTitle(i18nc("@title:window", "Find/Replace Output View"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("edit-find"), windowIcon()));

    m_prev = new QAction(QIcon::fromTheme(QStringLiteral("go-previous")), i18nc("@action", "&Previous Item"), this);
    m_next = new QAction(QIcon::fromTheme(QStringLiteral("go-next")), i18nc("@action", "&Next Item"), this);
    /* Expand-all and collapse-all icons were added to breeze with version 5.57. We use a fallback
     * icon here because we support older frameworks versions and oxygen doesn't have such an icon
     */
    m_collapseAll = new QAction(QIcon::fromTheme(QStringLiteral("collapse-all"),
                        QIcon::fromTheme(QStringLiteral("arrow-left-double"))), i18nc("@action", "C&ollapse All"), this);
    m_expandAll = new QAction(QIcon::fromTheme(QStringLiteral("expand-all"),
                        QIcon::fromTheme(QStringLiteral("arrow-right-double"))), i18nc("@action", "&Expand All"), this);
    updateButtonState(false);
    auto *separator = new QAction(this);
    separator->setSeparator(true);
    auto* newSearchAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18nc("@action", "New &Search"), this);
    m_refresh = new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18nc("@action", "Refresh"), this);
    m_refresh->setEnabled(false);
    m_clearSearchHistory = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear-list")), i18nc("@action", "Clear Search History"), this);
    m_clearSearchHistory->setEnabled(false);

    addAction(m_prev);
    addAction(m_next);
    addAction(m_collapseAll);
    addAction(m_expandAll);
    addAction(separator);
    addAction(newSearchAction);
    addAction(m_refresh);
    addAction(m_clearSearchHistory);

    separator = new QAction(this);
    separator->setSeparator(true);
    addAction(separator);

    auto *statusWidget = new QWidgetAction(this);
    m_statusLabel = new QLabel(this);
    statusWidget->setDefaultWidget(m_statusLabel);
    addAction(statusWidget);

    modelSelector->setEditable(false);
    modelSelector->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(modelSelector, &KComboBox::customContextMenuRequested,
            this, &GrepOutputView::modelSelectorContextMenu);
    connect(modelSelector, QOverload<int>::of(&KComboBox::currentIndexChanged),
            this, &GrepOutputView::changeModel);

    resultsTreeView->setItemDelegate(GrepOutputDelegate::self());
    resultsTreeView->setRootIsDecorated(false);
    resultsTreeView->setHeaderHidden(true);
    resultsTreeView->setUniformRowHeights(false);
    resultsTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(m_prev, &QAction::triggered, this, &GrepOutputView::selectPreviousItem);
    connect(m_next, &QAction::triggered, this, &GrepOutputView::selectNextItem);
    connect(m_collapseAll, &QAction::triggered, this, &GrepOutputView::collapseAllItems);
    connect(m_expandAll, &QAction::triggered, this, &GrepOutputView::expandAllItems);
    connect(applyButton, &QPushButton::clicked,  this, &GrepOutputView::onApply);
    connect(m_refresh, &QAction::triggered, this, &GrepOutputView::refresh);
    connect(m_clearSearchHistory, &QAction::triggered, this, &GrepOutputView::clearSearchHistory);
    KConfigGroup cg = ICore::self()->activeSession()->config()->group(QStringLiteral("GrepDialog"));
    replacementCombo->addItems( cg.readEntry("LastReplacementItems", QStringList()) );
    replacementCombo->setInsertPolicy(QComboBox::InsertAtTop);
    applyButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));

    connect(replacementCombo, &KComboBox::editTextChanged, this, &GrepOutputView::replacementTextChanged);
    connect(replacementCombo, QOverload<const QString&>::of(&KComboBox::returnPressed), this, &GrepOutputView::onApply);

    connect(newSearchAction, &QAction::triggered, this, &GrepOutputView::showDialog);

    resultsTreeView->header()->setStretchLastSection(true);

    resultsTreeView->header()->setStretchLastSection(true);

    // read Find/Replace settings history
    const QStringList s = cg.readEntry("LastSettings", QStringList());
    if (s.size() % GrepSettingsStorageItemCount != 0) {
        qCWarning(PLUGIN_GREPVIEW) << "Stored settings history has unexpected size:" << s;
    } else if (!s.empty()) {
        QList<GrepJobSettings> settingsHistory;
        settingsHistory.reserve(s.size() / GrepSettingsStorageItemCount);
        auto it = s.begin();
        while (it != s.end()) {
            GrepJobSettings settings;
            settings.projectFilesOnly = ((it++)->toUInt() != 0);
            settings.caseSensitive = ((it++)->toUInt() != 0);
            settings.regexp = ((it++)->toUInt() != 0);
            settings.depth = (it++)->toInt();
            settings.pattern = *(it++);
            settings.searchTemplate = *(it++);
            settings.replacementTemplate = *(it++);
            settings.files = *(it++);
            settings.exclude = *(it++);
            settings.searchPaths = *(it++);

            settings.fromHistory = true;
            settingsHistory.push_back(settings);
        }

        // Restore the grep jobs with settings from the history without performing a search.
        auto* const dlg = new GrepDialog(m_plugin, this, this, false);
        dlg->historySearch(std::move(settingsHistory));
    }

    updateCheckable();
}

void GrepOutputView::replacementTextChanged()
{
    updateCheckable();

    if (model()) {
        // see https://bugs.kde.org/show_bug.cgi?id=274902 - renewModel can trigger a call here without an active model
        updateApplyState(model()->index(0, 0), model()->index(0, 0));
    }
}

GrepOutputView::~GrepOutputView()
{
    KConfigGroup cg = ICore::self()->activeSession()->config()->group(QStringLiteral("GrepDialog"));
    cg.writeEntry("LastReplacementItems", qCombo2StringList(replacementCombo, true));
    QStringList settingsStrings;
    settingsStrings.reserve(m_settingsHistory.size() * GrepSettingsStorageItemCount);
    for (const GrepJobSettings& s : std::as_const(m_settingsHistory)) {
        settingsStrings
            << QString::number(s.projectFilesOnly ? 1 : 0)
            << QString::number(s.caseSensitive ? 1 : 0)
            << QString::number(s.regexp ? 1 : 0)
            << QString::number(s.depth)
            << s.pattern
            << s.searchTemplate
            << s.replacementTemplate
            << s.files
            << s.exclude
            << s.searchPaths;
    }
    cg.writeEntry("LastSettings", settingsStrings);
    emit outputViewIsClosed();
}

GrepOutputModel* GrepOutputView::renewModel(const GrepJobSettings& settings, const QString& description)
{
    // clear oldest model
    while(modelSelector->count() >= GrepOutputView::HISTORY_SIZE) {
        QVariant var = modelSelector->itemData(GrepOutputView::HISTORY_SIZE - 1);
        qvariant_cast<QObject*>(var)->deleteLater();
        modelSelector->removeItem(GrepOutputView::HISTORY_SIZE - 1);
    }

    while(m_settingsHistory.count() >= GrepOutputView::HISTORY_SIZE) {
        m_settingsHistory.removeFirst();
    }

    replacementCombo->clearEditText();

    auto* newModel = new GrepOutputModel(resultsTreeView);
    applyButton->setEnabled(false);
    // text may be already present
    newModel->setReplacement(replacementCombo->currentText());
    connect(newModel, &GrepOutputModel::rowsRemoved,
            this, &GrepOutputView::rowsRemoved);
    connect(resultsTreeView, &QTreeView::activated, newModel, &GrepOutputModel::activate);
    connect(replacementCombo, &KComboBox::editTextChanged, newModel, &GrepOutputModel::setReplacement);
    connect(newModel, &GrepOutputModel::rowsInserted, this, &GrepOutputView::expandElements);
    connect(newModel, &GrepOutputModel::showErrorMessage, this, &GrepOutputView::showErrorMessage);
    connect(m_plugin, &GrepViewPlugin::grepJobFinished, this, &GrepOutputView::updateScrollArea);

    // appends new model to history
    modelSelector->insertItem(0, description, QVariant::fromValue<QObject*>(newModel));
    modelSelector->setCurrentIndex(0);

    m_settingsHistory.append(settings);

    updateCheckable();

    return newModel;
}


GrepOutputModel* GrepOutputView::model()
{
    return static_cast<GrepOutputModel*>(resultsTreeView->model());
}

void GrepOutputView::changeModel(int index)
{
    if (model()) {
        disconnect(model(), &GrepOutputModel::showMessage,
                   this, &GrepOutputView::showMessage);
        disconnect(model(), &GrepOutputModel::dataChanged,
                   this, &GrepOutputView::updateApplyState);
    }

    replacementCombo->clearEditText();

    //after deleting the whole search history, index is -1
    if(index >= 0)
    {
        QVariant var = modelSelector->itemData(index);
        auto *resultModel = static_cast<GrepOutputModel *>(qvariant_cast<QObject*>(var));
        resultsTreeView->setModel(resultModel);
        resultsTreeView->expandAll();

        connect(model(), &GrepOutputModel::showMessage,
                this, &GrepOutputView::showMessage);
        connect(model(), &GrepOutputModel::dataChanged,
                this, &GrepOutputView::updateApplyState);
        model()->showMessageEmit();
        applyButton->setEnabled(model()->hasResults() &&
                                model()->getRootItem() &&
                                model()->getRootItem()->checkState() != Qt::Unchecked &&
                                !replacementCombo->currentText().isEmpty());
        if(model()->hasResults())
            expandElements(QModelIndex());
        else {
            updateButtonState(false);
        }
    }

    updateCheckable();
    updateApplyState(model()->index(0, 0), model()->index(0, 0));
    m_refresh->setEnabled(true);
    m_clearSearchHistory->setEnabled(true);
}

void GrepOutputView::setMessage(const QString& msg, MessageType type)
{
    if (type == Error) {
        QPalette palette = m_statusLabel->palette();
        KColorScheme::adjustForeground(palette, KColorScheme::NegativeText, QPalette::WindowText);
        m_statusLabel->setPalette(palette);
    } else {
        m_statusLabel->setPalette(QPalette());
    }
    m_statusLabel->setText(msg);
}

void GrepOutputView::showErrorMessage( const QString& errorMessage )
{
    setMessage(errorMessage, Error);
}

void GrepOutputView::showMessage(const QString& message)
{
    setMessage(message, Information);
}

void GrepOutputView::onApply()
{
    if(model())
    {
        Q_ASSERT(model()->rowCount());
        // ask a confirmation before an empty string replacement
        if (replacementCombo->currentText().length() == 0
            && KMessageBox::questionTwoActions(
                   this, i18n("Do you want to replace with an empty string?"),
                   i18nc("@title:window", "Start Replacement"),
                   KGuiItem(i18nc("@action:button", "Replace"), QStringLiteral("dialog-ok-apply")),
                   KStandardGuiItem::cancel())
                == KMessageBox::SecondaryAction) {
            return;
        }

        setEnabled(false);
        model()->doReplacements();
        setEnabled(true);
    }
}

void GrepOutputView::showDialog()
{
    m_plugin->showDialog(true);
}

void GrepOutputView::refresh()
{
    int index = modelSelector->currentIndex();
    if (index >= 0) {
        QVariant var = modelSelector->currentData();
        qvariant_cast<QObject*>(var)->deleteLater();
        modelSelector->removeItem(index);

        QVector<GrepJobSettings> refresh_history({
            m_settingsHistory.takeAt(m_settingsHistory.count() - 1 - index)
        });
        refresh_history.first().fromHistory = false;

        auto* const dlg = new GrepDialog(m_plugin, this, this, false);
        dlg->historySearch(std::move(refresh_history));
    }
}

void GrepOutputView::expandElements(const QModelIndex& index)
{
    updateButtonState(true);

    resultsTreeView->expand(index);
}

void GrepOutputView::updateButtonState(bool enable)
{
    m_prev->setEnabled(enable);
    m_next->setEnabled(enable);
    m_collapseAll->setEnabled(enable);
    m_expandAll->setEnabled(enable);
}

void GrepOutputView::selectPreviousItem()
{
    if (!model()) {
        return;
    }

    QModelIndex prev_idx = model()->previousItemIndex(resultsTreeView->currentIndex());
    if (prev_idx.isValid()) {
        resultsTreeView->setCurrentIndex(prev_idx);
        model()->activate(prev_idx);
    }
}

void GrepOutputView::selectNextItem()
{
    if (!model()) {
        return;
    }

    QModelIndex next_idx = model()->nextItemIndex(resultsTreeView->currentIndex());
    if (next_idx.isValid()) {
        resultsTreeView->setCurrentIndex(next_idx);
        model()->activate(next_idx);
    }
}


void GrepOutputView::collapseAllItems()
{
    // Collapse everything
    resultsTreeView->collapseAll();

    if (resultsTreeView->model()) {
        // Now reopen the first children, which correspond to the files.
        resultsTreeView->expand(resultsTreeView->model()->index(0, 0));
    }
}

void GrepOutputView::expandAllItems()
{
    resultsTreeView->expandAll();
}

void GrepOutputView::rowsRemoved()
{
    Q_ASSERT(model());

    updateButtonState(model()->rowCount() > 0);
}

void GrepOutputView::updateApplyState(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    Q_UNUSED(bottomRight);

    if (!model() || !model()->hasResults()) {
        applyButton->setEnabled(false);
        return;
    }

    // we only care about the root item
    if(!topLeft.parent().isValid())
    {
        applyButton->setEnabled(topLeft.data(Qt::CheckStateRole) != Qt::Unchecked && model()->itemsCheckable());
    }
}

void GrepOutputView::updateCheckable()
{
    if(model())
        model()->makeItemsCheckable(!replacementCombo->currentText().isEmpty() || model()->itemsCheckable());
}

void GrepOutputView::clearSearchHistory()
{
    GrepJob *runningJob = m_plugin->grepJob();
    if(runningJob)
    {
        connect(runningJob, &GrepJob::finished, this, [=]() {updateButtonState(false);});
        runningJob->kill();
    }
    while(modelSelector->count() > 0)
    {
        QVariant var = modelSelector->itemData(0);
        qvariant_cast<QObject*>(var)->deleteLater();
        modelSelector->removeItem(0);
    }

    m_settingsHistory.clear();

    applyButton->setEnabled(false);

    updateButtonState(false);
    m_refresh->setEnabled(false);
    m_clearSearchHistory->setEnabled(false);
    m_statusLabel->setText(QString());
}

void GrepOutputView::modelSelectorContextMenu(const QPoint& pos)
{
    QPoint globalPos = modelSelector->mapToGlobal(pos);
    QMenu myMenu(this);
    myMenu.addAction(m_clearSearchHistory);
    myMenu.exec(globalPos);
}

void GrepOutputView::updateScrollArea()
{
    if (!model()) {
        return;
    }

    for (int col = 0; col < model()->columnCount(); ++col)
        resultsTreeView->resizeColumnToContents(col);
}

#include "moc_grepoutputview.cpp"
