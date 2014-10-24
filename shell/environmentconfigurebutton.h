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

#ifndef KDEVPLATFORM_ENVIRONMENTCONFIGUREBUTTON_H
#define KDEVPLATFORM_ENVIRONMENTCONFIGUREBUTTON_H

#include "shellexport.h"
#include <QPushButton>

namespace KDevelop
{

class EnvironmentSelectionWidget;

/**
 * A tool button that shows a dialog to configure the environment settings.
 * You want to place that next to an @c EnvironmentSelectionWidget and pass
 * that one along. This button will automatically update the selection widget
 * if required then.
 */
class KDEVPLATFORMSHELL_EXPORT EnvironmentConfigureButton : public QPushButton
{
    Q_OBJECT
public:
    explicit EnvironmentConfigureButton(QWidget* parent = 0);
    ~EnvironmentConfigureButton();

    void setSelectionWidget(EnvironmentSelectionWidget* widget);

signals:
    /**
     * Gets emitted whenever the dialog was acceppted
     * and the env settings might have changed.
     */
    void environmentConfigured();

private:
    class EnvironmentConfigureButtonPrivate* const d;
    friend class EnvironmentConfigureButtonPrivate;
};

}

#endif // KDEVPLATFORM_ENVIRONMENTCONFIGUREBUTTON_H
