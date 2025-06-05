/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2011 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "projectselectionpage.h"

#include "appwizarddialog.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNSWidgets/Button>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/itemplateprovider.h>
#include <language/codegen/templatepreviewicon.h>
#include <language/codegen/templatesmodel.h>
#include <language/codegen/templatesviewhelper.h>

#include "ui_projectselectionpage.h"

#include <QDir>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

using namespace KDevelop;

namespace {
constexpr auto categoryViewLevelCount = 2;
constexpr auto lastCategoryViewLevel = categoryViewLevelCount - 1;
constexpr auto templateTypeViewLevel = categoryViewLevelCount;
} // unnamed namespace

class ProjectTemplatesViewHelper final : public TemplatesViewHelper
{
public:
    explicit ProjectTemplatesViewHelper(TemplatesModel& model, MultiLevelListView& categoryView,
                                        QComboBox& templateTypeView)
        : TemplatesViewHelper(model)
        , m_categoryView{categoryView}
        , m_templateTypeView{templateTypeView}
    {
    }

    [[nodiscard]] const QStandardItem* currentItem() const
    {
        const auto* const categoryItem = m_model.itemFromIndex(m_categoryView.currentIndex());
        if (!categoryItem || !categoryItem->hasChildren()) {
            return categoryItem;
        }

        const auto typeRow = m_templateTypeView.currentIndex();
        const auto typeIndex = m_model.index(typeRow, 0, m_templateTypeView.rootModelIndex());
        return m_model.itemFromIndex(typeIndex);
    }

private:
    [[nodiscard]] QString currentTemplateFileName() const override
    {
        const auto* const item = currentItem();
        return item ? item->data(TemplatesModel::ArchiveFileRole).toString() : QString{};
    }

    bool setCurrentTemplate(const QList<QModelIndex>& indexes) override
    {
        if (indexes.size() <= lastCategoryViewLevel) {
            return false;
        }
        m_categoryView.setCurrentIndex(indexes.at(lastCategoryViewLevel));
        if (indexes.size() > templateTypeViewLevel) {
            m_templateTypeView.setCurrentIndex(indexes.at(templateTypeViewLevel).row());
        }
        return true;
    }

    MultiLevelListView& m_categoryView;
    QComboBox& m_templateTypeView;
};

ProjectSelectionPage::ProjectSelectionPage(ITemplateProvider& templateProvider, AppWizardDialog* wizardDialog)
    : AppWizardPageWidget(wizardDialog)
    , m_templatesModel{templateProvider.createTemplatesModel()}
{
    m_templatesModel->refresh();

    ui = new Ui::ProjectSelectionPage();
    ui->setupUi(this);
    ui->descriptionContent->setBackgroundRole(QPalette::Base);
    ui->descriptionContent->setForegroundRole(QPalette::Text);

    ui->locationUrl->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    ui->locationUrl->setUrl(KDevelop::ICore::self()->projectController()->projectsBaseDirectory());

    ui->locationValidWidget->hide();
    ui->locationValidWidget->setMessageType(KMessageWidget::Error);
    ui->locationValidWidget->setCloseButtonVisible(false);

    connect(ui->locationUrl->lineEdit(), &KLineEdit::textEdited, this,
            &ProjectSelectionPage::projectNameOrLocationChanged);
    connect(ui->locationUrl, &KUrlRequester::urlSelected, this, &ProjectSelectionPage::projectNameOrLocationChanged);
    connect(ui->projectNameEdit, &QLineEdit::textEdited, this, &ProjectSelectionPage::projectNameOrLocationChanged);

    ui->listView->setLevels(categoryViewLevelCount);
    ui->listView->setHeaderLabels(QStringList{
        i18nc("@title:column", "Category"),
        i18nc("@title:column", "Project Type")
    });
    ui->listView->setModel(m_templatesModel.get());
    ui->listView->setLastLevelViewMode(MultiLevelListView::DirectChildren);
    connect (ui->listView, &MultiLevelListView::currentIndexChanged, this, &ProjectSelectionPage::typeChanged);
    if (const auto index = ui->listView->currentIndex(); index.isValid()) {
        typeChanged(index);
    }

    connect( ui->templateType, QOverload<int>::of(&QComboBox::currentIndexChanged),
             this, &ProjectSelectionPage::templateChanged );

    auto* getMoreButton = new KNSWidgets::Button(i18nc("@action:button", "Get More Templates"),
                                                 templateProvider.knsConfigurationFile(), ui->listView);
    connect(getMoreButton, &KNSWidgets::Button::dialogFinished, this,
            [this](const QList<KNSCore::Entry>& changedEntries) {
                if (!viewHelper().handleNewStuffDialogFinished(changedEntries)) {
                    makeFirstTemplateCurrent();
                }
            });
    ui->listView->addWidget(0, getMoreButton);

    auto* loadButton = new QPushButton(ui->listView);
    loadButton->setText(i18nc("@action:button", "Load Template from File"));
    loadButton->setIcon(QIcon::fromTheme(QStringLiteral("application-x-archive")));
    connect(loadButton, &QPushButton::clicked, this, [this] {
        if (!viewHelper().loadTemplatesFromFiles(this)) {
            makeFirstTemplateCurrent();
        }
    });
    ui->listView->addWidget(0, loadButton);

    m_wizardDialog = wizardDialog;
}

