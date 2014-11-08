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

#include "environmentselectionwidget.h"
#include "environmentgrouplist.h"

#include <KLocalizedString>
#include <KDialog>
#include <KCModuleProxy>
#include <KCModuleInfo>

#include <QApplication>
#include <QIcon>

namespace KDevelop {

class EnvironmentConfigureButtonPrivate
{
public:
    EnvironmentConfigureButtonPrivate(EnvironmentConfigureButton* _q)
        : q(_q), selectionWidget(0)
    {
    }

    void showDialog()
    {
        KDialog dlg(qApp->activeWindow());
        QStringList selected;
        if (selectionWidget) {
            selected << selectionWidget->effectiveProfileName();
        }

        KCModuleProxy proxy("kcm_kdev_envsettings", 0, selected);
        dlg.setMainWidget(&proxy);
        dlg.setWindowTitle(proxy.moduleInfo().moduleName());
        dlg.setWindowIcon(QIcon::fromTheme(proxy.moduleInfo().icon()));
        dlg.resize(480, 320);
        if (dlg.exec() == KDialog::Accepted) {
            proxy.save();
            emit q->environmentConfigured();
        }
    }

    EnvironmentConfigureButton *q;
    EnvironmentSelectionWidget *selectionWidget;
};

EnvironmentConfigureButton::EnvironmentConfigureButton(QWidget* parent)
    : QPushButton(parent),
      d(new EnvironmentConfigureButtonPrivate(this))
{
    setText(QString());
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setIcon(QIcon::fromTheme("configure"));
    setToolTip(i18n("Configure environment variables"));

    connect(this, &EnvironmentConfigureButton::clicked,
            this, [&] { d->showDialog(); });
}

EnvironmentConfigureButton::~EnvironmentConfigureButton()
{
    delete d;
}

void EnvironmentConfigureButton::setSelectionWidget(EnvironmentSelectionWidget* widget)
{
    connect(this, &EnvironmentConfigureButton::environmentConfigured,
            widget, &EnvironmentSelectionWidget::reconfigure);
    d->selectionWidget = widget;
}


}

#include "moc_environmentconfigurebutton.cpp"
