/*
    SPDX-FileCopyrightText: 2023 George Florea Bănuș <georgefb899@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "outputwidgetconfig.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {
constexpr const char* maxViewCountConfigKey = "MaxOutputViewCount";
constexpr const char* isViewLimitEnabledConfigKey = "IsViewLimitEnabled";
}

OutputWidgetConfig::OutputWidgetConfig(const QString& configSubgroupName, const QString& toolViewTitle,
                                       QObject* parent)
    : QObject(parent)
    , m_configSubgroupName{configSubgroupName}
    , m_toolViewTitle{toolViewTitle}
{
}

KConfigGroup OutputWidgetConfig::configSubgroup() const
{
    // group is mutable, because the returned subgroup becomes read-only if group is const.
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("StandardOutputView"))   ;
    return KConfigGroup(&group, m_configSubgroupName);
}

int OutputWidgetConfig::maxViewCount(const KConfigGroup& configSubgroup) const
{
    return configSubgroup.readEntry(maxViewCountConfigKey, 10);
}

bool OutputWidgetConfig::isViewLimitEnabled(const KConfigGroup& configSubgroup) const
{
    return configSubgroup.readEntry(isViewLimitEnabledConfigKey, false);
}

std::optional<int> OutputWidgetConfig::maxViewCount() const
{
    const auto config = configSubgroup();
    if (isViewLimitEnabled(config))
        return maxViewCount(config);
    return std::nullopt;
}

void OutputWidgetConfig::openDialog(QWidget* dialogParent)
{
    auto* const dialog = new QDialog(dialogParent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(
        i18nc("@title:window %1: output type, e.g. Build or Run", "Configure %1 Output", m_toolViewTitle));

    auto* const dialogLayout = new QVBoxLayout();
    dialog->setLayout(dialogLayout);

    auto* const formLayout = new QFormLayout();
    dialogLayout->addLayout(formLayout);

    const auto config = configSubgroup();

    auto* const checkboxLabel = new QLabel(i18nc("@option:check", "Limit the number of output views"), dialog);
    checkboxLabel->setToolTip(i18nc("@info:tooltip", "Automatically close oldest output views"));
    auto* const checkbox = new QCheckBox(dialog);
    checkbox->setToolTip(checkboxLabel->toolTip());
    checkbox->setChecked(isViewLimitEnabled(config));
    formLayout->addRow(checkboxLabel, checkbox);

    auto* const spinboxLabel = new QLabel(i18nc("@label:spinbox", "Maximum output view count"), dialog);
    spinboxLabel->setToolTip(
        i18nc("@info:tooltip", "The oldest output view is closed when\nthe number of views exceeds this value"));
    auto* const spinbox = new QSpinBox(dialog);
    spinbox->setToolTip(spinboxLabel->toolTip());
    spinbox->setRange(1, 99);
    spinbox->setValue(maxViewCount(config));

    const auto enableSpinbox = [spinbox, spinboxLabel](bool enable) {
        spinbox->setEnabled(enable);
        spinboxLabel->setEnabled(enable);
    };
    enableSpinbox(checkbox->isChecked());
    connect(checkbox, &QCheckBox::toggled, this, enableSpinbox);

    formLayout->addRow(spinboxLabel, spinbox);

    auto* const buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this, dialog, checkbox, spinbox]() {
        auto config = configSubgroup();
        config.writeEntry(isViewLimitEnabledConfigKey, checkbox->isChecked());
        config.writeEntry(maxViewCountConfigKey, spinbox->value());
        dialog->accept();
        Q_EMIT settingsChanged();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    dialogLayout->addStretch();
    dialogLayout->addWidget(buttonBox);
    dialog->open();
}

#include "moc_outputwidgetconfig.cpp"
