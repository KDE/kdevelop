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
#include <language/codegen/templatesviewhelper.h>
#include <language/interfaces/ilanguagesupport.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>
#include <interfaces/itemplateprovider.h>

#include "ui_templateselection.h"

#include <QPushButton>
#include <QTemporaryDir>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KNSWidgets/Button>
#include <KTextEditor/Document>

using namespace KDevelop;

namespace {

static const char LastUsedTemplateEntry[] = "LastUsedTemplate";
static inline QString FileTemplatesGroup()
{
    return QStringLiteral("SourceFileTemplates");
}

constexpr auto viewLevelCount = 3;
constexpr auto lastViewLevel = viewLevelCount - 1;

class FileTemplatesViewHelper final : public TemplatesViewHelper
{
public:
    explicit FileTemplatesViewHelper(TemplatesModel& model, MultiLevelListView& view)
        : TemplatesViewHelper(model)
        , m_view{view}
    {
    }

private:
    [[nodiscard]] QString currentTemplateFileName() const override
    {
        return m_view.currentIndex().data(TemplatesModel::ArchiveFileRole).toString();
    }

    bool setCurrentTemplate(const QList<QModelIndex>& indexes) override
    {
        if (indexes.size() <= lastViewLevel) {
            // If indexes.empty(), the template is probably absent from the model (and the file system).
            // If the number of indexes is greater than zero but less than viewLevelCount, the template is invalid
            // or unsupported. Whether or not we set such a template as current in the view, if this function returns
            // true for it, a segmentation fault in QAbstractProxyModelPrivate::emitHeaderDataChanged() ensues.
            return false;
        }
        m_view.setCurrentIndex(indexes.constLast());
        return true;
    }

    MultiLevelListView& m_view;
};

} // unnamed namespace

class KDevelop::TemplateSelectionPagePrivate
{
public:
    explicit TemplateSelectionPagePrivate(TemplateSelectionPage* q, ITemplateProvider& templateProvider,
                                          TemplateClassAssistant* assistant)
        : model{templateProvider.createTemplatesModel()}
        , assistant{assistant}
        , q_ptr{q}
    {
        Q_ASSERT(q);
        Q_ASSERT(assistant);
        model->refresh();
    }

    const std::unique_ptr<TemplatesModel> model;
    TemplateClassAssistant* const assistant;
    Ui::TemplateSelection* ui;

    QString selectedTemplate;

    [[nodiscard]] FileTemplatesViewHelper viewHelper()
    {
        return FileTemplatesViewHelper(*model, *ui->view);
    }

    /**
     * Select the first template in @a ui->view.
     *
     * This function is used as a fallback when selecting a more relevant template fails.
     */
    void makeFirstTemplateCurrent();

    /**
     * Call this function when refreshing the model invalidates the current template.
     */
    void currentTemplateInvalidated();

    void currentTemplateChanged(const QModelIndex& index);
    void previewTemplate(const QString& templateFile);

private:
    TemplateSelectionPage* const q_ptr;
    Q_DECLARE_PUBLIC(TemplateSelectionPage)
};

void TemplateSelectionPagePrivate::makeFirstTemplateCurrent()
{
    currentTemplateInvalidated(); // in case the model is empty or selecting the first template fails
    // Set an invalid index as current to select the very first template because something should always be selected.
    // Furthermore, MultiLevelListView is not a real item view and requires manual setting of its current index after
    // the model is refreshed in order to prevent a crash in QAbstractProxyModelPrivate::emitHeaderDataChanged().
    Q_ASSERT(!ui->view->currentIndex().isValid());
    ui->view->setCurrentIndex({});
}

void TemplateSelectionPagePrivate::currentTemplateInvalidated()
{
    assistant->setValid(assistant->currentPage(), false);
    ui->previewLabel->setVisible(false);
    ui->tabWidget->setVisible(false);
}