ProjectSelectionPage::~ProjectSelectionPage()
{
    delete ui;
}

void ProjectSelectionPage::typeChanged(const QModelIndex& idx)
{
    Q_ASSERT_X(idx.isValid(), Q_FUNC_INFO, "MultiLevelListView::currentIndexChanged() emitted an invalid index");

    int children = idx.model()->rowCount(idx);
    ui->templateType->setVisible(children);
    ui->templateType->setEnabled(children > 1);
    if (children) {
        ui->templateType->setModel(m_templatesModel.get());
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

QString ProjectSelectionPage::selectedTemplate() const
{
    const auto* const item = currentItem();
    if (item)
        return item->data().toString();
    else
        return QString();
}

QUrl ProjectSelectionPage::location() const
{
    QUrl url = ui->locationUrl->url().adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + QString::fromUtf8(encodedProjectName()));
    return url;
}

QString ProjectSelectionPage::projectName() const
{
    return ui->projectNameEdit->text();
}

void ProjectSelectionPage::projectNameOrLocationChanged()
{
    validateData();
    // location() depends both on the project name and on its location
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

    {
        auto projectName = this->projectName();
        if (projectName.isEmpty()) {
            ui->locationValidWidget->setText( i18n("Empty project name") );
            ui->locationValidWidget->animatedShow();
            emit invalid();
            return;
        }

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

    if (const auto* const item = currentItem(); !item || item->hasChildren()) {
        ui->locationValidWidget->setText( i18n("Invalid project template, please choose a leaf item") );
        ui->locationValidWidget->animatedShow();
        emit invalid();
        return;
    }

    // Check for non-empty target directory.
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

    ui->locationValidWidget->animatedHide();
    emit valid();
}

QByteArray ProjectSelectionPage::encodedProjectName() const
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

ProjectTemplatesViewHelper ProjectSelectionPage::viewHelper()
{
    return ProjectTemplatesViewHelper(*m_templatesModel, *ui->listView, *ui->templateType);
}

const QStandardItem* ProjectSelectionPage::currentItem() const
{
    // ProjectTemplatesViewHelper::currentItem() is const-qualified, which justifies the const_cast
    return const_cast<ProjectSelectionPage*>(this)->viewHelper().currentItem();
}

void ProjectSelectionPage::makeFirstTemplateCurrent()
{
    itemChanged({}); // in case the model is empty or selecting the first template fails
    // Set an invalid index as current to select the very first template because something should always be selected.
    // Furthermore, MultiLevelListView is not a real item view and requires manual setting of its current index after
    // the model is refreshed in order to prevent a crash in QAbstractProxyModelPrivate::emitHeaderDataChanged().
    Q_ASSERT(!ui->listView->currentIndex().isValid());
    ui->listView->setCurrentIndex({});
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

#include "moc_projectselectionpage.cpp"
