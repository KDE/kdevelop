/*
    SPDX-FileCopyrightText: 2010 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "standardvcslocationwidget.h"
#include <QVBoxLayout>
#include <KUrlRequester>
#include <KLocalizedString>
#include <vcs/vcslocation.h>

using namespace KDevelop;

StandardVcsLocationWidget::StandardVcsLocationWidget(QWidget* parent, Qt::WindowFlags f)
    : VcsLocationWidget(parent, f)
{
    auto* widgetLayout = new QVBoxLayout;
    widgetLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(widgetLayout);

    m_urlWidget = new KUrlRequester(this);
    m_urlWidget->setPlaceholderText(i18nc("@info:placeholder", "Enter the repository URL..."));
    layout()->addWidget(m_urlWidget);
    
    connect(m_urlWidget, &KUrlRequester::textChanged, this, &StandardVcsLocationWidget::textChanged);
}

void StandardVcsLocationWidget::setLocation(const QUrl& remoteLocation)
{
    setUrl(remoteLocation);
}

void StandardVcsLocationWidget::setUrl(const QUrl& url)
{
    m_urlWidget->setUrl(url);
}

QUrl StandardVcsLocationWidget::url() const
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
