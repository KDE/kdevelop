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

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
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

class ProjectSelectionPagePrivate final : public TemplatesViewHelper
{
public:
    explicit ProjectSelectionPagePrivate(ProjectSelectionPage* q, ITemplateProvider& templateProvider,
                                         const AppWizardDialog* wizardDialog)
        : TemplatesViewHelper(templateProvider)
        , wizardDialog{(Q_ASSERT(wizardDialog), *wizardDialog)}
        , q_ptr{q}
    {
        Q_ASSERT(q);
    }

    void projectTypeChanged(const QModelIndex& current);
    void templateTypeChanged(int current);
    void currentItemChanged(const QModelIndex& current);
    void projectNameOrLocationChanged();
    void validateData();

    [[nodiscard]] QByteArray encodedProjectName() const;
    [[nodiscard]] const QStandardItem* currentItem() const;

    const AppWizardDialog& wizardDialog;
    Ui::ProjectSelectionPage ui;

private:
    [[nodiscard]] QWidget* dialogParent() override
    {
        Q_Q(ProjectSelectionPage);
        return q;
    }

    [[nodiscard]] QString currentTemplateFileName() const override
    {
        const auto* const item = currentItem();
        return item ? item->data(TemplatesModel::ArchiveFileRole).toString() : QString{};
    }

    bool setCurrentTemplate(const QList<QModelIndex>& indexes) override;
    void handleNoTemplateSelectedAfterRefreshingModel() override;

    ProjectSelectionPage* const q_ptr;
    Q_DECLARE_PUBLIC(ProjectSelectionPage)
};

ProjectSelectionPage::ProjectSelectionPage(ITemplateProvider& templateProvider, AppWizardDialog* wizardDialog)
    : AppWizardPageWidget(wizardDialog)
    , d_ptr{std::make_unique<ProjectSelectionPagePrivate>(this, templateProvider, wizardDialog)}
{
    Q_D(ProjectSelectionPage);

    d->ui.setupUi(this);

    d->ui.descriptionContent->setBackgroundRole(QPalette::Base);
    d->ui.descriptionContent->setForegroundRole(QPalette::Text);

    d->ui.locationUrl->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    d->ui.locationUrl->setUrl(ICore::self()->projectController()->projectsBaseDirectory());

    d->ui.locationValidWidget->hide();
    d->ui.locationValidWidget->setMessageType(KMessageWidget::Error);
    d->ui.locationValidWidget->setCloseButtonVisible(false);

    const auto projectNameOrLocationChanged = [d] {
        d->projectNameOrLocationChanged();
    };
    connect(d->ui.locationUrl->lineEdit(), &KLineEdit::textEdited, this, projectNameOrLocationChanged);
    connect(d->ui.locationUrl, &KUrlRequester::urlSelected, this, projectNameOrLocationChanged);
    connect(d->ui.projectNameEdit, &QLineEdit::textEdited, this, projectNameOrLocationChanged);

    d->ui.listView->setLevels(categoryViewLevelCount);
    d->ui.listView->setHeaderLabels(
        QStringList{i18nc("@title:column", "Category"), i18nc("@title:column", "Project Type")});
    d->ui.listView->setModel(&d->model());
    d->ui.listView->setLastLevelViewMode(MultiLevelListView::DirectChildren);
    connect(d->ui.listView, &MultiLevelListView::currentIndexChanged, this, [d](const QModelIndex& current) {
        d->projectTypeChanged(current);
    });
    if (const auto index = d->ui.listView->currentIndex(); index.isValid()) {
        d->projectTypeChanged(index);
    }

    connect(d->ui.templateType, &QComboBox::currentIndexChanged, this, [d](int index) {
        d->templateTypeChanged(index);
    });

    auto* const getMoreButton = d->createGetMoreTemplatesButton(templateProvider, d->ui.listView);
    Q_ASSERT_X(getMoreButton, Q_FUNC_INFO, "An Application Templates configuration file for Get Hot New Stuff exists");
    d->ui.listView->addWidget(0, getMoreButton);
    auto* const loadButton = d->createLoadTemplateFromFileButton(d->ui.listView);
    d->ui.listView->addWidget(0, loadButton);
}

ProjectSelectionPage::~ProjectSelectionPage() = default;

void ProjectSelectionPagePrivate::projectTypeChanged(const QModelIndex& current)
{
    Q_ASSERT_X(current.isValid(), Q_FUNC_INFO, "MultiLevelListView::currentIndexChanged() emitted an invalid index");

    const auto childCount = current.model()->rowCount(current);
    ui.templateType->setVisible(childCount > 0);
    ui.templateType->setEnabled(childCount > 1);
    if (childCount > 0) {
        ui.templateType->setModel(&model());
        ui.templateType->setRootModelIndex(current);
        ui.templateType->setCurrentIndex(0);
        currentItemChanged(current.model()->index(0, 0, current));
    } else {
        currentItemChanged(current);
    }
}

void ProjectSelectionPagePrivate::templateTypeChanged(int current)
{
    const auto index = model().index(current, 0, ui.templateType->rootModelIndex());
    currentItemChanged(index);
}

