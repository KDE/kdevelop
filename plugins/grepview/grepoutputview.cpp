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
#include <QLineEdit>
#include <QMenu>
#include <QValidator>
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

/**
 * Regardless of the input text, this validator class accepts if and only if the widget it tracks is enabled.
 *
 * If the tracked widget is disabled, the validation result is Intermediate to allow editing text
 * but prevent the emission of QLineEdit::editingFinished() and QLineEdit::returnPressed().
 */
class WidgetEnabledValidator : public QValidator
{
public:
    /**
     * Constructs the validator
     *
     * @param widget the widget to track and set as this validator's parent
     *
     * @note Setting the tracked widget as the validator's parent makes checking
     *       the enabled state safe. The widget, on which the validator is set,
     *       should support the destruction of the validator. For example, QLineEdit's
     *       validator is stored in a QPointer and checked for null before access.
     */
    explicit WidgetEnabledValidator(QWidget& widget)
        : QValidator(&widget)
        , m_widget(widget)
    {
    }

    [[nodiscard]] State validate(QString& input, int& pos) const override
    {
        Q_UNUSED(input)
        Q_UNUSED(pos)
        return m_widget.isEnabled() ? Acceptable : Intermediate;
    }

private:
    const QWidget& m_widget;
};
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
    resultsTreeView->setUniformRowHeights(true);
    resultsTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    resultsTreeView->header()->setStretchLastSection(true);

    connect(resultsTreeView, &QTreeView::activated, this, [this](const QModelIndex& index) {
        if (auto* const model = this->model()) {
            model->activate(index);
        }
    });

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

    {
        auto* const replacementLineEdit = replacementCombo->lineEdit();
        Q_ASSERT(replacementLineEdit);

        // Do not invoke onApply() and do not insert an item with the current text into replacementCombo
        // if the apply button is disabled. The apply button is disabled while replacement is forbidden.
        replacementLineEdit->setValidator(new WidgetEnabledValidator(*applyButton));

        connect(replacementLineEdit, &QLineEdit::textChanged, this, &GrepOutputView::replacementTextChanged);
        connect(replacementLineEdit, &QLineEdit::returnPressed, this, &GrepOutputView::onApply);
    }

    connect(newSearchAction, &QAction::triggered, this, &GrepOutputView::showDialog);

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

            settingsHistory.push_back(settings);
        }

        // m_restoredSettingsHistory is initialized to true. Thus if this branch is not taken (due to an empty or
        // invalid search settings config entry), we assume that search settings have been restored from history.
        // This is necessary because addModelsFromHistory() is called only if we call GrepDialog::historySearch()
        // below. And without a call to addModelsFromHistory(), m_restoredSettingsHistory is never again
        // set to true, which prevents our destructor from saving search settings history.
        m_restoredSettingsHistory = false;

        // Restore the grep jobs with settings from the history without performing a search.
        auto* const dlg = new GrepDialog(m_plugin, this, this, false);
        dlg->historySearch(std::move(settingsHistory));
    }
}

void GrepOutputView::replacementTextChanged(const QString& replacementText)
{
    if (auto* const model = this->model()) {
        model->setReplacement(replacementText);
        if (!replacementText.isEmpty()) {
            model->makeItemsCheckable(true);
        }
    }
}

GrepOutputView::~GrepOutputView()
{
    KConfigGroup cg = ICore::self()->activeSession()->config()->group(QStringLiteral("GrepDialog"));
    cg.writeEntry("LastReplacementItems", qCombo2StringList(replacementCombo, true));
    if (m_restoredSettingsHistory) {
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
    }

    emit outputViewIsClosed();
}

