/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "templateselectionpage.h"

#include "templateclassassistant.h"
#include "templatepreview.h"

#include <language/codegen/templatesmodel.h>
#include <language/codegen/sourcefiletemplate.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/templaterenderer.h>
#include <language/interfaces/ilanguagesupport.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>
#include <util/scopeddialog.h>

#include "ui_templateselection.h"

#include <QFileDialog>
#include <QPushButton>
#include <QTemporaryDir>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KNSWidgets/Button>
#include <KTextEditor/Document>

using namespace KDevelop;

static constexpr QLatin1String LastUsedTemplateEntry("LastUsedTemplate");
static constexpr QLatin1String FileTemplatesGroup("SourceFileTemplates");

class KDevelop::TemplateSelectionPagePrivate
{
public:
    explicit TemplateSelectionPagePrivate(TemplateSelectionPage* page_)
    : page(page_)
    {}

    TemplateSelectionPage* page;
    Ui::TemplateSelection* ui;
    QString selectedTemplate;
    TemplateClassAssistant* assistant;
    TemplatesModel* model;

    void currentTemplateChanged(const QModelIndex& index);
    void handleNewStuffDialogFinished();
    void loadFileClicked();
    void previewTemplate(const QString& templateFile);
};

void TemplateSelectionPagePrivate::currentTemplateChanged(const QModelIndex& index)
{
    // delete preview tabs
    if (!index.isValid() || index.model()->hasChildren(index))
    {
        // invalid or has child
        assistant->setValid(assistant->currentPage(), false);
        ui->previewLabel->setVisible(false);
        ui->tabWidget->setVisible(false);
    } else {
        selectedTemplate = model->data(index, TemplatesModel::DescriptionFileRole).toString();
        assistant->setValid(assistant->currentPage(), true);
        previewTemplate(selectedTemplate);
        ui->previewLabel->setVisible(true);
        ui->tabWidget->setVisible(true);
        ui->previewLabel->setText(i18nc("%1: template comment", "<b>Preview:</b> %1",
                                        index.data(TemplatesModel::CommentRole).toString()));
    }
}

void TemplateSelectionPagePrivate::previewTemplate(const QString& file)
{
    SourceFileTemplate fileTemplate(file);
    if (!fileTemplate.isValid() || fileTemplate.outputFiles().isEmpty()) {
        return;
    }

    TemplatePreviewRenderer renderer;
    // set default option values
    if (fileTemplate.hasCustomOptions()) {
        QVariantHash extraVars;
        const auto& optionGroups = fileTemplate.customOptions(&renderer);
        for (const auto& optionGroup : optionGroups) {
            for (const auto& entry : optionGroup.options) {
                extraVars[entry.name] = entry.value;
            }
        }
        renderer.addVariables(extraVars);
    }
    renderer.setEmptyLinesPolicy(TemplateRenderer::TrimEmptyLines);

    QTemporaryDir dir;
    QUrl base = QUrl::fromLocalFile(dir.path() + QLatin1Char('/'));
    QHash<QString, QUrl> fileUrls;
    const auto outputFiles = fileTemplate.outputFiles();
    for (const SourceFileTemplate::OutputFile& out : outputFiles) {
        QUrl url = base.resolved(QUrl(renderer.render(out.outputName)));
        fileUrls.insert(out.identifier, url);
    }
    DocumentChangeSet changes = renderer.renderFileTemplate(fileTemplate, base, fileUrls);
    changes.setActivationPolicy(DocumentChangeSet::DoNotActivate);
    changes.setUpdateHandling(DocumentChangeSet::NoUpdate);
    DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
    if (!result) {
        return;
    }

    int idx = 0;
    for (const SourceFileTemplate::OutputFile& out : outputFiles) {
        TemplatePreview* preview = nullptr;
        if (ui->tabWidget->count() > idx) {
            // reuse existing tab
            preview = qobject_cast<TemplatePreview*>(ui->tabWidget->widget(idx));
            ui->tabWidget->setTabText(idx, out.label);
            Q_ASSERT(preview);
        } else {
            // create new tabs on demand
            preview = new TemplatePreview(page);
            ui->tabWidget->addTab(preview, out.label);
        }
        preview->document()->openUrl(fileUrls.value(out.identifier));
        ++idx;
    }
    // remove superfluous tabs from last time
    while (ui->tabWidget->count() > fileUrls.size()) {
        delete ui->tabWidget->widget(fileUrls.size());
    }
    return;
}