void ProjectSelectionPagePrivate::currentItemChanged(const QModelIndex& current)
{
    TemplatePreviewIcon icon = current.data(KDevelop::TemplatesModel::PreviewIconRole).value<TemplatePreviewIcon>();

    QPixmap pixmap = icon.pixmap();
    ui.icon->setPixmap(pixmap);
    ui.icon->setFixedHeight(pixmap.height());
    // header name is either from this index directly or the parents if we show the combo box
    const auto headerData = ui.templateType->isVisible() ? current.parent().data() : current.data();
    ui.header->setText(QLatin1String("<h1>%1</h1>").arg(headerData.toString().trimmed()));
    ui.description->setText(current.data(TemplatesModel::CommentRole).toString());
    validateData();

    ui.propertiesBox->setEnabled(true);
}

QString ProjectSelectionPage::selectedTemplate() const
{
    Q_D(const ProjectSelectionPage);

    const auto* const item = d->currentItem();
    if (item)
        return item->data().toString();
    else
        return QString();
}

QUrl ProjectSelectionPage::location() const
{
    Q_D(const ProjectSelectionPage);

    auto url = d->ui.locationUrl->url().adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + QString::fromUtf8(d->encodedProjectName()));
    return url;
}

QString ProjectSelectionPage::projectName() const
{
    Q_D(const ProjectSelectionPage);

    return d->ui.projectNameEdit->text();
}

void ProjectSelectionPagePrivate::projectNameOrLocationChanged()
{
    Q_Q(ProjectSelectionPage);

    validateData();
    // location() depends both on the project name and on its location
    Q_EMIT q->locationChanged(q->location());
}

void ProjectSelectionPagePrivate::validateData()
{
    Q_Q(ProjectSelectionPage);

    const auto reportError = [this](const QString& errorMessage) {
        Q_Q(ProjectSelectionPage);
        ui.locationValidWidget->setText(errorMessage);
        ui.locationValidWidget->animatedShow();
        Q_EMIT q->invalid();
    };

    auto url = ui.locationUrl->url();
    if( !url.isLocalFile() || url.isEmpty() )
    {
        reportError(i18n("Invalid location"));
        return;
    }

    {
        auto projectName = q->projectName();
        if (projectName.isEmpty()) {
            reportError(i18n("Empty project name"));
            return;
        }

        const auto templatefile = wizardDialog.appInfo().appTemplate;
        // Read template file
        KConfig config(templatefile);
        KConfigGroup configgroup(&config, QStringLiteral("General"));
        QString pattern = configgroup.readEntry( "ValidProjectName" ,  "^[a-zA-Z][a-zA-Z0-9_-]+$" );

        // Validation
        int pos = 0;
        const QRegularExpressionValidator validator(QRegularExpression{pattern});
        if( validator.validate(projectName, pos) == QValidator::Invalid )
        {
            reportError(i18n("Invalid project name"));
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
            reportError(i18n("Unable to create subdirectories, missing permissions on: %1", tDir.absolutePath()));
            return;
        }
    }

    if (const auto* const item = currentItem(); !item || item->hasChildren()) {
        reportError(i18n("Invalid project template, please choose a leaf item"));
        return;
    }

    // Check for non-empty target directory.
    url.setPath(url.path() + QLatin1Char('/') + QString::fromUtf8(encodedProjectName()));
    QFileInfo fi( url.toLocalFile() );
    if( fi.exists() && fi.isDir() )
    {
        if( !QDir( fi.absoluteFilePath()).entryList( QDir::NoDotAndDotDot | QDir::AllEntries ).isEmpty() )
        {
            reportError(i18n("Path already exists and contains files. Open it as a project."));
            return;
        }
    }

    ui.locationValidWidget->animatedHide();
    Q_EMIT q->valid();
}

QByteArray ProjectSelectionPagePrivate::encodedProjectName() const
{
    Q_Q(const ProjectSelectionPage);

    // : < > * ? / \ | " are invalid on windows
    auto tEncodedName = q->projectName().toUtf8();
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

const QStandardItem* ProjectSelectionPagePrivate::currentItem() const
{
    const auto* const categoryItem = model().itemFromIndex(ui.listView->currentIndex());
    if (!categoryItem || !categoryItem->hasChildren()) {
        return categoryItem;
    }

    const auto typeRow = ui.templateType->currentIndex();
    const auto typeIndex = model().index(typeRow, 0, ui.templateType->rootModelIndex());
    return model().itemFromIndex(typeIndex);
}

bool ProjectSelectionPagePrivate::setCurrentTemplate(const QList<QModelIndex>& indexes)
{
    if (indexes.size() <= lastCategoryViewLevel) {
        return false;
    }
    ui.listView->setCurrentIndex(indexes.at(lastCategoryViewLevel));
    if (indexes.size() > templateTypeViewLevel) {
        ui.templateType->setCurrentIndex(indexes.at(templateTypeViewLevel).row());
    }
    return true;
}

void ProjectSelectionPagePrivate::handleNoTemplateSelectedAfterRefreshingModel()
{
    currentItemChanged({}); // in case the model is empty or selecting the first template fails
    // Set an invalid index as current to select the very first template because something should always be selected.
    // Furthermore, MultiLevelListView is not a real item view and requires manual setting of its current index after
    // the model is refreshed in order to prevent a crash in QAbstractProxyModelPrivate::emitHeaderDataChanged().
    Q_ASSERT(!ui.listView->currentIndex().isValid());
    ui.listView->setCurrentIndex({});
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
