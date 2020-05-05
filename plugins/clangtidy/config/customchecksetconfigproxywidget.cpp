/* This file is part of KDevelop

   Copyright 2020 Friedrich W. H. Kossebau <kossebaukde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "customchecksetconfigproxywidget.h"

namespace ClangTidy
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
