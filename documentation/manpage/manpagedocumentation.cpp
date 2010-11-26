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

#include <QtDebug>
#include <KIO/TransferJob>
#include <KIO/Job>
#include <kio/jobclasses.h>
#include <documentation/standarddocumentationview.h>

ManPagePlugin* ManPageDocumentation::s_provider=0;


ManPageDocumentation::ManPageDocumentation(ManPage page)
    : m_url(page.second), m_name(page.first)
{
    m_description = getManPageContent();
}

void ManPageDocumentation::readDataFromManPage(KIO::Job * job, const QByteArray &data){
    m_manPageBuffer.append(QString::fromUtf8(data));
}

QString ManPageDocumentation::getManPageContent()
{
    KIO::TransferJob  * transferJob = KIO::get(m_url, KIO::NoReload, KIO::HideProgressInfo);
    connect( transferJob, SIGNAL( data  (  KIO::Job *, const QByteArray &)),
             this, SLOT( readDataFromManPage( KIO::Job *, const QByteArray & ) ) );
    if (transferJob->exec()){
        return m_manPageBuffer;
    } else {
        return i18n("Could not find any documentation for '%1'", m_name);
        qDebug() << "Get man page transferJob error";
    }
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
    view->setHtml(description());
    view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    QObject::connect(view, SIGNAL(linkClicked(QUrl)), ManPageDocumentation::s_provider->model(), SLOT(showItemFromUrl(QUrl)));
    return view;
}

bool ManPageDocumentation::providesWidget() const
{
    return false;
}

ManPageHomeDocumentation::ManPageHomeDocumentation()
{
    ManPageModel* model = ManPageDocumentation::s_provider->model();
    QProgressBar* progressBar = ManPageDocumentation::s_provider->progressBar();

    m_qswidget = new QStackedWidget();
    QObject::connect(model, SIGNAL(manPagesLoaded()), this, SLOT(manPagesLoaded()));

    m_loadingWidget = new QWidget(m_qswidget);
    QLabel* label = new QLabel(i18n("Loading man pages ..."));
    label->setAlignment(Qt::AlignHCenter);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(label);
    layout->addWidget(progressBar);
    layout->addStretch();
    m_loadingWidget->setLayout(layout);
    m_qswidget->addWidget(m_loadingWidget);

    m_contents = new QTreeView(0);
    m_contents->header()->setVisible(false);
    QObject::connect(m_contents, SIGNAL(clicked(QModelIndex)), model, SLOT(showItem(QModelIndex)));
    m_qswidget->addWidget(m_contents);

    m_qswidget->setCurrentWidget(m_loadingWidget);
}

ManPageHomeDocumentation::~ManPageHomeDocumentation()
{
    delete m_contents;
    if(m_loadingWidget){
        delete m_loadingWidget;
    }
}


QWidget* ManPageHomeDocumentation::documentationWidget(KDevelop::DocumentationFindWidget *findWidget, QWidget *parent){

    QProgressBar* progressBar = ManPageDocumentation::s_provider->progressBar();
    ManPageModel* model = ManPageDocumentation::s_provider->model();

    if(progressBar != 0){
        m_qswidget->setCurrentWidget(m_loadingWidget);
    }
    else {
        m_contents->setModel(model);
        m_qswidget->setCurrentWidget(m_contents);
    }
    return m_qswidget;
}

void ManPageHomeDocumentation::manPagesLoaded()
{
    ManPageModel* model = ManPageDocumentation::s_provider->model();
    m_contents->setModel(model);
    m_qswidget->setCurrentWidget(m_contents);
    m_qswidget->removeWidget(m_loadingWidget);
    delete m_loadingWidget;
    m_loadingWidget = 0;
}

QString ManPageHomeDocumentation::name() const
{
    return i18n("Man Content Page");
}

KDevelop::IDocumentationProvider* ManPageHomeDocumentation::provider() const
{
    return ManPageDocumentation::s_provider;
}
