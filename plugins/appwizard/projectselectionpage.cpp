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
#include <kmessagebox.h>
#include <kdebug.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include <util/multilevellistview.h>

#include "ui_projectselectionpage.h"
#include "projecttemplatesmodel.h"
#include <KColorScheme>
#include <KFileDialog>
#include <KNS3/KNewStuffButton>
#include <KTar>
#include <KZip>
#include <KI18n/KLocalizedString>
#include <KDELibs4Support/kpushbutton.h>

ProjectSelectionPage::ProjectSelectionPage(ProjectTemplatesModel *templatesModel, AppWizardDialog *wizardDialog)
    : AppWizardPageWidget(wizardDialog), m_templatesModel(templatesModel)
{
    ui = new Ui::ProjectSelectionPage();
    ui->setupUi(this);
    setContentsMargins(0,0,0,0);
    ui->descriptionContent->setBackgroundRole(QPalette::Base);
    ui->descriptionContent->setForegroundRole(QPalette::Text);

    ui->locationUrl->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    ui->locationUrl->setUrl(KDevelop::ICore::self()->projectController()->projectsBaseDirectory());

    ui->locationValidLabel->setText(QString(" "));

    connect( ui->locationUrl->lineEdit(), SIGNAL(textEdited(QString)),
             this, SLOT(urlEdited()));
    connect( ui->locationUrl, SIGNAL(urlSelected(QUrl)),
             this, SLOT(urlEdited()));
    connect( ui->appNameEdit, SIGNAL(textEdited(QString)),
             this, SLOT(nameChanged()) );

    m_listView = new KDevelop::MultiLevelListView(this);
    m_listView->setLevels(2);
    m_listView->setHeaderLabels(QStringList() << i18n("Category") << i18n("Project Type"));
    m_listView->setModel(templatesModel);
    m_listView->setLastModelsFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
    m_listView->setContentsMargins(0, 0, 0, 0);
    connect (m_listView, SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)), SLOT(typeChanged(QModelIndex)));
    ui->gridLayout->addWidget(m_listView, 0, 0, 1, 1);
    typeChanged(m_listView->currentIndex());

    connect( ui->templateType, SIGNAL(currentIndexChanged(int)),
             this, SLOT(templateChanged(int)) );

    KNS3::Button* knsButton = new KNS3::Button(i18n("Get More Templates"), "kdevappwizard.knsrc", m_listView);
    connect (knsButton, SIGNAL(dialogFinished(KNS3::Entry::List)), 
             this, SLOT(templatesDownloaded(KNS3::Entry::List)));
    m_listView->addWidget(0, knsButton);

    KPushButton* loadButton = new KPushButton(m_listView);
    loadButton->setText(i18n("Load Template From File"));
    loadButton->setIcon(QIcon::fromTheme("application-x-archive"));
    connect (loadButton, SIGNAL(clicked(bool)), this, SLOT(loadFileClicked()));
    m_listView->addWidget(0, loadButton);

    m_wizardDialog = wizardDialog;
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

void ProjectSelectionPage::typeChanged(const QModelIndex& idx)
{
    if (!idx.model())
    {
        kDebug() << "Index with no model";
        return;
    }
    int children = idx.model()->rowCount(idx);
    ui->templateType->setVisible(children);
    ui->templateType->setEnabled(children > 1);
    if (children) {
        ui->templateType->setModel(m_templatesModel);
        ui->templateType->setRootModelIndex(idx);
        ui->templateType->setCurrentIndex(0);
        itemChanged(idx.child(0, 0));
    } else {
        itemChanged(idx);
    }
}

void ProjectSelectionPage::templateChanged(int current)
{
    QModelIndex idx=m_templatesModel->index(current, 0, ui->templateType->rootModelIndex());
    itemChanged(idx);
}

void ProjectSelectionPage::itemChanged( const QModelIndex& current)
{
    QString picPath = current.data( KDevelop::TemplatesModel::IconNameRole ).toString();
    if( picPath.isEmpty() ) {
        QIcon icon("kdevelop");
        ui->icon->setPixmap(icon.pixmap(128, 128));
        ui->icon->setFixedHeight(128);
    } else {
        QPixmap pixmap( picPath );
        ui->icon->setPixmap( pixmap );
        ui->icon->setFixedHeight( pixmap.height() );
    }
    // header name is either from this index directly or the parents if we show the combo box
    const QVariant headerData = ui->templateType->isVisible()
                                    ? current.parent().data()
                                    : current.data();
    ui->header->setText(QString("<h1>%1</h1>").arg(headerData.toString().trimmed()));
    ui->description->setText(current.data(KDevelop::TemplatesModel::CommentRole).toString());
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

QUrl ProjectSelectionPage::location()
{
    QUrl url = ui->locationUrl->url().adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + '/' + encodedAppName());
    return url;
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

    if( !appName().isEmpty() )
    {
        QString appname = appName();
        QString templatefile = m_wizardDialog->appInfo().appTemplate;

        // Read template file
        KConfig config(templatefile);
        KConfigGroup configgroup(&config, "General");
        QString pattern = configgroup.readEntry( "ValidProjectName" ,  "^[a-zA-Z][a-zA-Z0-9_]+$" );

        // Validation
        int pos = 0;
        QRegExp regex( pattern );
        QRegExpValidator validator( regex );
        if( validator.validate(appname, pos) == QValidator::Invalid )
        {
            ui->locationValidLabel->setText( i18n("Invalid project name") );
            setForeground(ui->locationValidLabel, KColorScheme::NegativeText);
            emit invalid();
            return;
        }
    }

    QDir tDir(url.toLocalFile( KUrl::RemoveTrailingSlash ));
    while (!tDir.exists() && !tDir.isRoot()) {
        if (!tDir.cdUp()) {
            break;
        }
    }

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

QStandardItem* ProjectSelectionPage::getCurrentItem() const
{
    QStandardItem* item = m_templatesModel->itemFromIndex( m_listView->currentIndex() );
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
    QFileInfo fi(location().toLocalFile());
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

void ProjectSelectionPage::loadFileClicked()
{
    QString filter = "application/x-desktop application/x-bzip-compressed-tar application/zip";
    QString fileName = KFileDialog::getOpenFileName(KUrl("kfiledialog:///kdevapptemplate"), filter, this);

    if (!fileName.isEmpty())
    {
        QString destination = m_templatesModel->loadTemplateFile(fileName);
        QModelIndexList indexes = m_templatesModel->templateIndexes(destination);
        if (indexes.size() > 2)
        {
            m_listView->setCurrentIndex(indexes.at(1));
            ui->templateType->setCurrentIndex(indexes.at(2).row());
        }
    }
}

void ProjectSelectionPage::templatesDownloaded (const KNS3::Entry::List& entries)
{
    if (entries.isEmpty()) {
        return;
    }

    m_templatesModel->refresh();
    bool updated = false;

    foreach (const KNS3::Entry& entry, entries)
    {
        if (!entry.installedFiles().isEmpty())
        {
            updated = true;
            setCurrentTemplate(entry.installedFiles().first());
            break;
        }
    }

    if (!updated)
    {
        m_listView->setCurrentIndex(QModelIndex());
    }
}

void ProjectSelectionPage::setCurrentTemplate (const QString& fileName)
{
    QModelIndexList indexes = m_templatesModel->templateIndexes(fileName);
    if (indexes.size() > 1)
    {
        m_listView->setCurrentIndex(indexes.at(1));
    }
    if (indexes.size() > 2)
    {
        ui->templateType->setCurrentIndex(indexes.at(2).row());
    }
}

#include "projectselectionpage.moc"
