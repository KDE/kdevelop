/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "environmentselectionwidget.h"
#include "environmentprofilelist.h"
#include "environmentselectionmodel.h"

#include <QHBoxLayout>

#include <KConfigDialogManager>
#include <KComboBox>

#include <interfaces/icore.h>

namespace KDevelop {

class EnvironmentSelectionWidgetPrivate
{
public:
    KComboBox* comboBox;
    EnvironmentSelectionModel* model;
    EnvironmentSelectionWidget* owner;

    explicit EnvironmentSelectionWidgetPrivate(EnvironmentSelectionWidget* _owner)
        : comboBox(new KComboBox(_owner))
        , model(new EnvironmentSelectionModel(_owner))
        , owner(_owner)
    {
        comboBox->setModel(model);
        comboBox->setEditable(false);
    }
};

EnvironmentSelectionWidget::EnvironmentSelectionWidget(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new EnvironmentSelectionWidgetPrivate(this))
{
    Q_D(EnvironmentSelectionWidget);

    setLayout(new QHBoxLayout(this));
    layout()->addWidget(d->comboBox);
    layout()->setContentsMargins(0, 0, 0, 0);

    setCurrentProfile(QString());   // select the default profile

    connect(d->comboBox, &QComboBox::currentTextChanged,
            this, &EnvironmentSelectionWidget::currentProfileChanged);
}

EnvironmentSelectionWidget::~EnvironmentSelectionWidget() = default;

QString EnvironmentSelectionWidget::currentProfile() const
{
    Q_D(const EnvironmentSelectionWidget);

    return d->model->index(d->comboBox->currentIndex(), 0).data(Qt::EditRole).toString();
}

void EnvironmentSelectionWidget::setCurrentProfile(const QString& profile)
{
    Q_D(EnvironmentSelectionWidget);

    d->comboBox->setCurrentIndex(d->comboBox->findData(profile, Qt::EditRole));
    emit currentProfileChanged(profile);
}

void EnvironmentSelectionWidget::reconfigure()
{
    Q_D(EnvironmentSelectionWidget);

    QString selectedProfile = currentProfile();
    d->model->reload();
    setCurrentProfile(d->model->reloadSelectedItem(selectedProfile));
}

QString EnvironmentSelectionWidget::effectiveProfileName() const
{
    Q_D(const EnvironmentSelectionWidget);

    return d->model->index(d->comboBox->currentIndex(),
                           0).data(EnvironmentSelectionModel::EffectiveNameRole).toString();
}

EnvironmentProfileList EnvironmentSelectionWidget::environmentProfiles() const
{
    Q_D(const EnvironmentSelectionWidget);

    return d->model->environmentProfiles();
}

}
