/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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

#include "templateselectionpage.h"
#include "templateclassassistant.h"
#include "templatesmodel.h"

#include <interfaces/icreateclasshelper.h>
#include <interfaces/ilanguagesupport.h>
#include "interfaces/icore.h"
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguage.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

#include "ui_templateselection.h"
#include "ui_classmembers.h"

#include <KNS3/DownloadDialog>
#include <KLocalizedString>
#include <KComponentData>
#include <KFileDialog>

using namespace KDevelop;

const char* LastUsedTemplateEntry = "LastUsedTemplate";
const char* ClassTemplatesGroup = "ClassTemplates";

class KDevelop::TemplateSelectionPagePrivate
{
public:
    Ui::TemplateSelection* ui;
    QString selectedTemplate;
    TemplateClassAssistant* assistant;
    TemplatesModel* model;
};

TemplateSelectionPage::TemplateSelectionPage (TemplateClassAssistant* parent, Qt::WindowFlags f)
: QWidget (parent, f)
, d(new TemplateSelectionPagePrivate)
{
    d->assistant = parent;

    d->ui = new Ui::TemplateSelection;
    d->ui->setupUi(this);

    d->model = new TemplatesModel(ICore::self()->componentData());
    d->model->setTemplateResourceType("filetemplates");
    d->model->setDescriptionResourceType("filetemplate_descriptions");
    d->model->refresh();

    d->ui->languageView->setModel(d->model);
    d->ui->templateView->setModel(d->model);
    
    connect (d->ui->languageView, SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)),
             SLOT(currentLanguageChanged(QModelIndex)));
    
    connect (d->ui->templateView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             SLOT(currentTemplateChanged(QModelIndex)));

    connect (d->ui->getMoreButton, SIGNAL(clicked(bool)), this, SLOT(getMoreClicked()));
    connect (d->ui->loadFileButton, SIGNAL(clicked(bool)), this, SLOT(loadFileClicked()));

    QModelIndex categoryIndex = d->model->index(0, 0);
    QModelIndex templateIndex = categoryIndex;

    while (templateIndex.child(0, 0).isValid())
    {
        templateIndex = templateIndex.child(0, 0);
    }

    IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl());
    if (project)
    {
        KConfigGroup group(project->projectConfiguration(), ClassTemplatesGroup);
        QString lastTemplate = group.readEntry(LastUsedTemplateEntry);

        QModelIndexList indexes = d->model->match(d->model->index(0, 0), TemplatesModel::DescriptionFileRole, lastTemplate);

        if (!indexes.isEmpty())
        {
            templateIndex = indexes.first();
        }
    }

    categoryIndex = templateIndex;
    while (categoryIndex.parent().isValid() && categoryIndex.parent().parent().isValid())
    {
        categoryIndex = categoryIndex.parent();
    }

    d->ui->languageView->setCurrentIndex(categoryIndex);
    d->ui->templateView->setCurrentIndex(templateIndex);
}

TemplateSelectionPage::~TemplateSelectionPage()
{
    delete d->ui;
    delete d;
}

QString TemplateSelectionPage::selectedTemplate() const
{
    return d->selectedTemplate;
}

void TemplateSelectionPage::currentLanguageChanged (const QModelIndex& index)
{
    d->ui->templateView->setRootIndex(index);
    d->ui->templateView->expandAll();

    QModelIndex templateIndex = index;
    while (templateIndex.child(0, 0).isValid())
    {
        templateIndex = templateIndex.child(0, 0);
    }
    d->ui->templateView->setCurrentIndex(templateIndex);
    d->model->setHorizontalHeaderLabels(QStringList(d->model->data(index).toString()));
}

void TemplateSelectionPage::currentTemplateChanged (const QModelIndex& index)
{
    if (!index.isValid() || d->model->index(0, 0, index).isValid())
    {
        // This index is invalid or has a child, so it is not a template
        d->assistant->setValid(d->assistant->currentPage(), false);
    }
    else
    {
        d->selectedTemplate = d->model->data(index, TemplatesModel::DescriptionFileRole).toString();
        d->assistant->setValid(d->assistant->currentPage(), true);
    }
}

void TemplateSelectionPage::getMoreClicked()
{
    KNS3::DownloadDialog dialog("kdevclassassistant.knsrc");
    dialog.exec();
    d->model->refresh();
}

void TemplateSelectionPage::loadFileClicked()
{
    QString filter = "application/x-desktop application/x-bzip-compressed-tar application/zip";
    QString fileName = KFileDialog::getOpenFileName(KUrl("kfiledialog:///kdevclasstemplate"), filter, this);

    if (!fileName.isEmpty())
    {
        QString destination = d->model->loadTemplateFile(fileName);
        QModelIndexList indexes = d->model->templateIndexes(destination);
        int n = indexes.size();
        if (n > 1)
        {
            d->ui->languageView->setCurrentIndex(indexes[1]);
            d->ui->templateView->setCurrentIndex(indexes.last());
        }
    }
}

void TemplateSelectionPage::saveConfig()
{
    IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl());
    if (project)
    {
        KConfigGroup group(project->projectConfiguration(), ClassTemplatesGroup);
        group.writeEntry(LastUsedTemplateEntry, selectedTemplate());
        group.sync();
    }
}
