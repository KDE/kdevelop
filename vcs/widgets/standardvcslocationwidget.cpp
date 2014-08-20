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
#include <QDebug>
#include <KUrlRequester>
#include <KLocalizedString>
#include <vcs/vcslocation.h>

using namespace KDevelop;

StandardVcsLocationWidget::StandardVcsLocationWidget(QWidget* parent, Qt::WindowFlags f)
    : VcsLocationWidget(parent, f)
{
    setLayout(new QVBoxLayout(this));
    m_urlWidget = new KUrlRequester(this);
    m_urlWidget->setClickMessage(i18n("Enter the repository URL..."));
    layout()->addWidget(m_urlWidget);
    
    connect(m_urlWidget, SIGNAL(textChanged(QString)), SLOT(textChanged(QString)));
}

KUrl StandardVcsLocationWidget::url() const
{
    return m_urlWidget->url();
}

VcsLocation StandardVcsLocationWidget::location() const
{
    VcsLocation v(m_urlWidget->url());
    return v;
}

bool StandardVcsLocationWidget::isCorrect() const
{
    return !m_urlWidget->url().isEmpty();
}

void StandardVcsLocationWidget::textChanged(const QString& /*str*/)
{
    emit changed();
}

QString KDevelop::StandardVcsLocationWidget::projectName() const
{
    return m_urlWidget->url().fileName();
}
