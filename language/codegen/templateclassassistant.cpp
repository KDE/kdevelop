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
#include "interfaces/icore.h"

#include "ui_templateselection.h"

#include <KLocalizedString>
#include <KComponentData>

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
    
    d->ui->languageView->setModel(d->model);
    d->ui->templateView->setModel(d->model);
    
    connect (d->ui->languageView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), 
        d->ui->templateView, SLOT(setRootIndex(QModelIndex)));
    
    connect (d->ui->templateView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), 
        this, SLOT(currentTemplateChanged(QModelIndex)));
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

void TemplateSelectionPage::currentTemplateChanged (QModelIndex index)
{
    if (d->model->index(0, 0, index).isValid())
    {
        // This index has a child, so it is not a template
        d->assistant->setValid(d->assistant->currentPage(), false);
    }
    else
    {
        d->selectedTemplate = d->model->data(index, TemplatesModel::TemplateFileRole).toString();
        d->assistant->setValid(d->assistant->currentPage(), true);
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

TemplateSelectionPage* TemplateClassAssistant::newTemplateSelectionPage()
{
    return new TemplateSelectionPage(this);
}
