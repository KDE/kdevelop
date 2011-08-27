/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *   Copyright 2011 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
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
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include "ui_projectselectionpage.h"
#include "projecttemplatesmodel.h"
#include "appwizardplugin.h"
#include <KColorScheme>

ProjectSelectionPage::ProjectSelectionPage(ProjectTemplatesModel *templatesModel, QWidget *parent)
    : AppWizardPageWidget(parent), m_templatesModel(templatesModel)
{
    ui = new Ui::ProjectSelectionPage();
    ui->setupUi(this);
    setContentsMargins(0,0,0,0);
    ui->descriptionContent->setBackgroundRole(QPalette::Base);
    ui->descriptionContent->setForegroundRole(QPalette::Text);
    ui->templateView->setModel(templatesModel);
    ui->templateView->setFocus();

    ui->locationUrl->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    ui->locationUrl->setUrl(KDevelop::ICore::self()->projectController()->projectsBaseDirectory());

    ui->locationValidLabel->setText(QString(" "));
    
    connect( ui->locationUrl->lineEdit(), SIGNAL(textEdited(QString)),
             this, SLOT(urlEdited()));
    connect( ui->locationUrl, SIGNAL(urlSelected(KUrl)),
             this, SLOT(urlEdited()));
    connect( ui->appNameEdit, SIGNAL(textEdited(QString)),
             this, SLOT(nameChanged()) );
    
    
    connect( ui->templateView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(templateFamilyChanged(QModelIndex,QModelIndex)) );
    connect( ui->templateType, SIGNAL(currentIndexChanged(int)),
             this, SLOT(templateChanged(int)) );
}


void ProjectSelectionPage::nameChanged()
{
    validateData();
    emit locationChanged( location() );
}


ProjectSelectionPage::~ProjectSelectionPage()
{
    delete ui;
}

void ProjectSelectionPage::templateFamilyChanged(const QModelIndex& current, const QModelIndex& )
{
    ui->templatesIconView->setModel(m_templatesModel);
    ui->templatesIconView->setRootIndex(current);
    
    ui->templatesIconView->setCurrentIndex(m_templatesModel->index(0,0, current));
    
    connect( ui->templatesIconView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(typeChanged(QModelIndex)) );
}

void ProjectSelectionPage::typeChanged(const QModelIndex& idx)
{
    bool hasChildren = idx.model()->rowCount(idx)>0;
    if(hasChildren) {
        ui->templateType->setModel(m_templatesModel);
        ui->templateType->setRootModelIndex(idx);
        ui->templateType->setCurrentIndex(0);
    } else
        itemChanged(idx);
    
    ui->templateType->setVisible(hasChildren);
}

void ProjectSelectionPage::templateChanged(int current)
{
    QModelIndex idx=m_templatesModel->index(current, 0, ui->templateType->rootModelIndex());
    itemChanged(idx);
}

void ProjectSelectionPage::itemChanged( const QModelIndex& current)
{
    qDebug() << "--------" << current << current.data() << current.data( Qt::UserRole+2 );
    
    KStandardDirs* dirs = m_templatesModel->plugin()->componentData().dirs();
    QString picPath = dirs->findResource("apptemplate_previews", current.data( Qt::UserRole+2 ).toString() );
    if( picPath.isEmpty() ) 
    {
        picPath = dirs->findResource("apptemplate_previews", "default-kdevelop.png");
    }
    ui->preview->setPixmap( QPixmap( picPath ) );
    ui->description->setText( current.data( Qt::UserRole+4 ).toString()+"\n"+current.data( Qt::UserRole+3 ).toString() );
    validateData();
    
    ui->propertiesBox->setEnabled(true);
}

QString ProjectSelectionPage::selectedTemplate()
{
    QStandardItem *item = getCurrentItem();
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
    validateData();
    emit locationChanged( location() );
}

void setForeground(QLabel* label, KColorScheme::ForegroundRole role)
{
    QPalette p = label->palette();
    KColorScheme::adjustForeground(p, role, label->foregroundRole(), KColorScheme::Window);
    label->setPalette(p);
}

void ProjectSelectionPage::validateData()
{
    KUrl url = ui->locationUrl->url();
    if( !url.isLocalFile() || url.isEmpty() )
    {
        ui->locationValidLabel->setText( i18n("Invalid location") );
        setForeground(ui->locationValidLabel, KColorScheme::NegativeText);
        emit invalid();
        return;
    }

    if( appName().isEmpty() )
    {
        ui->locationValidLabel->setText( i18n("Empty project name") );
        setForeground(ui->locationValidLabel, KColorScheme::NegativeText);
        emit invalid();
        return;
    }

    if( appName() == "." || appName() == "..")
    {
        ui->locationValidLabel->setText( i18n("Invalid project name") );
        setForeground(ui->locationValidLabel, KColorScheme::NegativeText);
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
            ui->locationValidLabel->setText( i18n("Unable to create subdirectories, "
                                                  "missing permissions on: %1", tDir.absolutePath()) );
            setForeground(ui->locationValidLabel, KColorScheme::NegativeText);
            emit invalid();
            return;
        }
    }

    QStandardItem* item = getCurrentItem();
    if( item && !item->hasChildren() )
    {
        ui->locationValidLabel->setText( QString(" ") );
        setForeground(ui->locationValidLabel, KColorScheme::NormalText);
        emit valid();
    } else
    {
        ui->locationValidLabel->setText( i18n("Invalid project template, please choose a leaf item") );
        setForeground(ui->locationValidLabel, KColorScheme::NegativeText);
        emit invalid();
        return;
    }

    // Check for non-empty target directory. Not an error, but need to display a warning.
    url.addPath( encodedAppName() );
    QFileInfo fi( url.toLocalFile( KUrl::RemoveTrailingSlash ) );
    if( fi.exists() && fi.isDir() )
    {
        if( !QDir( fi.absoluteFilePath()).entryList( QDir::NoDotAndDotDot | QDir::AllEntries ).isEmpty() )
        {
            ui->locationValidLabel->setText( i18n("Path already exists and contains files") );
            setForeground(ui->locationValidLabel, KColorScheme::NegativeText);
        }
    }
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

QStandardItem* ProjectSelectionPage::getCurrentItem() const
{
    QStandardItem* item = m_templatesModel->itemFromIndex( ui->templateView->currentIndex() );
    if ( item && item->hasChildren() )
        item = m_templatesModel->itemFromIndex( ui->templatesIconView->currentIndex() );
    if ( item && item->hasChildren() )
    {
        const int currect = ui->templateType->currentIndex();
        const QModelIndex idx = m_templatesModel->index( currect, 0, ui->templateType->rootModelIndex() );
        item = m_templatesModel->itemFromIndex(idx);
    }
    return item;
}


bool ProjectSelectionPage::shouldContinue()
{
    QFileInfo fi(location().toLocalFile(KUrl::RemoveTrailingSlash));
    if (fi.exists() && fi.isDir())
    {
        if (!QDir(fi.absoluteFilePath()).entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty())
        {
            int res = KMessageBox::questionYesNo(this, i18n("The specified path already exists and contains files. "
                                                            "Are you sure you want to proceed?"));
            return res == KMessageBox::Yes;
        }
    }
    return true;
}

#include "projectselectionpage.moc"
