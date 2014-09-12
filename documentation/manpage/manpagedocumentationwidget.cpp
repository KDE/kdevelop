/*  This file is part of KDevelop
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>
    Copyright 2010 Yannick Motta <yannick.motta@gmail.com>

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

#include "manpagedocumentationwidget.h"
#include "manpagedocumentation.h"
#include "manpageplugin.h"
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <QHeaderView>
#include <QTreeView>

ManPageDocumentationWidget::ManPageDocumentationWidget(QWidget *parent)
    : QStackedWidget(parent)
    , m_loadingWidget(0)
{
    ManPageModel* model = ManPageDocumentation::s_provider->model();
    m_treeView = new QTreeView(this);
    m_treeView->header()->setVisible(false);
    connect(m_treeView, SIGNAL(clicked(QModelIndex)), model, SLOT(showItem(QModelIndex)));
    addWidget(m_treeView);
    if(!model->isLoaded()){
        m_loadingWidget = new QWidget(this);
        m_progressBar = new QProgressBar(m_loadingWidget);
        QLabel* label = new QLabel(i18n("Loading man pages ..."));
        if(model->sectionCount() == 0){
            connect(model, SIGNAL(sectionListUpdated()), this, SLOT(sectionListUpdated()) );
        } else {
            sectionListUpdated();
        }
        connect(model, SIGNAL(sectionParsed()), this, SLOT(sectionParsed()) );
        connect(model, SIGNAL(manPagesLoaded()), this, SLOT(manIndexLoaded()));
        label->setAlignment(Qt::AlignHCenter);
        QVBoxLayout* layout = new QVBoxLayout();
        layout->addWidget(label);
        layout->addWidget(m_progressBar);
        layout->addStretch();
        m_loadingWidget->setLayout(layout);
        addWidget(m_loadingWidget);
        setCurrentWidget(m_loadingWidget);
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
        m_loadingWidget = 0;
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
