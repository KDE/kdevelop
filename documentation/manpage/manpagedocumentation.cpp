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

#include <QLabel>
#include <KLocale>
#include <QTreeView>
#include <QHeaderView>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QStackedWidget>

#include "manpagedocumentation.h"
#include "manpageplugin.h"
#include "manpagedocumentationwidget.h"

#include <KIO/TransferJob>
#include <KIO/Job>
#include <kio/jobclasses.h>
#include <KStandardDirs>
#include <documentation/standarddocumentationview.h>

ManPagePlugin* ManPageDocumentation::s_provider=0;


ManPageDocumentation::ManPageDocumentation(const QString& name, const QUrl& url)
    : m_url(url), m_name(name)
{
    KIO::StoredTransferJob* transferJob = KIO::storedGet(m_url, KIO::NoReload, KIO::HideProgressInfo);
    connect( transferJob, SIGNAL(finished(KJob*)), SLOT(finished(KJob*)));
    transferJob->start();
}

void ManPageDocumentation::finished(KJob* j)
{
    KIO::StoredTransferJob* job = qobject_cast<KIO::StoredTransferJob*>(j);
    if(job && job->error()==0) {
        m_description = QString::fromUtf8(job->data());
    } else {
        m_description = i18n("Could not find any documentation for '%1'", m_name);
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
    view->setDocumentation(KSharedPtr<IDocumentation>(this));

    // apply custom style-sheet to normalize look of the page
    const QString cssFile = KStandardDirs::locate("data", "kdevmanpage/manpagedocumentation.css");
    QWebSettings* settings = view->settings();
    settings->setUserStyleSheetUrl(QUrl::fromLocalFile(cssFile));

    view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    QObject::connect(view, SIGNAL(linkClicked(QUrl)), ManPageDocumentation::s_provider->model(), SLOT(showItemFromUrl(QUrl)));
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

#include "manpagedocumentation.moc"
