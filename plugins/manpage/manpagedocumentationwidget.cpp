/*
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>
    SPDX-FileCopyrightText: 2010 Yannick Motta <yannick.motta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "manpagedocumentationwidget.h"
#include "manpagedocumentation.h"
#include "manpageplugin.h"
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <QEvent>
#include <QHeaderView>
#include <QTreeView>

ManPageDocumentationWidget::ManPageDocumentationWidget(QWidget *parent)
    : QStackedWidget(parent)
    , m_loadingWidget(nullptr)
{
    ManPageModel* model = ManPageDocumentation::s_provider->model();
    m_treeView = new QTreeView(this);
    m_treeView->viewport()->installEventFilter(this);
    m_treeView->header()->setVisible(false);
    connect(m_treeView, &QTreeView::clicked, model, &ManPageModel::showItem);
    addWidget(m_treeView);
    if(!model->isLoaded()){
        m_loadingWidget = new QWidget(this);
        m_progressBar = new QProgressBar(m_loadingWidget);
        m_statusLabel = new QLabel(i18n("Loading man pages..."));
        if(model->sectionCount() == 0){
            connect(model, &ManPageModel::sectionListUpdated, this, &ManPageDocumentationWidget::sectionListUpdated );
        } else {
            sectionListUpdated();
        }
        connect(model, &ManPageModel::sectionParsed, this, &ManPageDocumentationWidget::sectionParsed );
        connect(model, &ManPageModel::manPagesLoaded, this, &ManPageDocumentationWidget::manIndexLoaded);
        connect(model, &ManPageModel::error, this, &ManPageDocumentationWidget::handleError);
        m_statusLabel->setAlignment(Qt::AlignHCenter);
        auto* layout = new QVBoxLayout();
        layout->addWidget(m_statusLabel);
        layout->addWidget(m_progressBar);
        layout->addStretch();
        m_loadingWidget->setLayout(layout);
        addWidget(m_loadingWidget);
        setCurrentWidget(m_loadingWidget);
        if(model->hasError())
            handleError(model->errorString());
    } else {
        manIndexLoaded();
    }

}

void ManPageDocumentationWidget::manIndexLoaded()
{
    ManPageModel* model = ManPageDocumentation::s_provider->model();
    m_treeView->setModel(model);
    setCurrentWidget(m_treeView);
    if(m_loadingWidget){
        removeWidget(m_loadingWidget);
        delete m_loadingWidget;
        m_loadingWidget = nullptr;
    }
}

void ManPageDocumentationWidget::sectionListUpdated()
{
    ManPageModel* model = ManPageDocumentation::s_provider->model();
    m_progressBar->setRange(0, model->sectionCount());
}

void ManPageDocumentationWidget::sectionParsed()
{
    ManPageModel* model = ManPageDocumentation::s_provider->model();
    m_progressBar->setValue(model->nbSectionLoaded());
}

void ManPageDocumentationWidget::handleError(const QString& errorString)
{
    delete m_progressBar;
    m_progressBar = nullptr;
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setText(i18n("Error while loading man pages:\n%1", errorString));
}

bool ManPageDocumentationWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress)
        event->ignore(); // Propagate to DocumentationView
    return QStackedWidget::eventFilter(watched, event);
}

#include "moc_manpagedocumentationwidget.cpp"
