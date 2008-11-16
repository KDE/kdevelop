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

#include <klineedit.h>

#include "ui_projectselectionpage.h"
#include "projecttemplatesmodel.h"

ProjectSelectionPage::ProjectSelectionPage(ProjectTemplatesModel *templatesModel, QWidget *parent)
    :QWidget(parent), m_templatesModel(templatesModel), m_urlEditedByUser( false )
{
    ui = new Ui::ProjectSelectionPage();
    ui->setupUi(this);
    ui->templateView->setModel(templatesModel);

    ui->locationUrl->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    connect( ui->locationUrl->lineEdit(), SIGNAL(textEdited(const QString&)),
             this, SLOT(urlEdited() ));
    connect( ui->locationUrl, SIGNAL(urlSelected(const KUrl&)),
             this, SLOT(urlEdited() ));
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
    KUrl tUrl = ui->locationUrl->url();
    tUrl.addPath( encodedAppName() );
    return tUrl;
}

QString ProjectSelectionPage::appName()
{
    return ui->appNameEdit->text();
}

void ProjectSelectionPage::urlEdited()
{
    m_urlEditedByUser = true;
    validateData();
    emit locationChanged( ui->locationUrl->url() );
}

void ProjectSelectionPage::validateData()
{
    KUrl url;
    if( m_urlEditedByUser )
    {
        url = ui->locationUrl->url();
    } else
    {
        url = KUrl( QDir::homePath() );
    }
    if( !url.isLocalFile() || url.isEmpty())
    {
        ui->locationValidLabel->setText( i18n("Invalid Location") );
        emit invalid();
        return;
    }

    if( appName().isEmpty() )
    {
        ui->locationValidLabel->setText( i18n("Empty project name") );
        emit invalid();
        return;
    }

    if( appName() == "." || appName() == "..")
    {
        ui->locationValidLabel->setText( i18n("Invalid project name") );
        emit invalid();
        return;
    }

    QDir tDir(url.toLocalFile( KUrl::RemoveTrailingSlash ));
    while (!tDir.exists() && !tDir.isRoot())
        tDir.setPath( pathUp( tDir.absolutePath() ));

    if (tDir.exists())
    {
        QFileInfo tFileInfo(tDir.absolutePath());
        if (!tFileInfo.isWritable() || !tFileInfo.isExecutable())
        {
            ui->locationValidLabel->setText( i18n("Unabled to create subdirectories, "
                                                  "missing permissions on: %1", tDir.absolutePath()) );
            emit invalid();
            return;
        }
    }

    if( !m_urlEditedByUser )
    {
        url.addPath( encodedAppName() );
    }
    QFileInfo fi( url.toLocalFile( KUrl::RemoveTrailingSlash ) );
    if( fi.exists() && fi.isDir() )
    {
        if( !QDir( fi.absoluteFilePath()).entryList( QDir::NoDotAndDotDot | QDir::AllEntries ).isEmpty() )
        {
            ui->locationValidLabel->setText( i18n("Directory already exists and is not empty!") );
            emit invalid();
            return;
        }
    }

    QStandardItem* item = m_templatesModel->itemFromIndex( ui->templateView->currentIndex() );
    if( item && !item->hasChildren() )
    {
        ui->locationValidLabel->setText( url.path( KUrl::RemoveTrailingSlash ) );
        if( !m_urlEditedByUser )
        {
            ui->locationUrl->setPath( url.path( KUrl::RemoveTrailingSlash ) );
        }
        emit valid();
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

QByteArray ProjectSelectionPage::encodedAppName()
{
    // : < > * ? / \ | " are invalid on windows
    QByteArray tEncodedName = appName().toUtf8();
    for (int i = 0; i < tEncodedName.size(); ++i)
    {
        QChar tChar(tEncodedName.at( i ));
        if (tChar.isDigit() || tChar.isSpace() || tChar.isLetter() || tChar == '%')
            continue;

        QByteArray tReplace = QUrl::toPercentEncoding( tChar );
        tEncodedName.replace( tEncodedName.at( i ) ,tReplace );
        i =  i + tReplace.size() - 1;
    }
    return tEncodedName;
}

QString ProjectSelectionPage::pathUp(const QString& aPath)
{
    QString tPath = aPath;
    int tIndex = tPath.lastIndexOf( QDir::separator() );
    tPath = tPath.remove(tIndex, tPath.length() - tIndex);
    return tPath;
}

#include "projectselectionpage.moc"