void TemplateSelectionPagePrivate::currentTemplateChanged(const QModelIndex& index)
{
    Q_ASSERT_X(index.isValid(), Q_FUNC_INFO, "MultiLevelListView::currentIndexChanged() emitted an invalid index");
    Q_ASSERT_X(!index.model()->hasChildren(index), Q_FUNC_INFO,
               "Only leaf items are selectable in MultiLevelListView with the default level view mode SubTrees");

    selectedTemplate = index.data(TemplatesModel::DescriptionFileRole).toString();
    assistant->setValid(assistant->currentPage(), true);
    previewTemplate(selectedTemplate);
    ui->previewLabel->setVisible(true);
    ui->tabWidget->setVisible(true);
    ui->previewLabel->setText(i18nc("%1: template comment", "<b>Preview:</b> %1",
                                    index.data(TemplatesModel::CommentRole).toString()));
}

void TemplateSelectionPagePrivate::previewTemplate(const QString& file)
{
    Q_Q(TemplateSelectionPage);

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
            preview = new TemplatePreview(q);
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

void TemplateSelectionPage::saveConfig()
{
    Q_D(TemplateSelectionPage);

    KSharedConfigPtr config;
    if (IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl()))
    {
        config = project->projectConfiguration();
    }
    else
    {
        config = ICore::self()->activeSession()->config();
    }

    KConfigGroup group(config, FileTemplatesGroup());
    group.writeEntry(LastUsedTemplateEntry, d->selectedTemplate);
    group.sync();
}

TemplateSelectionPage::TemplateSelectionPage(ITemplateProvider& templateProvider, TemplateClassAssistant* parent)
    : QWidget(parent)
    , d_ptr{std::make_unique<TemplateSelectionPagePrivate>(this, templateProvider, parent)}
{
    Q_D(TemplateSelectionPage);

    d->ui = new Ui::TemplateSelection;
    d->ui->setupUi(this);

    d->ui->view->setLevels(viewLevelCount);
    d->ui->view->setHeaderLabels(QStringList{
        i18nc("@title:column", "Language"),
        i18nc("@title:column", "Framework"),
        i18nc("@title:column", "Template")
    });
    d->ui->view->setModel(d->model.get());

    connect(d->ui->view, &MultiLevelListView::currentIndexChanged, this, [d](const QModelIndex& current) {
        d->currentTemplateChanged(current);
    });

    d->currentTemplateInvalidated(); // in case the model is empty or selecting the last used or first template fails

    KSharedConfigPtr config;
    if (IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl()))
    {
        config = project->projectConfiguration();
    }
    else
    {
        config = ICore::self()->activeSession()->config();
    }

    KConfigGroup group(config, FileTemplatesGroup());
    QString lastTemplate = group.readEntry(LastUsedTemplateEntry);

    QModelIndex templateIndex;
    const auto indexes = d->model->match(d->model->index(0, 0), TemplatesModel::DescriptionFileRole, lastTemplate, 1,
                                         Qt::MatchRecursive);
    if (!indexes.isEmpty())
    {
        templateIndex = indexes.first();
    }

    d->ui->view->setCurrentIndex(templateIndex);

    auto* getMoreButton = new KNSWidgets::Button(i18nc("@action:button", "Get More Templates..."),
                                                 templateProvider.knsConfigurationFile(), d->ui->view);
    connect(getMoreButton, &KNSWidgets::Button::dialogFinished, this, [d](const QList<KNSCore::Entry>& changedEntries) {
        if (!d->viewHelper().handleNewStuffDialogFinished(changedEntries)) {
            d->makeFirstTemplateCurrent();
        }
    });
    d->ui->view->addWidget(0, getMoreButton);

    auto* loadButton = new QPushButton(QIcon::fromTheme(QStringLiteral("application-x-archive")), i18nc("@action:button", "Load Template from File"), d->ui->view);
    connect(loadButton, &QPushButton::clicked, this, [this] {
        Q_D(TemplateSelectionPage);
        if (!d->viewHelper().loadTemplatesFromFiles(this)) {
            d->makeFirstTemplateCurrent();
        }
    });
    d->ui->view->addWidget(0, loadButton);

    d->ui->view->setContentsMargins(0, 0, 0, 0);
}

TemplateSelectionPage::~TemplateSelectionPage()
{
    Q_D(TemplateSelectionPage);

    delete d->ui;
}

QSize TemplateSelectionPage::minimumSizeHint() const
{
    return QSize(400, 600);
}

QString TemplateSelectionPage::selectedTemplate() const
{
    Q_D(const TemplateSelectionPage);

    return d->selectedTemplate;
}

#include "moc_templateselectionpage.cpp"
