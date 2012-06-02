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

#include "templateclassassistant.h"
#include "templatesmodel.h"
#include "templateclassgenerator.h"
#include <interfaces/icreateclasshelper.h>
#include <interfaces/ilanguagesupport.h>
#include "interfaces/icore.h"
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguage.h>

#include "ui_templateselection.h"

#include <KLocalizedString>
#include <KComponentData>
#include <KNS3/DownloadDialog>
#include <KFileDialog>

using namespace KDevelop;

class KDevelop::TemplateSelectionPagePrivate
{
public:
    Ui::TemplateSelection* ui;
    QString selectedTemplate;
    TemplateClassAssistant* assistant;
    TemplatesModel* model;
};

class KDevelop::TemplateClassAssistantPrivate
{
public:
    KPageWidgetItem* templateSelectionPage;
    ICreateClassHelper* helper;
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
    
    connect (d->ui->languageView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), 
        this, SLOT(currentLanguageChanged(QModelIndex)));
    
    connect (d->ui->templateView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), 
        this, SLOT(currentTemplateChanged(QModelIndex)));
    
    connect (d->ui->getMoreButton, SIGNAL(clicked(bool)), this, SLOT(getMoreClicked()));
    connect (d->ui->loadFileButton, SIGNAL(clicked(bool)), this, SLOT(loadFileClicked()));
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
    d->ui->templateView->setCurrentIndex(d->model->index(0, 0, index));
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
    // TODO: Install .knsrc file
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
        if (indexes.size() > 1)
        {
            d->ui->languageView->setCurrentIndex(indexes.first());
            d->ui->templateView->setCurrentIndex(indexes.last());
        }
    }
}

TemplateClassAssistant::TemplateClassAssistant (QWidget* parent, const KUrl& baseUrl)
: CreateClassAssistant (parent, baseUrl)
, d(new TemplateClassAssistantPrivate)
{
    
}

TemplateClassAssistant::~TemplateClassAssistant()
{
    delete d;
}

void TemplateClassAssistant::setup()
{
    TemplateSelectionPage* page = newTemplateSelectionPage();
    d->templateSelectionPage = addPage(page, i18n("Language and Template"));
    CreateClassAssistant::setup();
    
    setCurrentPage(d->templateSelectionPage);
}

void TemplateClassAssistant::next()
{
    if (currentPage() == d->templateSelectionPage)
    {
        QString description = currentPage()->widget()->property("currentTemplate").toString();
        
        KConfig config(description);
        KConfigGroup group(&config, "General");
        QString languageName = group.readEntry("Category").split('/').first();
        
        ILanguage* language = ICore::self()->languageController()->language(languageName);
        
        if (!language)
        {
            return;
        }
        
        d->helper = language->languageSupport()->createClassHelper(this);
        
        if (!d->helper)
        {
            return;
        }
        
        ClassGenerator* generator = d->helper->generator();
        if (TemplateClassGenerator* templateGenerator = dynamic_cast<TemplateClassGenerator*>(generator))
        {
            templateGenerator->setTemplateDescription(description);
        }
        
        setGenerator(generator);
    }
    KDevelop::CreateClassAssistant::next();
}


TemplateSelectionPage* TemplateClassAssistant::newTemplateSelectionPage()
{
    return new TemplateSelectionPage(this);
}

ClassIdentifierPage* TemplateClassAssistant::newIdentifierPage()
{
    return d->helper->identifierPage();
}

OverridesPage* TemplateClassAssistant::newOverridesPage()
{
    return d->helper->overridesPage();
}