void TemplateSelectionPagePrivate::handleNewStuffDialogFinished()
{
    model->refresh();
}

void TemplateSelectionPagePrivate::loadFileClicked()
{
    const QStringList filters{
        QStringLiteral("application/x-desktop"),
        QStringLiteral("application/x-bzip-compressed-tar"),
        QStringLiteral("application/zip")
    };
    ScopedDialog<QFileDialog> dlg(page);
    dlg->setMimeTypeFilters(filters);
    dlg->setFileMode(QFileDialog::ExistingFiles);

    if (!dlg->exec())
    {
        return;
    }

    const auto selectedFiles = dlg->selectedFiles();
    for (const QString& fileName : selectedFiles) {
        QString destination = model->loadTemplateFile(fileName);
        QModelIndexList indexes = model->templateIndexes(destination);
        int n = indexes.size();
        if (n > 1)
        {
            ui->view->setCurrentIndex(indexes[1]);
        }
    }
}

void TemplateSelectionPage::saveConfig()
{
    KSharedConfigPtr config;
    if (IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl()))
    {
        config = project->projectConfiguration();
    }
    else
    {
        config = ICore::self()->activeSession()->config();
    }

    KConfigGroup group(config, FileTemplatesGroup);
    group.writeEntry(LastUsedTemplateEntry, d->selectedTemplate);
    group.sync();
}

TemplateSelectionPage::TemplateSelectionPage(TemplateClassAssistant* parent)
: QWidget(parent)
, d(new TemplateSelectionPagePrivate(this))
{
    d->assistant = parent;

    d->ui = new Ui::TemplateSelection;
    d->ui->setupUi(this);

    d->model = new TemplatesModel(QStringLiteral("kdevfiletemplates"), this);
    d->model->refresh();

    d->ui->view->setLevels(3);
    d->ui->view->setHeaderLabels(QStringList{
        i18nc("@title:column", "Language"),
        i18nc("@title:column", "Framework"),
        i18nc("@title:column", "Template")
    });
    d->ui->view->setModel(d->model);

    connect(d->ui->view, &MultiLevelListView::currentIndexChanged,
            this, [&] (const QModelIndex& index) { d->currentTemplateChanged(index); });

    QModelIndex templateIndex;
    while (d->model->hasIndex(0, 0, templateIndex))
    {
        templateIndex = d->model->index(0, 0, templateIndex);
    }

    KSharedConfigPtr config;
    if (IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl()))
    {
        config = project->projectConfiguration();
    }
    else
    {
        config = ICore::self()->activeSession()->config();
    }

    KConfigGroup group(config, FileTemplatesGroup);
    QString lastTemplate = group.readEntry(LastUsedTemplateEntry);

    QModelIndexList indexes = d->model->match(d->model->index(0, 0), TemplatesModel::DescriptionFileRole, lastTemplate, 1, Qt::MatchRecursive);

    if (!indexes.isEmpty())
    {
        templateIndex = indexes.first();
    }

    d->ui->view->setCurrentIndex(templateIndex);

    auto* getMoreButton = new KNSWidgets::Button(i18nc("@action:button", "Get More Templates..."),
                                                 QStringLiteral("kdevfiletemplates.knsrc"), d->ui->view);
    connect(getMoreButton, &KNSWidgets::Button::dialogFinished, this, [&]() {
        d->handleNewStuffDialogFinished();
    });
    d->ui->view->addWidget(0, getMoreButton);

    auto* loadButton = new QPushButton(QIcon::fromTheme(QStringLiteral("application-x-archive")), i18nc("@action:button", "Load Template from File"), d->ui->view);
    connect (loadButton, &QPushButton::clicked, this, [&] { d->loadFileClicked(); });
    d->ui->view->addWidget(0, loadButton);

    d->ui->view->setContentsMargins(0, 0, 0, 0);
}

TemplateSelectionPage::~TemplateSelectionPage()
{
    delete d->ui;
    delete d;
}

QSize TemplateSelectionPage::minimumSizeHint() const
{
    return QSize(400, 600);
}

QString TemplateSelectionPage::selectedTemplate() const
{
    return d->selectedTemplate;
}

#include "moc_templateselectionpage.cpp"
