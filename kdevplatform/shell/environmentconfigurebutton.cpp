/*  This file is part of KDevelop

    Copyright 2010 Milian Wolff <mail@milianw.de>

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

#include "environmentconfigurebutton.h"

#include <util/environmentselectionwidget.h>
#include "settings/environmentpreferences.h"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QIcon>
#include <QVBoxLayout>

#include <util/scopeddialog.h>

#include <KLocalizedString>

namespace KDevelop {

class EnvironmentConfigureButtonPrivate
{
public:
    explicit EnvironmentConfigureButtonPrivate(EnvironmentConfigureButton* _q)
        : q(_q), selectionWidget(nullptr)
    {
    }

    void showDialog()
    {
        ScopedDialog<QDialog> dlg(qApp->activeWindow());
        QString selected;
        if (selectionWidget) {
            selected = selectionWidget->effectiveProfileName();
        }

        auto prefs = new EnvironmentPreferences(selected, q);

        // TODO: This should be implicit when constructing EnvironmentPreferences
        prefs->initConfigManager();
        prefs->reset();

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, dlg.data(), &QDialog::accept);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, dlg.data(), &QDialog::reject);
        auto layout = new QVBoxLayout;
        layout->addWidget(prefs);
        layout->addWidget(buttonBox);
        dlg->setLayout(layout);
        dlg->setWindowTitle(prefs->fullName());
        dlg->setWindowIcon(prefs->icon());
        dlg->resize(800, 600);
        if (dlg->exec() == QDialog::Accepted) {
            prefs->apply();
            emit q->environmentConfigured();
        }
    }

    EnvironmentConfigureButton* const q;
    EnvironmentSelectionWidget *selectionWidget;
};

EnvironmentConfigureButton::EnvironmentConfigureButton(QWidget* parent)
    : QPushButton(parent),
      d_ptr(new EnvironmentConfigureButtonPrivate(this))
{
    setText(QString());
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    setToolTip(i18n("Configure environment variables"));

    connect(this, &EnvironmentConfigureButton::clicked,
            this, [this] { Q_D(EnvironmentConfigureButton); d->showDialog(); });
}

EnvironmentConfigureButton::~EnvironmentConfigureButton() = default;

void EnvironmentConfigureButton::setSelectionWidget(EnvironmentSelectionWidget* widget)
{
    Q_D(EnvironmentConfigureButton);

    connect(this, &EnvironmentConfigureButton::environmentConfigured,
            widget, &EnvironmentSelectionWidget::reconfigure);
    d->selectionWidget = widget;
}


}

#include "moc_environmentconfigurebutton.cpp"
