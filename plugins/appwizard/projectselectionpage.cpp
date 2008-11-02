/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "projectselectionpage.h"

#include <QDir>

#include "ui_projectselectionpage.h"
#include "projecttemplatesmodel.h"

ProjectSelectionPage::ProjectSelectionPage(ProjectTemplatesModel *templatesModel, QWidget *parent)
    :QWidget(parent), m_templatesModel(templatesModel)
{
    ui = new Ui::ProjectSelectionPage();
    ui->setupUi(this);
    ui->templateView->setModel(templatesModel);

    ui->locationUrl->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    connect( ui->locationUrl, SIGNAL(textChanged(const QString&)),
             this, SLOT(validateData() ));
    connect( ui->locationUrl, SIGNAL(urlSelected(const KUrl&)),
             this, SLOT(validateData() ));
    connect( ui->templateView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), 
             this, SLOT( validateData() ) );
    connect( ui->appNameEdit, SIGNAL(textEdited(const QString&)),
             this, SLOT( validateData() ) );
    ui->locationUrl->setPath(QDir::homePath());
}

ProjectSelectionPage::~ProjectSelectionPage()
{
    delete ui;
}

QString ProjectSelectionPage::selectedTemplate()
{
    QStandardItem *item = m_templatesModel->itemFromIndex(ui->templateView->currentIndex());
    if (item)
        return item->data().toString();
    else
        return "";
}

KUrl ProjectSelectionPage::location()
{
    return ui->locationUrl->url();
}

QString ProjectSelectionPage::appName()
{
    return ui->appNameEdit->text();
}

void ProjectSelectionPage::validateData()
{
    KUrl url = ui->locationUrl->url();
    if( !url.isLocalFile() )
    {
        ui->locationValidLabel->setText( i18n("Invalid Location") );
        emit invalid();
        return;
    }
    QFileInfo fi( url.toLocalFile( KUrl::RemoveTrailingSlash ) );
    if( fi.exists() && fi.isDir() )
    {
           if( !QDir( fi.absoluteFilePath()).entryList().isEmpty() )
           {
                ui->locationValidLabel->setText( i18n("Directory already exists and is not empty!") );
                emit invalid();
                return;
           }
    }

    if( ui->appNameEdit->text().isEmpty() )
    {
        ui->locationValidLabel->setText( i18n("Empty project name") );
        emit invalid();
        return;
    }

    QStandardItem* item = m_templatesModel->itemFromIndex( ui->templateView->currentIndex() );
    if( item && !item->hasChildren() )
    {
        ui->locationValidLabel->setText( i18n("Valid Location") );
        emit valid();
        emit locationChanged( url );
        return;
    } else 
    {
        ui->locationValidLabel->setText( i18n("Invalid Project Template") );
        emit invalid();
        return;
    }
    ui->locationValidLabel->setText( i18n("Invalid Location") );
    emit invalid();
}

#include "projectselectionpage.moc"