void GrepOutputView::addModelsFromHistory(QList<GrepJobSettings>&& settingsHistory,
                                          const QStringList& searchDescriptions)
{
    Q_ASSERT(settingsHistory.size() == searchDescriptions.size());
    Q_ASSERT(!settingsHistory.empty());

    Q_ASSERT(!m_restoredSettingsHistory);

    // If searches are performed before models are restored from history (which occurs once all
    // projects in the session are opened), the models from history should not appear more recent
    // in the UI or replace the models of the new searches. Therefore, calculate the maximum number
    // of models to restore and display the restored models as older than the new models (if any).
    const auto maxHistoryEntriesToRestore = HISTORY_SIZE - modelSelector->count();
    if (settingsHistory.size() > maxHistoryEntriesToRestore) {
        // remove the oldest history entries if too many
        settingsHistory.remove(0, settingsHistory.size() - maxHistoryEntriesToRestore);
    }

    // If modelSelector is currently empty (it usually *is* empty when search history is restored),
    // the first inserted model becomes active. As the last history entry's model should be
    // activated in this scenario, insert it first. If modelSelector is not empty, the models
    // restored from history are older than the already present models, and so are not activated.
    const auto begin = searchDescriptions.crbegin();
    const auto end = begin + settingsHistory.size();
    for (auto it = begin; it != end; ++it) {
        auto* const newModel = createModel();
        modelSelector->addItem(*it, QVariant::fromValue<QObject*>(newModel));
    }

    // Prepend settingsHistory to m_settingsHistory (the QList::prepend(QList<T> &&) overload is missing).
    settingsHistory.append(std::move(m_settingsHistory));
    m_settingsHistory = std::move(settingsHistory);

    Q_ASSERT(m_settingsHistory.size() == modelSelector->count());
    Q_ASSERT(m_settingsHistory.size() <= HISTORY_SIZE);

    m_restoredSettingsHistory = true;
}

GrepOutputModel* GrepOutputView::renewModel(const GrepJobSettings& settings, const QString& description)
{
    auto* const newModel = createModel();
    // appends new model to history
    modelSelector->insertItem(0, description, QVariant::fromValue<QObject*>(newModel));
    modelSelector->setCurrentIndex(0);

    m_settingsHistory.append(settings);

    removeOldestModelsIfTooMany();

    return newModel;
}

void GrepOutputView::removeOldestModelsIfTooMany()
{
    // This loop implementation is efficient in practice,
    // because the number of items in history cannot normally be greater than HISTORY_SIZE + 1.
    while (modelSelector->count() > HISTORY_SIZE) {
        const auto var = modelSelector->itemData(HISTORY_SIZE);
        qvariant_cast<QObject*>(var)->deleteLater();
        modelSelector->removeItem(HISTORY_SIZE);
    }
    while (m_settingsHistory.size() > HISTORY_SIZE) {
        m_settingsHistory.removeFirst();
    }
    Q_ASSERT(m_settingsHistory.size() == modelSelector->count());
}

GrepOutputModel* GrepOutputView::model() const
{
    return static_cast<GrepOutputModel*>(resultsTreeView->model());
}

bool GrepOutputView::isActiveModel(const GrepOutputModel* model) const
{
    Q_ASSERT(model);
    // If this->model() is not null but modelSelector->count() is 0, it means that we just cleared all
    // models while the active model's job was still running. The job is killed, but still emits signals
    // before destruction. The model forwards the signals to us, but we do not care about them anymore.
    return model == this->model() && modelSelector->count() != 0;
}

GrepOutputModel* GrepOutputView::createModel()
{
    auto* const model = new GrepOutputModel(resultsTreeView);
    connect(model, &GrepOutputModel::finishedAddingResults, this, &GrepOutputView::finishedAddingResults);
    connect(model, &GrepOutputModel::showMessage, this, &GrepOutputView::showMessage);
    connect(model, &GrepOutputModel::anyItemCheckedChanged, this, &GrepOutputView::anyItemCheckedChanged);
    return model;
}

void GrepOutputView::changeModel(int index)
{
    // after deleting the whole search history, index is -1
    if (index < 0) {
        return;
    }

    auto* model = this->model();

    const auto dataForIndex = modelSelector->itemData(index);
    auto* const modelForIndex = static_cast<GrepOutputModel*>(qvariant_cast<QObject*>(dataForIndex));
    if (modelForIndex == model) {
        // The selected model matches the active model, only its index changed -
        // probably because a new item was inserted at front. Nothing to do.
        return;
    }

    replacementCombo->clearEditText();

    if (model) {
        disconnect(model, &GrepOutputModel::rowsInserted, this, &GrepOutputView::rowsInserted);
        disconnect(model, &GrepOutputModel::rowsRemoved, this, &GrepOutputView::rowsRemoved);
    }

    resultsTreeView->setModel(modelForIndex);
    model = this->model();
    resultsTreeView->expandAll();

    connect(model, &GrepOutputModel::rowsInserted, this, &GrepOutputView::rowsInserted);
    connect(model, &GrepOutputModel::rowsRemoved, this, &GrepOutputView::rowsRemoved);

    model->showMessageEmit();
    applyButton->setEnabled(model->anyItemChecked());
    updateButtonState(model->hasResults());

    m_refresh->setEnabled(true);
    m_clearSearchHistory->setEnabled(true);

    updateScrollArea();
}

