/*
    SPDX-FileCopyrightText: 2010 Yannick Motta <yannick.motta@gmail.com>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    auto* job = qobject_cast<KIO::StoredTransferJob*>(j);
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
    auto* view = new KDevelop::StandardDocumentationView(findWidget, parent);
    view->initZoom(provider()->name());
    view->setDocumentation(IDocumentation::Ptr(this));
    view->setDelegateLinks(true);
    QObject::connect(view, &KDevelop::StandardDocumentationView::linkClicked, ManPageDocumentation::s_provider->model(), &ManPageModel::showItemFromUrl);

    // apply custom style-sheet to normalize look of the page
    const QString cssFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kdevmanpage/manpagedocumentation.css"));
    if (!cssFile.isEmpty()) {
        view->setOverrideCssFile(cssFile);
    }

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

