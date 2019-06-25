/* This file is part of KDevelop
Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#include "environmentselectionwidget.h"
#include "environmentprofilelist.h"
#include "environmentselectionmodel.h"

#include <QHBoxLayout>

#include <kconfigwidgets_version.h>
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

    // since 5.32 the signal is by default taken as set for the used property
#if KCONFIGWIDGETS_VERSION < QT_VERSION_CHECK(5, 32, 0)
    KConfigDialogManager::changedMap()->insert(QStringLiteral("KDevelop::EnvironmentSelectionWidget"),
                                               SIGNAL(currentProfileChanged(QString)));
#endif

    setLayout(new QHBoxLayout(this));
    layout()->addWidget(d->comboBox);
    layout()->setMargin(0);

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
