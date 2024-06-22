/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checkswidget.h"
#include "ui_checkswidget.h"

#include "checksdb.h"
#include "debug.h"

#include <KLocalizedString>

#include <QMenu>

namespace Clazy
{

enum DataRole {
    CheckRole = Qt::UserRole + 1,
    DescriptionRole = Qt::UserRole + 2
};

enum ItemType {
    LevelType,
    CheckType
};

ChecksWidget::ChecksWidget(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::ChecksWidget)
{
    m_ui->setupUi(this);

    m_ui->filterEdit->addTreeWidget(m_ui->checksTree);
    m_ui->filterEdit->setPlaceholderText(i18nc("@info:placeholder", "Search checks..."));
    connect(m_ui->filterEdit, &KTreeWidgetSearchLine::searchUpdated, this, &ChecksWidget::searchUpdated);
}

void ChecksWidget::setChecksDb(const QSharedPointer<const ChecksDB>& db)
{
    auto resetMenu = new QMenu(this);
    m_ui->resetButton->setMenu(resetMenu);

    for (auto level : db->levels()) {
        auto levelItem = new QTreeWidgetItem(m_ui->checksTree, { level->displayName }, LevelType);
        levelItem->setData(0, CheckRole, level->name);
        levelItem->setData(0, DescriptionRole, level->description);
        levelItem->setCheckState(0, Qt::Unchecked);

        m_items[level->name] = levelItem;

        auto levelAction = resetMenu->addAction(level->displayName);
        connect(levelAction, &QAction::triggered, this, [this, level, levelItem]() {
            {
                // Block QLineEdit::textChanged() signal, which is used by KTreeWidgetSearchLine to
                // start delayed search.
                QSignalBlocker blocker(m_ui->filterEdit);
                m_ui->filterEdit->clear();
            }
            m_ui->filterEdit->updateSearch();

            setChecks(level->name);
            m_ui->checksTree->setCurrentItem(levelItem);
        });

        for (auto check : std::as_const(level->checks)) {
            auto checkItem = new QTreeWidgetItem(levelItem, { check->name }, CheckType);
            checkItem->setData(0, CheckRole, check->name);
            checkItem->setData(0, DescriptionRole, check->description);
            checkItem->setCheckState(0, Qt::Unchecked);

            m_items[check->name] = checkItem;
        }
    }

    connect(m_ui->checksTree, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item) {
        setState(item, item->checkState(0));
        updateChecks();
    });

    connect(m_ui->checksTree, &QTreeWidget::currentItemChanged, this, [this, db](QTreeWidgetItem* current) {
        if (current) {
            m_ui->descriptionView->setText(current->data(0, DescriptionRole).toString());
        } else {
            m_ui->descriptionView->clear();
        }
    });
}

ChecksWidget::~ChecksWidget() = default;

QString ChecksWidget::checks() const
{
    return m_checks;
}

void ChecksWidget::setChecks(const QString& checks)
{
    if (m_checks == checks) {
        return;
    }

    // Clear all selections
    for (int i = 0 ; i < m_ui->checksTree->topLevelItemCount(); ++i) {
        setState(m_ui->checksTree->topLevelItem(i), Qt::Unchecked);
    }

    const auto checksList = QStringView{checks}.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (auto rawCheckName : checksList) {
        rawCheckName = rawCheckName.trimmed();
        if (rawCheckName == QLatin1String("manual")) {
            continue;
        }

        auto checkName = rawCheckName.toString();
        auto state = Qt::Checked;
        if (checkName.startsWith(QLatin1String("no-"))) {
            checkName.remove(0, 3);
            state = Qt::Unchecked;
        }

        if (auto checkItem = m_items.value(checkName, nullptr)) {
            setState(checkItem, state);
        }
    }

    updateChecks();
    m_ui->checksTree->setCurrentItem(nullptr);
}

QStringList levelChecks(
    const QTreeWidget* checksTree,
    const QString& levelName,
    const QList<const QTreeWidgetItem*>& levelItems)
{
    QStringList checksList;
    if (!levelName.isEmpty()) {
        checksList += levelName;
    }

    for (int i = 0; i < checksTree->topLevelItemCount(); ++i) {
        const auto levelItem = checksTree->topLevelItem(i);
        const bool insideLevel = levelItems.contains(levelItem);

        for (int j = 0; j < levelItem->childCount(); ++j) {
            auto checkItem = levelItem->child(j);
            auto checkName = checkItem->data(0, CheckRole).toString();

            if (insideLevel) {
                if (checkItem->checkState(0) == Qt::Unchecked) {
                    checksList += QStringLiteral("no-%1").arg(checkName);
                }
            } else {
                if (checkItem->checkState(0) == Qt::Checked) {
                    checksList += checkName;
                }
            }
        }
    }

    return checksList;
}

