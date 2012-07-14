/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "templateselectionpage.h"
#include "ui_templateselectionpage.h"

#include <language/codegen/templatesmodel.h>
#include <interfaces/icore.h>

#include <KComponentData>

TemplateSelectionPage::TemplateSelectionPage (QWidget* parent)
: QWidget(parent)
{
    ui = new Ui::TemplateSelectionPage;
    ui->setupUi(this);

    m_model = new KDevelop::TemplatesModel(KDevelop::ICore::self()->componentData(), this);
    m_model->setDescriptionResourceType("filetemplate_descriptions");
    m_model->setTemplateResourceType("filetemplates");
    m_model->refresh();

    ui->treeView->setModel(m_model);

    QModelIndex categoryIndex;
    QList<QStandardItem*> tests = m_model->findItems("Test");
    if (!tests.isEmpty())
    {
        categoryIndex = tests.first()->index();
    }
    ui->treeView->setRootIndex(categoryIndex);

    connect (ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(contentsChanged()));
    connect (ui->identifierLineEdit, SIGNAL(textChanged(QString)), SLOT(contentsChanged()));
    connect (ui->directoryUrlRequester, SIGNAL(textChanged(QString)), SLOT(contentsChanged()));
}

TemplateSelectionPage::~TemplateSelectionPage()
{
    delete ui;
}

QString TemplateSelectionPage::selectedTemplate() const
{
    QStandardItem* item = m_model->itemFromIndex(ui->treeView->selectionModel()->currentIndex());
    return item->data(KDevelop::TemplatesModel::DescriptionFileRole).toString();
}

void TemplateSelectionPage::contentsChanged()
{
    QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    QStandardItem* item = m_model->itemFromIndex(index);
    bool valid = item && !item->hasChildren() && !item->data(KDevelop::TemplatesModel::DescriptionFileRole).toString().isEmpty();
    valid = valid && !ui->identifierLineEdit->text().isEmpty() && ui->directoryUrlRequester->url().isValid();
    emit templateValid(valid);
}

QString TemplateSelectionPage::identifier() const
{
    return ui->identifierLineEdit->text();
}

KUrl TemplateSelectionPage::baseUrl() const
{
    return ui->directoryUrlRequester->url();
}

void TemplateSelectionPage::setBaseUrl (const KUrl& url)
{
    ui->directoryUrlRequester->setUrl(url);
}

