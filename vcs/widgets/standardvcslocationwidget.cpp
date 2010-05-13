/* KDevPlatform Vcs Support
 *
 * Copyright 2010 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "standardvcslocationwidget.h"
#include <QVBoxLayout>
#include <KUrlRequester>
#include <vcs/vcslocation.h>
#include <KLocalizedString>

using namespace KDevelop;

StandardVcsLocationWidget::StandardVcsLocationWidget(QWidget* parent, Qt::WindowFlags f): VcsLocationWidget(parent, f)
{
    setLayout(new QVBoxLayout(this));
    m_urlWidget = new KUrlRequester(this);
    m_urlWidget->setClickMessage(i18n("Introduce the repository URL..."));
    layout()->addWidget(m_urlWidget);
}

VcsLocation StandardVcsLocationWidget::location() const
{
    return VcsLocation(m_urlWidget->url());
}

bool KDevelop::StandardVcsLocationWidget::isCorrect() const
{
    return !m_urlWidget->url().isRelative();
}
