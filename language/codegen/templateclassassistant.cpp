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
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

#include "ui_templateselection.h"

#include <KLocalizedString>
#include <KComponentData>
#include <KNS3/DownloadDialog>
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

class KDevelop::TemplateClassAssistantPrivate
{
public:
    KPageWidgetItem* templateSelectionPage;
    KPageWidgetItem* dummyPage;
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
    
    QModelIndex languageIndex = d->model->index(0, 0);
    QModelIndex templateIndex = d->model->index(0, 0, languageIndex);
    
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
            QStandardItem* item = d->model->itemFromIndex(templateIndex);
            
            while (item->parent() && item->parent() != d->model->invisibleRootItem())
            {
                item = item->parent();
            }
            languageIndex = item->index();
        }
    }
    
    d->ui->languageView->setCurrentIndex(languageIndex);
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

TemplateClassAssistant::TemplateClassAssistant (QWidget* parent, const KUrl& baseUrl)
: CreateClassAssistant (parent, baseUrl)
, d(new TemplateClassAssistantPrivate)
{
    setup();
}

TemplateClassAssistant::~TemplateClassAssistant()
{
    delete d;
}

void TemplateClassAssistant::setup()
{
    setWindowTitle(i18n("Create New Class in %1", baseUrl().prettyUrl()));

    TemplateSelectionPage* page = newTemplateSelectionPage();
    d->templateSelectionPage = addPage(page, i18n("Language and Template"));
    connect (this, SIGNAL(accepted()), page, SLOT(saveConfig()));
    
    /*
     * All assistant pages except the first one require the helper to already be set.
     * However, we can only choose the helper aften the language is selected,
     * so other pages cannot be loaded here yet. 
     * 
     * OTOH, having only one page disables the "next" button and enables the "finish" button.
     * This is not wanted, so we create a dummy page and delete it when "next" is clicked
     */
    QWidget* dummy = new QWidget(this);
    d->dummyPage = addPage(dummy, QLatin1String("Dummy Page"));
    
    setCurrentPage(d->templateSelectionPage);
}

void TemplateClassAssistant::next()
{
    if (currentPage() == d->templateSelectionPage)
    {
        kDebug() << "Current page is template selection";
        QString description = currentPage()->widget()->property("selectedTemplate").toString();
        
        kDebug() << "Chosen template is" << description;
        
        KConfig config(description);
        KConfigGroup group(&config, "General");
        
        kDebug() << "Template name is" << group.readEntry("Name");
        
        QString languageName = group.readEntry("Category").split('/').first();
        
        ILanguage* language = ICore::self()->languageController()->language(languageName);
        
        if (!language)
        {
            kDebug() << "No language named" << languageName;
            return;
        }
        
        d->helper = language->languageSupport()->createClassHelper(this);
        
        if (!d->helper)
        {
            kDebug() << "No class creation helper for language" << languageName;
            return;
        }
        
        ClassGenerator* generator = d->helper->generator();
        if (!generator)
        {
            kDebug() << "No generator for language" << languageName;
            return;
        }
        
        if (TemplateClassGenerator* templateGenerator = dynamic_cast<TemplateClassGenerator*>(generator))
        {
            kDebug() << "Class generator uses templates";
            templateGenerator->setTemplateDescription(description);
        }
        setGenerator(generator);
                
        removePage(d->dummyPage);
        KDevelop::CreateClassAssistant::setup();
        
        return;
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

