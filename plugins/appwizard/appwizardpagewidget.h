/*
    SPDX-FileCopyrightText: 2009 Kris Wong <kris.p.wong@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_APPWIZARDPAGEWIDGET_H
#define KDEVPLATFORM_PLUGIN_APPWIZARDPAGEWIDGET_H

#include <QWidget>

class AppWizardPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppWizardPageWidget(QWidget* parent = nullptr);
    ~AppWizardPageWidget() override;

    virtual bool shouldContinue();
};

#endif // KDEVPLATFORM_PLUGIN_APPWIZARDPAGEWIDGET_H