void ChecksWidget::updateChecks()
{
    QStringList checksList;
    QList<const QTreeWidgetItem*> levelItems;

    // Here we try to find "best" (shortest) checks representation. To do this we build checks list
    // for every level and test it's size.
    for (int i = 0; i < m_ui->checksTree->topLevelItemCount(); ++i) {
        auto levelItem = m_ui->checksTree->topLevelItem(i);
        auto levelName = levelItem->data(0, CheckRole).toString();

        if (levelName == QLatin1String("manual")) {
            // Manual level is "fake level" so we clear the name and will store only
            // selected checks.
            levelItems.clear();
            levelName.clear();
        } else {
            levelItems += levelItem;
        }

        auto levelList = levelChecks(m_ui->checksTree, levelName, levelItems);
        if (checksList.isEmpty() || checksList.size() > levelList.size()) {
            checksList = levelList;
        }
    }

    m_ui->messageLabel->setVisible(checksList.isEmpty());

    auto checks = checksList.join(QLatin1Char(','));
    if (m_checks != checks) {
        m_checks = checks;
        emit checksChanged(m_checks);
    }
}

void ChecksWidget::setState(QTreeWidgetItem* item, Qt::CheckState state, bool force)
{
    Q_ASSERT(item);

    QSignalBlocker blocker(m_ui->checksTree);

    if (item->type() == LevelType) {
        if (state == Qt::Checked) {
            // When we enable some non-manual level item, we should also try to enable all
            // upper level items. We enable upper item only when it's state is Qt::Unchecked.
            // If the state is Qt::PartiallyChecked we assume that it was configured earlier and
            // we should skip the item to keep user's checks selection.
            const int index = m_ui->checksTree->indexOfTopLevelItem(item);
            if (index > 0 && index < (m_ui->checksTree->topLevelItemCount() - 1)) {
                setState(m_ui->checksTree->topLevelItem(index - 1), state, false);
            }

            if (item->checkState(0) != Qt::Unchecked && !force) {
                return;
            }
        }

        item->setCheckState(0, state);
        if (state != Qt::PartiallyChecked) {
            for (int i = 0; i < item->childCount(); ++i) {
                item->child(i)->setCheckState(0, state);
            }
        }
        return;
    }

    item->setCheckState(0, state);

    auto levelItem = item->parent();
    Q_ASSERT(levelItem);

    const int childCount = levelItem->childCount();
    int checkedCount = 0;

    for (int i = 0; i < childCount; ++i) {
        if (levelItem->child(i)->checkState(0) == Qt::Checked) {
            ++checkedCount;
        }
    }

    if (checkedCount == 0) {
        setState(levelItem, Qt::Unchecked);
    } else if (checkedCount == childCount) {
        setState(levelItem, Qt::Checked);
    } else {
        setState(levelItem, Qt::PartiallyChecked);
    }
}

void ChecksWidget::searchUpdated(const QString& searchString)
{
    if (searchString.isEmpty()) {
        m_ui->checksTree->collapseAll();
        m_ui->checksTree->setCurrentItem(nullptr);
        return;
    }

    m_ui->checksTree->expandAll();

    QTreeWidgetItem* firstVisibleLevel = nullptr;
    for (int i = 0; i < m_ui->checksTree->topLevelItemCount(); ++i) {
        auto levelItem = m_ui->checksTree->topLevelItem(i);
        if (levelItem->isHidden()) {
            continue;
        }

        if (!firstVisibleLevel) {
            firstVisibleLevel = levelItem;
        }

        for (int j = 0; j < levelItem->childCount(); ++j) {
            auto checkItem = levelItem->child(j);
            if (!checkItem->isHidden()) {
                m_ui->checksTree->setCurrentItem(checkItem);
                return;
            }
        }
    }

    m_ui->checksTree->setCurrentItem(firstVisibleLevel);
}

void ChecksWidget::setEditable(bool editable)
{
    if (m_isEditable == editable) {
        return;
    }

    m_isEditable = editable;

    m_ui->resetButton->setEnabled(editable);
    for (auto* item : std::as_const(m_items)) {
        auto flags = item->flags();
        flags.setFlag(Qt::ItemIsUserCheckable, m_isEditable);
        item->setFlags(flags);
    }
}

}

#include "moc_checkswidget.cpp"
