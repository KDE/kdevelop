/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebaukde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "customchecksetconfigproxywidget.h"

namespace Clazy
{

CustomCheckSetConfigProxyWidget::CustomCheckSetConfigProxyWidget(QWidget* parent)
    : QWidget(parent)
{
}

CustomCheckSetConfigProxyWidget::~CustomCheckSetConfigProxyWidget() = default;

QString CustomCheckSetConfigProxyWidget::checks() const
{
    return m_checks;
}

void CustomCheckSetConfigProxyWidget::setChecks(const QString& checks)
{
    if (m_checks == checks) {
        return;
    }

    m_checks = checks;
    emit checksChanged(m_checks);
}

}
