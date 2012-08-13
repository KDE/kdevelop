/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "testoutputpage.h"

#include <language/codegen/sourcefiletemplate.h>

#include <KUrl>
#include <QHash>
#include <QFormLayout>
#include <QLabel>

TestOutputPage::TestOutputPage(QWidget* parent)
 : QWidget(parent)
{
    m_layout = new QFormLayout(this);
    setLayout(m_layout);
}

TestOutputPage::~TestOutputPage()
{

}

void TestOutputPage::setFileUrls(const TestOutputPage::UrlHash& urls, const QHash<QString,QString>& labels)
{
    for (UrlHash::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        KUrlRequester* r = new KUrlRequester(it.value(), this);
        m_layout->addRow(new QLabel(labels[it.key()]), r);
        m_requesters.insert(it.key(), r);
    }
}

TestOutputPage::UrlHash TestOutputPage::fileUrls() const
{
    UrlHash urls;
    QHash<QString,KUrlRequester*>::const_iterator it;
    for (it = m_requesters.constBegin(); it != m_requesters.constEnd(); ++it)
    {
        urls.insert(it.key(), it.value()->url());
    }
    return urls;
}