/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ENVIRONMENTCONFIGUREBUTTON_H
#define KDEVPLATFORM_ENVIRONMENTCONFIGUREBUTTON_H

#include "shellexport.h"
#include <QPushButton>

namespace KDevelop
{

class EnvironmentSelectionWidget;
class EnvironmentConfigureButtonPrivate;

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
    explicit EnvironmentConfigureButton(QWidget* parent = nullptr);
    ~EnvironmentConfigureButton() override;

    void setSelectionWidget(EnvironmentSelectionWidget* widget);

Q_SIGNALS:
    /**
     * Gets emitted whenever the dialog was accepted
     * and the env settings might have changed.
     */
    void environmentConfigured();

private:
    const QScopedPointer<class EnvironmentConfigureButtonPrivate> d_ptr;
    Q_DECLARE_PRIVATE(EnvironmentConfigureButton)

    friend class EnvironmentConfigureButtonPrivate;
};

}

#endif // KDEVPLATFORM_ENVIRONMENTCONFIGUREBUTTON_H
