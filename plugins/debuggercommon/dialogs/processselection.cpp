/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "processselection.h"

#include <processcore/process_data_model.h>
#include <processcore/process.h>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KUser>

#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTreeView>

using namespace KDevMI;

class ProcessesSortFilterModel : public QSortFilterProxyModel
{
public:
    enum class ProcessOwner { Self, Users, System, All };

    explicit ProcessesSortFilterModel(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        setSortRole(KSysGuard::ProcessDataModel::Value);
        setSortCaseSensitivity(Qt::CaseInsensitive);
        setSortLocaleAware(true);

        setFilterRole(KSysGuard::ProcessDataModel::Value);
        setFilterCaseSensitivity(Qt::CaseInsensitive);
        setRecursiveFilteringEnabled(true);

        m_currentUserUid = KUserId::currentEffectiveUserId().nativeId();
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
    {
        auto uid = sourceModel()->data(sourceModel()->index(sourceRow, m_uidColumn, sourceParent)).toUInt();

        bool filtered = false;
        switch (m_processOwner) {
        case ProcessOwner::Self:
            filtered = m_currentUserUid != uid;
            break;
        case ProcessOwner::Users:
            filtered = uid < 1000;
            break;
        case ProcessOwner::System:
            filtered = uid >= 1000;
            break;
        case ProcessOwner::All:
            break;
        }

        if (filtered) {
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

    void setFilterProcessOwner(int uidColumn, ProcessOwner owner)
    {
        m_uidColumn = uidColumn;
        m_processOwner = owner;
        invalidateFilter();
    }

private:
    uint m_currentUserUid;
    int m_uidColumn = 0;
    ProcessOwner m_processOwner = ProcessOwner::Self;
};

ProcessSelectionDialog::ProcessSelectionDialog(QWidget* parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    auto view = m_ui.view;

    m_dataModel = new KSysGuard::ProcessDataModel(this);
    m_dataModel->setEnabledAttributes(
        {QStringLiteral("pid"), QStringLiteral("name"), QStringLiteral("command"), QStringLiteral("uid")});

    m_sortModel = new ProcessesSortFilterModel(this);
    m_sortModel->setSourceModel(m_dataModel);

    view->setModel(m_sortModel);
    view->setSortingEnabled(true);

    connect(m_ui.filterEdit, &QLineEdit::textEdited, this, [this](const QString& text) {
        m_sortModel->setFilterFixedString(text);
    });

    connect(m_ui.processesCombo, &QComboBox::activated, this, &ProcessSelectionDialog::onProcessesComboActivated);

    connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &ProcessSelectionDialog::selectionChanged);

    connect(m_ui.buttonList, &QToolButton::toggled, this, [this](bool checked) {
        if (checked) {
            m_dataModel->setFlatList(true);
            m_ui.buttonTree->setChecked(false);
            m_ui.view->setIndentation(0);
        }
    });

    connect(m_ui.buttonTree, &QToolButton::toggled, this, [this](bool checked) {
        if (checked) {
            m_dataModel->setFlatList(false);
            m_ui.buttonList->setChecked(false);
            m_ui.view->resetIndentation();
        }
    });

    auto buttonBox = m_ui.buttonBox;
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_attachButton = buttonBox->button(QDialogButtonBox::Ok);
    m_attachButton->setDefault(true);
    m_attachButton->setText(i18nc("@action:button", "Attach"));
    m_attachButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    m_attachButton->setEnabled(false);

    KConfigGroup config = KSharedConfig::openConfig()->group("GdbProcessSelectionDialog");
    m_ui.filterEdit->setText(config.readEntry("filterText", QString()));
    m_sortModel->setFilterFixedString(m_ui.filterEdit->text());

    m_sortModel->sort(config.readEntry("sortColumn", 0),
                      Qt::SortOrder(config.readEntry("sortOrder", int(Qt::AscendingOrder))));

    if (config.readEntry("treeView", false)) {
        m_ui.buttonTree->toggle();
    }

    m_ui.processesCombo->setCurrentIndex(config.readEntry("processOwner", 0));
    onProcessesComboActivated(m_ui.processesCombo->currentIndex());
}

ProcessSelectionDialog::~ProcessSelectionDialog()
{
    KConfigGroup config = KSharedConfig::openConfig()->group("GdbProcessSelectionDialog");
    config.writeEntry("filterText", m_ui.filterEdit->text());
    config.writeEntry("sortColumn", m_sortModel->sortColumn());
    config.writeEntry("sortOrder", int(m_sortModel->sortOrder()));
    config.writeEntry("processOwner", int(m_sortModel->filterProcessOwner()));
    config.writeEntry("treeView", m_ui.buttonTree->isChecked());
    config.writeEntry("dialogGeometry", saveGeometry());
}

long int ProcessSelectionDialog::pidSelected()
{
    auto selectedIndices = m_ui.view->selectionModel()->selectedIndexes();
    return m_sortModel->data(selectedIndices.at(0), KSysGuard::ProcessDataModel::Value).toInt();
}

void ProcessSelectionDialog::selectionChanged(const QItemSelection& selected)
{
    m_attachButton->setEnabled(selected.count());
}

void ProcessSelectionDialog::onProcessesComboActivated(int index)
{
    auto uidColumn = m_dataModel->enabledAttributes().indexOf(u"uid");

    m_sortModel->setFilterProcessOwner(uidColumn, static_cast<ProcessesSortFilterModel::ProcessOwner>(index));
}

#include "moc_processselection.cpp"
