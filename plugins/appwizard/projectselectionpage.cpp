/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2011 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "projectselectionpage.h"

#include "appwizarddialog.h"
#include "debug.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNSCore/Entry>
#include <KNSWidgets/Button>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <language/codegen/templatepreviewicon.h>

#include <util/scopeddialog.h>

#include "ui_projectselectionpage.h"
#include "projecttemplatesmodel.h"

#include <QDir>
#include <QFileDialog>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

using namespace KDevelop;

ProjectSelectionPage::ProjectSelectionPage(ProjectTemplatesModel *templatesModel, AppWizardDialog *wizardDialog)
    : AppWizardPageWidget(wizardDialog), m_templatesModel(templatesModel)
{
    ui = new Ui::ProjectSelectionPage();
    ui->setupUi(this);
    ui->descriptionContent->setBackgroundRole(QPalette::Base);
    ui->descriptionContent->setForegroundRole(QPalette::Text);

    ui->locationUrl->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    ui->locationUrl->setUrl(KDevelop::ICore::self()->projectController()->projectsBaseDirectory());

    ui->locationValidWidget->hide();
    ui->locationValidWidget->setMessageType(KMessageWidget::Error);
    ui->locationValidWidget->setCloseButtonVisible(false);

    connect( ui->locationUrl->lineEdit(), &KLineEdit::textEdited,
             this, &ProjectSelectionPage::urlEdited);
    connect( ui->locationUrl, &KUrlRequester::urlSelected,
             this, &ProjectSelectionPage::urlEdited);
    connect( ui->projectNameEdit, &QLineEdit::textEdited,
             this, &ProjectSelectionPage::nameChanged );

    ui->listView->setLevels(2);
    ui->listView->setHeaderLabels(QStringList{
        i18nc("@title:column", "Category"),
        i18nc("@title:column", "Project Type")
    });
    ui->listView->setModel(templatesModel);
    ui->listView->setLastLevelViewMode(MultiLevelListView::DirectChildren);
    connect (ui->listView, &MultiLevelListView::currentIndexChanged, this, &ProjectSelectionPage::typeChanged);
    typeChanged(ui->listView->currentIndex());

    connect( ui->templateType, QOverload<int>::of(&QComboBox::currentIndexChanged),
             this, &ProjectSelectionPage::templateChanged );

    auto* getMoreButton = new KNSWidgets::Button(i18nc("@action:button", "Get More Templates"),
                                                 QStringLiteral("kdevappwizard.knsrc"), ui->listView);
    connect(getMoreButton, &KNSWidgets::Button::dialogFinished, this,
            &ProjectSelectionPage::handleNewStuffDialogFinished);
    ui->listView->addWidget(0, getMoreButton);

    auto* loadButton = new QPushButton(ui->listView);
    loadButton->setText(i18nc("@action:button", "Load Template from File"));
    loadButton->setIcon(QIcon::fromTheme(QStringLiteral("application-x-archive")));
    connect (loadButton, &QPushButton::clicked, this, &ProjectSelectionPage::loadFileClicked);
    ui->listView->addWidget(0, loadButton);

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
        qCDebug(PLUGIN_APPWIZARD) << "Index with no model";
        return;
    }
    int children = idx.model()->rowCount(idx);
    ui->templateType->setVisible(children);
    ui->templateType->setEnabled(children > 1);
    if (children) {
        ui->templateType->setModel(m_templatesModel);
        ui->templateType->setRootModelIndex(idx);
        ui->templateType->setCurrentIndex(0);
        itemChanged(idx.model()->index(0, 0, idx));
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
    TemplatePreviewIcon icon = current.data(KDevelop::TemplatesModel::PreviewIconRole).value<TemplatePreviewIcon>();

    QPixmap pixmap = icon.pixmap();
    ui->icon->setPixmap(pixmap);
    ui->icon->setFixedHeight(pixmap.height());
    // header name is either from this index directly or the parents if we show the combo box
    const QVariant headerData = ui->templateType->isVisible()
                                    ? current.parent().data()
                                    : current.data();
    ui->header->setText(QStringLiteral("<h1>%1</h1>").arg(headerData.toString().trimmed()));
    ui->description->setText(current.data(KDevelop::TemplatesModel::CommentRole).toString());
    validateData();

    ui->propertiesBox->setEnabled(true);
}

QString ProjectSelectionPage::selectedTemplate()
{
    QStandardItem *item = currentItem();
    if (item)
        return item->data().toString();
    else
        return QString();
}

QUrl ProjectSelectionPage::location()
{
    QUrl url = ui->locationUrl->url().adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + QString::fromUtf8(encodedProjectName()));
    return url;
}

QString ProjectSelectionPage::projectName()
{
    return ui->projectNameEdit->text();
}

void ProjectSelectionPage::urlEdited()
{
    validateData();
    emit locationChanged( location() );
}

