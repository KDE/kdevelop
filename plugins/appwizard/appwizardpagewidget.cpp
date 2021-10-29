/*
    SPDX-FileCopyrightText: 2009 Kris Wong <kris.p.wong@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appwizardpagewidget.h"

AppWizardPageWidget::AppWizardPageWidget(QWidget* parent)
    : QWidget(parent)
{
}

AppWizardPageWidget::~AppWizardPageWidget()
{
}

bool AppWizardPageWidget::shouldContinue()
{
    return true;
}
