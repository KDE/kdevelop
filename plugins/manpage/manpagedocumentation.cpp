/*  This file is part of KDevelop
    Copyright 2010 Yannick Motta <yannick.motta@gmail.com>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

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

#include "manpagedocumentation.h"

#include "manpageplugin.h"
#include "manpagedocumentationwidget.h"

#include <documentation/standarddocumentationview.h>

#include <KIO/TransferJob>
#include <KLocalizedString>

#include <QStandardPaths>


ManPagePlugin* ManPageDocumentation::s_provider=nullptr;

ManPageDocumentation::ManPageDocumentation(const QString& name, const QUrl& url)
    : m_url(url), m_name(name)
{
    KIO::StoredTransferJob* transferJob = KIO::storedGet(m_url, KIO::NoReload, KIO::HideProgressInfo);
    connect( transferJob, &KIO::StoredTransferJob::finished, this, &ManPageDocumentation::finished);
    transferJob->start();
}

void ManPageDocumentation::finished(KJob* j)
{
    KIO::StoredTransferJob* job = qobject_cast<KIO::StoredTransferJob*>(j);
    if(job && job->error()==0) {
        m_description = QString::fromUtf8(job->data());
    } else {
        m_description.clear();
    }
    emit descriptionChanged();
}

KDevelop::IDocumentationProvider* ManPageDocumentation::provider() const
{
    return s_provider;
}

QString ManPageDocumentation::description() const
{
    return m_description;
}

QWidget* ManPageDocumentation::documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent )
{
    KDevelop::StandardDocumentationView* view = new KDevelop::StandardDocumentationView(findWidget, parent);
    view->initZoom(provider()->name());
    view->setDocumentation(IDocumentation::Ptr(this));
    view->setDelegateLinks(true);
    QObject::connect(view, &KDevelop::StandardDocumentationView::linkClicked, ManPageDocumentation::s_provider->model(), &ManPageModel::showItemFromUrl);

    // apply custom style-sheet to normalize look of the page
    const QString cssFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kdevmanpage/manpagedocumentation.css"));
    view->setOverrideCss(QUrl::fromLocalFile(cssFile));
    return view;
}

bool ManPageDocumentation::providesWidget() const
{
    return false;
}

QWidget* ManPageHomeDocumentation::documentationWidget(KDevelop::DocumentationFindWidget *findWidget, QWidget *parent){
    Q_UNUSED(findWidget);
    return new ManPageDocumentationWidget(parent);
}


QString ManPageHomeDocumentation::name() const
{
    return i18n("Man Content Page");
}

KDevelop::IDocumentationProvider* ManPageHomeDocumentation::provider() const
{
    return ManPageDocumentation::s_provider;
}