void ProjectSelectionPage::validateData()
{
    QUrl url = ui->locationUrl->url();
    if( !url.isLocalFile() || url.isEmpty() )
    {
        ui->locationValidWidget->setText( i18n("Invalid location") );
        ui->locationValidWidget->animatedShow();
        emit invalid();
        return;
    }

    if (projectName().isEmpty()) {
        ui->locationValidWidget->setText( i18n("Empty project name") );
        ui->locationValidWidget->animatedShow();
        emit invalid();
        return;
    }

    if (!projectName().isEmpty()) {
        QString projectName = this->projectName();
        QString templatefile = m_wizardDialog->appInfo().appTemplate;

        // Read template file
        KConfig config(templatefile);
        KConfigGroup configgroup(&config, QStringLiteral("General"));
        QString pattern = configgroup.readEntry( "ValidProjectName" ,  "^[a-zA-Z][a-zA-Z0-9_-]+$" );

        // Validation
        int pos = 0;
        const QRegularExpressionValidator validator(QRegularExpression{pattern});
        if( validator.validate(projectName, pos) == QValidator::Invalid )
        {
            ui->locationValidWidget->setText( i18n("Invalid project name") );
            ui->locationValidWidget->animatedShow();
            emit invalid();
            return;
        }
    }

    QDir tDir(url.toLocalFile());
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
            ui->locationValidWidget->setText( i18n("Unable to create subdirectories, "
                                                  "missing permissions on: %1", tDir.absolutePath()) );
            ui->locationValidWidget->animatedShow();
            emit invalid();
            return;
        }
    }

    QStandardItem* item = currentItem();
    if( item && !item->hasChildren() )
    {
        ui->locationValidWidget->animatedHide();
        emit valid();
    } else
    {
        ui->locationValidWidget->setText( i18n("Invalid project template, please choose a leaf item") );
        ui->locationValidWidget->animatedShow();
        emit invalid();
        return;
    }

    // Check for non-empty target directory. Not an error, but need to display a warning.
    url.setPath(url.path() + QLatin1Char('/') + QString::fromUtf8(encodedProjectName()));
    QFileInfo fi( url.toLocalFile() );
    if( fi.exists() && fi.isDir() )
    {
        if( !QDir( fi.absoluteFilePath()).entryList( QDir::NoDotAndDotDot | QDir::AllEntries ).isEmpty() )
        {
            ui->locationValidWidget->setText( i18n("Path already exists and contains files. Open it as a project.") );
            ui->locationValidWidget->animatedShow();
            emit invalid();
            return;
        }
    }
}

QByteArray ProjectSelectionPage::encodedProjectName()
{
    // : < > * ? / \ | " are invalid on windows
    QByteArray tEncodedName = projectName().toUtf8();
    for (int i = 0; i < tEncodedName.size(); ++i)
    {
        QChar tChar(QLatin1Char(tEncodedName.at(i)));
        if (tChar.isDigit() || tChar.isSpace() || tChar.isLetter() || tChar == QLatin1Char('%')) {
            continue;
        }

        QByteArray tReplace = QUrl::toPercentEncoding( tChar );
        tEncodedName.replace( tEncodedName.at( i ) ,tReplace );
        i =  i + tReplace.size() - 1;
    }
    return tEncodedName;
}

QStandardItem* ProjectSelectionPage::currentItem() const
{
    QStandardItem* item = m_templatesModel->itemFromIndex( ui->listView->currentIndex() );
    if ( item && item->hasChildren() )
    {
        const int current = ui->templateType->currentIndex();
        const QModelIndex idx = m_templatesModel->index( current, 0, ui->templateType->rootModelIndex() );
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
            int res = KMessageBox::questionTwoActions(this,
                                                      i18n("The specified path already exists and contains files. "
                                                           "Are you sure you want to proceed?"),
                                                      {}, KStandardGuiItem::cont(), KStandardGuiItem::cancel());
            return res == KMessageBox::PrimaryAction;
        }
    }
    return true;
}

void ProjectSelectionPage::loadFileClicked()
{
    const QStringList supportedMimeTypes {
        QStringLiteral("application/x-desktop"),
        QStringLiteral("application/x-bzip-compressed-tar"),
        QStringLiteral("application/zip")
    };
    ScopedDialog<QFileDialog> fileDialog(this, i18nc("@title:window", "Load Template from File"));
    fileDialog->setMimeTypeFilters(supportedMimeTypes);
    fileDialog->setFileMode(QFileDialog::ExistingFiles);

    if (!fileDialog->exec()) {
        return;
    }

    const auto& fileNames = fileDialog->selectedFiles();
    for (const auto& fileName : fileNames) {
        QString destination = m_templatesModel->loadTemplateFile(fileName);
        QModelIndexList indexes = m_templatesModel->templateIndexes(destination);
        if (indexes.size() > 2)
        {
            ui->listView->setCurrentIndex(indexes.at(1));
            ui->templateType->setCurrentIndex(indexes.at(2).row());
        }
    }
}

void ProjectSelectionPage::handleNewStuffDialogFinished(const QList<KNSCore::Entry>& changedEntries)
{
    if (changedEntries.isEmpty()) {
        return;
    }

    m_templatesModel->refresh();
    bool updated = false;

    for (const auto& entry : changedEntries) {
        if (!entry.installedFiles().isEmpty())
        {
            updated = true;
            setCurrentTemplate(entry.installedFiles().at(0));
            break;
        }
    }

    if (!updated)
    {
        ui->listView->setCurrentIndex(QModelIndex());
    }
}

void ProjectSelectionPage::setCurrentTemplate (const QString& fileName)
{
    QModelIndexList indexes = m_templatesModel->templateIndexes(fileName);
    if (indexes.size() > 1)
    {
        ui->listView->setCurrentIndex(indexes.at(1));
    }
    if (indexes.size() > 2)
    {
        ui->templateType->setCurrentIndex(indexes.at(2).row());
    }
}

#include "moc_projectselectionpage.cpp"