void GrepOutputView::finishedAddingResults(const GrepOutputModel* model)
{
    if (!isActiveModel(model)) {
        return;
    }
    // Optimization: update the width of resultsTreeView to fit its contents once all results
    // have been added to the active model rather than every time rows are added to it.
    updateScrollArea();
}

void GrepOutputView::showMessage(GrepOutputModel* model, MessageType type, const QString& message)
{
    if (!isActiveModel(model)) {
        return; // only the active model's messages belong to the status label
    }

    if (type == MessageType::Error) {
        QPalette palette = m_statusLabel->palette();
        KColorScheme::adjustForeground(palette, KColorScheme::NegativeText, QPalette::WindowText);
        m_statusLabel->setPalette(palette);
    } else {
        m_statusLabel->setPalette(QPalette());
    }
    m_statusLabel->setText(message);
}

void GrepOutputView::anyItemCheckedChanged(GrepOutputModel* model, bool anyItemChecked)
{
    if (!isActiveModel(model)) {
        return;
    }
    applyButton->setEnabled(anyItemChecked);
}

void GrepOutputView::onApply()
{
    auto* const model = this->model();
    Q_ASSERT(model);
    Q_ASSERT(model->anyItemChecked());

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
    model->doReplacements();
    setEnabled(true);
}

void GrepOutputView::showDialog()
{
    m_plugin->showDialog(true);
}

void GrepOutputView::refresh()
{
    const auto currentIndex = modelSelector->currentIndex();
    auto* const model = this->model();
    // The refresh action must be disabled when search history is empty.
    Q_ASSERT(currentIndex >= 0);
    Q_ASSERT(model);
    const auto currentSettingsHistoryIndex = m_settingsHistory.size() - 1 - currentIndex;

    auto* const dlg = new GrepDialog(m_plugin, this, this, false);
    auto result = dlg->refreshSearch(model, m_settingsHistory.at(currentSettingsHistoryIndex));
    if (!result.isValid()) {
        return; // the search was canceled by the user
    }

    Q_ASSERT_X(!model->hasResults(), Q_FUNC_INFO, "Refreshing a search must clear the model.");

    // After a model is cleared, its future items can become not checkable.
    // Clear the replacement text and make new items not checkable initially.
    replacementCombo->clearEditText();
    model->makeItemsCheckable(false);

    updateButtonState(false);

    // Insert the refreshed model and the updated description at the top of the combobox and
    // activate the newly inserted item *before* removing the original item in order to make
    // the changeModel() invocation return early, seeing as the model object remains the same.
    modelSelector->insertItem(0, result.description, QVariant::fromValue<QObject*>(model));
    modelSelector->setCurrentIndex(0);
    modelSelector->removeItem(currentIndex + 1); // +1 because inserting an item at the top moved the original item down

    m_settingsHistory.remove(currentSettingsHistoryIndex);
    m_settingsHistory.push_back(std::move(result.settings));
}

void GrepOutputView::rowsInserted(const QModelIndex& parent)
{
    if (!parent.isValid()) {
        // The buttons need to be enabled only when the first (root) item with an invalid parent index is inserted.
        updateButtonState(true);
    }

    resultsTreeView->expand(parent);
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

void GrepOutputView::clearSearchHistory()
{
    GrepJob *runningJob = m_plugin->grepJob();
    if(runningJob)
    {
        runningJob->kill();
    }

    for (int i = modelSelector->count() - 1; i >= 0; --i) {
        const auto var = modelSelector->itemData(i);
        qvariant_cast<QObject*>(var)->deleteLater();
    }
    modelSelector->clear();

    m_settingsHistory.clear();

    replacementCombo->clearEditText();
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
    const auto* const model = this->model();
    Q_ASSERT(model);
    for (int col = 0, columnCount = model->columnCount(); col < columnCount; ++col)
        resultsTreeView->resizeColumnToContents(col);
}

#include "moc_grepoutputview.cpp"
