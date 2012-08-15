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
#include <interfaces/isession.h>

#include "ui_templateselection.h"
#include "ui_classmembers.h"

#include <KNS3/DownloadDialog>
#include <KNS3/KNewStuffButton>
#include <KLocalizedString>
#include <KComponentData>
#include <KFileDialog>

using namespace KDevelop;

const char* LastUsedTemplateEntry = "LastUsedTemplate";
const char* FileTemplatesGroup = "SourceFileTemplates";

class KDevelop::TemplateSelectionPagePrivate
{
public:
    TemplateSelectionPagePrivate(TemplateSelectionPage* page_)
    : page(page_)
    {}

    TemplateSelectionPage* page;
    Ui::TemplateSelection* ui;
    QString selectedTemplate;
    TemplateClassAssistant* assistant;
    TemplatesModel* model;

    void currentTemplateChanged(const QModelIndex& index);
    void currentLanguageChanged(const QModelIndex& index);
    void getMoreClicked();
    void loadFileClicked();
};

void TemplateSelectionPagePrivate::currentLanguageChanged(const QModelIndex& index)
{
    ui->templateView->setRootIndex(index);
    ui->templateView->expandAll();

    QModelIndex templateIndex = index;
    while (templateIndex.child(0, 0).isValid())
    {
        templateIndex = templateIndex.child(0, 0);
    }
    ui->templateView->setCurrentIndex(templateIndex);
}

void TemplateSelectionPagePrivate::currentTemplateChanged(const QModelIndex& index)
{
    if (!index.isValid() || model->index(0, 0, index).isValid())
    {
        // This index is invalid or has a child, so it is not a template
        assistant->setValid(assistant->currentPage(), false);
    }
    else
    {
        selectedTemplate = model->data(index, TemplatesModel::DescriptionFileRole).toString();
        assistant->setValid(assistant->currentPage(), true);
    }
}

void TemplateSelectionPagePrivate::getMoreClicked()
{
    model->refresh();
}

void TemplateSelectionPagePrivate::loadFileClicked()
{
    QString filter = "application/x-desktop application/x-bzip-compressed-tar application/zip";
    QString fileName = KFileDialog::getOpenFileName(KUrl("kfiledialog:///kdevclasstemplate"), filter, page);

    if (!fileName.isEmpty())
    {
        QString destination = model->loadTemplateFile(fileName);
        QModelIndexList indexes = model->templateIndexes(destination);
        int n = indexes.size();
        if (n > 1)
        {
            ui->languageView->setCurrentIndex(indexes[1]);
            ui->templateView->setCurrentIndex(indexes.last());
        }
    }
}

void TemplateSelectionPage::saveConfig()
{
    KSharedConfig::Ptr config;
    if (IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl()))
    {
        config = project->projectConfiguration();
    }
    else
    {
        config = ICore::self()->activeSession()->config();
    }

    KConfigGroup group(config, FileTemplatesGroup);
    group.writeEntry(LastUsedTemplateEntry, d->selectedTemplate);
    group.sync();
}

TemplateSelectionPage::TemplateSelectionPage(TemplateClassAssistant* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, d(new TemplateSelectionPagePrivate(this))
{
    d->assistant = parent;

    d->ui = new Ui::TemplateSelection;
    d->ui->setupUi(this);

    d->model = new TemplatesModel(ICore::self()->componentData());
    d->model->setTemplateResourceType("filetemplates");
    d->model->setDescriptionResourceType("filetemplate_descriptions");
    d->model->refresh();

    d->ui->languageView->setLevels(2);
    d->ui->languageView->setHeaderLabels(QStringList() << i18n("Category") << i18n("Language") << i18n("Template"));
    d->ui->languageView->setModel(d->model);

    d->ui->templateView->setModel(d->model);

    connect (d->ui->languageView, SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)),
             SLOT(currentLanguageChanged(QModelIndex)));

    connect (d->ui->templateView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             SLOT(currentTemplateChanged(QModelIndex)));

    QModelIndex categoryIndex = d->model->index(0, 0);
    QModelIndex templateIndex = categoryIndex;

    while (templateIndex.child(0, 0).isValid())
    {
        templateIndex = templateIndex.child(0, 0);
    }

    KSharedConfig::Ptr config;
    if (IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl()))
    {
        config = project->projectConfiguration();
    }
    else
    {
        config = ICore::self()->activeSession()->config();
    }

    KConfigGroup group(config, FileTemplatesGroup);
    QString lastTemplate = group.readEntry(LastUsedTemplateEntry);

    QModelIndexList indexes = d->model->match(d->model->index(0, 0), TemplatesModel::DescriptionFileRole, lastTemplate, 1, Qt::MatchRecursive);

    if (!indexes.isEmpty())
    {
        templateIndex = indexes.first();
    }

    categoryIndex = templateIndex;
    while (categoryIndex.parent().isValid() && categoryIndex.parent().parent().isValid())
    {
        categoryIndex = categoryIndex.parent();
    }

    d->ui->languageView->setCurrentIndex(categoryIndex);
    d->ui->templateView->setCurrentIndex(templateIndex);

    KNS3::Button* getMoreButton = new KNS3::Button(i18n("Get More Templates..."), "kdevclassassistant.knsrc", d->ui->languageView);
    connect (getMoreButton, SIGNAL(dialogFinished(KNS3::Entry::List)), SLOT(getMoreClicked()));
    d->ui->languageView->addWidget(0, getMoreButton);

    KPushButton* loadButton = new KPushButton(KIcon("application-x-archive"), i18n("Load Template From File"), d->ui->languageView);
    connect (loadButton, SIGNAL(clicked(bool)), SLOT(loadFileClicked()));
    d->ui->languageView->addWidget(0, loadButton);

    d->ui->languageView->setContentsMargins(0, 0, 0, 0);
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

#include "templateselectionpage.moc"
