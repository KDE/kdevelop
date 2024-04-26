/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "processselection.h"

#include <util/scopeddialog.h>

#include <processcore/process.h>
#include <processcore/process_data_model.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KUser>

#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTreeView>

using namespace KDevMI;

static constexpr bool isSystemUser(unsigned int userId)
{
    constexpr unsigned int minimumNonSystemId = 1000;
    constexpr unsigned int maximumNonSystemId = 65533;
    return userId < minimumNonSystemId || userId > maximumNonSystemId;
}

class ProcessesSortFilterModel : public QSortFilterProxyModel
{
public:
    enum class ProcessOwner { Self, Users, System, All };

    explicit ProcessesSortFilterModel(int uidColumn, QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
        , m_uidColumn(uidColumn)
    {
        setSortRole(KSysGuard::ProcessDataModel::Value);
        setSortCaseSensitivity(Qt::CaseInsensitive);
        setSortLocaleAware(true);

        setFilterRole(KSysGuard::ProcessDataModel::Value);
        setFilterCaseSensitivity(Qt::CaseInsensitive);
        setRecursiveFilteringEnabled(true);
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
    {
        const auto uid = sourceModel()->data(sourceModel()->index(sourceRow, m_uidColumn, sourceParent)).toUInt();

        bool accept = true;
        switch (m_processOwner) {
        case ProcessOwner::Self:
            accept = m_currentUserUid == uid;
            break;
        case ProcessOwner::Users:
            accept = !isSystemUser(uid);
            break;
        case ProcessOwner::System:
            accept = isSystemUser(uid);
            break;
        case ProcessOwner::All:
            break;
        }

        if (!accept) {
            return false;
        }

        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }

    bool filterAcceptsColumn(int sourceColumn, const QModelIndex& sourceParent) const override
    {
        if (sourceColumn == m_uidColumn) {
            return false;
        }

        return QSortFilterProxyModel::filterAcceptsColumn(sourceColumn, sourceParent);
    }

    ProcessOwner filterProcessOwner() const
    {
        return m_processOwner;
    }

    void setFilterProcessOwner(ProcessOwner owner)
    {
        m_processOwner = owner;
        invalidateFilter();
    }

private:
    const uint m_currentUserUid = KUserId::currentEffectiveUserId().nativeId();
    const int m_uidColumn;
    ProcessOwner m_processOwner = ProcessOwner::Self;
};

ProcessSelectionDialog::ProcessSelectionDialog(QWidget* parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    auto* const view = m_ui.view;

    const QStringList attributes = {QStringLiteral("name"),      QStringLiteral("pid"),     QStringLiteral("username"),
                                    QStringLiteral("startTime"), QStringLiteral("command"), QStringLiteral("euid")};

    m_dataModel = new KSysGuard::ProcessDataModel(this);
    m_dataModel->setEnabledAttributes(attributes);

    m_sortModel = new ProcessesSortFilterModel(attributes.indexOf(QStringLiteral("euid")), this);
    m_sortModel->setSourceModel(m_dataModel);

    view->setModel(m_sortModel);

    m_pidColumn = attributes.indexOf(QStringLiteral("pid"));

    connect(m_ui.filterEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_sortModel->setFilterFixedString(text);
    });

    connect(m_ui.processesCombo, &QComboBox::activated, this, &ProcessSelectionDialog::onProcessesComboActivated);

    connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &ProcessSelectionDialog::selectionChanged);

    connect(m_ui.buttonList, &QToolButton::toggled, this, [this](bool checked) {
        m_ui.buttonTree->setChecked(!checked);
    });

    connect(m_ui.buttonTree, &QToolButton::toggled, this, [this](bool checked) {
        m_ui.view->clearSelection();
        m_dataModel->setFlatList(!checked);
        m_ui.buttonList->setChecked(!checked);
        m_ui.view->expandAll();
    });

    auto* const buttonBox = m_ui.buttonBox;
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_attachButton = buttonBox->button(QDialogButtonBox::Ok);
    m_attachButton->setDefault(true);
    m_attachButton->setText(i18nc("@action:button", "Attach"));
    m_attachButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    m_attachButton->setEnabled(false);

    const KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("ProcessSelectionDialog"));
    m_ui.filterEdit->setText(config.readEntry("filterText", QString()));

    restoreGeometry(config.readEntry("dialogGeometry", QByteArray{}));

    m_ui.processesCombo->setCurrentIndex(config.readEntry("processOwner", 0));
    onProcessesComboActivated(m_ui.processesCombo->currentIndex());

    const auto headerState = config.readEntry("headerState", QByteArray{});
    if (headerState.isEmpty()) {
        m_ui.view->sortByColumn(0, Qt::SortOrder::AscendingOrder);
        m_ui.view->setColumnWidth(0, 250);
    } else {
        m_ui.view->header()->restoreState(headerState);
    }

    if (config.readEntry("treeView", false)) {
        m_ui.buttonTree->toggle();
    }
}

ProcessSelectionDialog::~ProcessSelectionDialog()
{
    KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("ProcessSelectionDialog"));
    config.writeEntry("filterText", m_ui.filterEdit->text());
    config.writeEntry("dialogGeometry", saveGeometry());
    config.writeEntry("processOwner", static_cast<int>(m_sortModel->filterProcessOwner()));
    config.writeEntry("headerState", m_ui.view->header()->saveState());
    config.writeEntry("treeView", m_ui.buttonTree->isChecked());
}

long long ProcessSelectionDialog::pidSelected() const
{
    const auto indexes = m_ui.view->selectionModel()->selectedIndexes();
    return m_sortModel->data(indexes.at(m_pidColumn), KSysGuard::ProcessDataModel::Value).toLongLong();
}

void ProcessSelectionDialog::selectionChanged(const QItemSelection& newSelection,
                                              const QItemSelection& /*oldSelection*/)
{
    m_attachButton->setEnabled(!newSelection.isEmpty());
}

void ProcessSelectionDialog::onProcessesComboActivated(int index)
{
    m_sortModel->setFilterProcessOwner(static_cast<ProcessesSortFilterModel::ProcessOwner>(index));
}

long long KDevMI::askUserForProcessId(QWidget* dialogParent)
{
    const KDevelop::ScopedDialog<ProcessSelectionDialog> dlg(dialogParent);
    if (dlg->exec()) {
        return dlg->pidSelected();
    }
    return 0LL;
}

#include "moc_processselection.cpp"
