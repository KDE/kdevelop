/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "templateclassassistant.h"

#include "templateselectionpage.h"
#include "templateoptionspage.h"
#include "classmemberspage.h"
#include "classidentifierpage.h"
#include "overridespage.h"
#include "licensepage.h"
#include "outputpage.h"
#include "testcasespage.h"
#include "defaultcreateclasshelper.h"
#include "debug.h"

#include <language/codegen/templateclassgenerator.h>
#include <language/codegen/sourcefiletemplate.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/templaterenderer.h>
#include <language/interfaces/icreateclasshelper.h>
#include <language/interfaces/ilanguagesupport.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <util/scopeddialog.h>

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <KIO/Global>
#include <KLocalizedString>

#define REMOVE_PAGE(name)          \
if (d->name##Page)                 \
{                                  \
    removePage(d->name##Page);     \
    d->name##Page = nullptr;       \
    d->name##PageWidget = nullptr; \
}

#define ZERO_PAGE(name)  \
d->name##Page = nullptr; \
d->name##PageWidget = nullptr;

using namespace KDevelop;

class KDevelop::TemplateClassAssistantPrivate
{
public:
    explicit TemplateClassAssistantPrivate(const QUrl& baseUrl);
    ~TemplateClassAssistantPrivate();

    void addFilesToTarget (const QHash<QString, QUrl>& fileUrls);

    KPageWidgetItem* templateSelectionPage;
    KPageWidgetItem* classIdentifierPage;
    KPageWidgetItem* overridesPage;
    KPageWidgetItem* membersPage;
    KPageWidgetItem* testCasesPage;
    KPageWidgetItem* licensePage;
    KPageWidgetItem* templateOptionsPage;
    KPageWidgetItem* outputPage;
    KPageWidgetItem* dummyPage;

    TemplateSelectionPage* templateSelectionPageWidget;
    ClassIdentifierPage* classIdentifierPageWidget;
    OverridesPage* overridesPageWidget;
    ClassMembersPage* membersPageWidget;
    TestCasesPage* testCasesPageWidget;
    LicensePage* licensePageWidget;
    TemplateOptionsPage* templateOptionsPageWidget;
    OutputPage* outputPageWidget;

    QUrl baseUrl;
    SourceFileTemplate fileTemplate;
    ICreateClassHelper* helper;
    TemplateClassGenerator* generator;
    TemplateRenderer* renderer;

    QVariantHash templateOptions;
};

TemplateClassAssistantPrivate::TemplateClassAssistantPrivate(const QUrl& baseUrl)
: baseUrl(baseUrl)
, helper(nullptr)
, generator(nullptr)
, renderer(nullptr)
{
}

TemplateClassAssistantPrivate::~TemplateClassAssistantPrivate()
{
    delete helper;
    if (generator)
    {
        delete generator;
    }
    else
    {
        // if we got a generator, it should keep ownership of the renderer
        // otherwise, we created a templaterenderer on our own
        delete renderer;
    }
}

void TemplateClassAssistantPrivate::addFilesToTarget (const QHash< QString, QUrl >& fileUrls)
{
    // Add the generated files to a target, if one is found
    QUrl url = baseUrl;
    if (!url.isValid())
    {
        // This was probably not launched from the project manager view
        // Still, we try to find the common URL where the generated files are located

        if (!fileUrls.isEmpty())
        {
            url = fileUrls.constBegin().value().adjusted(QUrl::RemoveFilename);
        }
    }
    qCDebug(PLUGIN_FILETEMPLATES) << "Searching for targets with URL" << url;
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    if (!project || !project->buildSystemManager())
    {
        qCDebug(PLUGIN_FILETEMPLATES) << "No suitable project found";
        return;
    }

    const QList<ProjectBaseItem*> items = project->itemsForPath(IndexedString(url));
    if (items.isEmpty())
    {
        qCDebug(PLUGIN_FILETEMPLATES) << "No suitable project items found";
        return;
    }

    QList<ProjectTargetItem*> targets;
    ProjectTargetItem* target = nullptr;

    for (ProjectBaseItem* item : items) {
        if (ProjectTargetItem* target = item->target())
        {
            targets << target;
        }
    }

    if (targets.isEmpty())
    {
        // If no target was explicitly found yet, try all the targets in the current folder
        targets.reserve(items.size());
        for (ProjectBaseItem* item : items) {
            targets << item->targetList();
        }
    }

    if (targets.isEmpty())
    {
        // If still no targets, we traverse the tree up to the first directory with targets
        ProjectBaseItem* item = items.first()->parent();
        while (targets.isEmpty() && item)
        {
            targets = item->targetList();
            item = item->parent();
        }
    }

    if (targets.size() == 1)
    {
        qCDebug(PLUGIN_FILETEMPLATES) << "Only one candidate target," << targets.first()->text() << ", using it";
        target = targets.first();
    }
    else if (targets.size() > 1)
    {
        // More than one candidate target, show the chooser dialog
        ScopedDialog<QDialog> d;

        auto mainLayout = new QVBoxLayout(d);
        mainLayout->addWidget(new QLabel(i18n("Choose one target to add the file or cancel if you do not want to do so.")));

        auto* targetsWidget = new QListWidget(d);
        targetsWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        for (ProjectTargetItem* target : std::as_const(targets)) {
            targetsWidget->addItem(target->text());
        }
        targetsWidget->setCurrentRow(0);
        mainLayout->addWidget(targetsWidget);

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        d->connect(buttonBox, &QDialogButtonBox::accepted, d.data(), &QDialog::accept);
        d->connect(buttonBox, &QDialogButtonBox::rejected, d.data(), &QDialog::reject);
        mainLayout->addWidget(buttonBox);

        if(d->exec() == QDialog::Accepted)
        {
            if (!targetsWidget->selectedItems().isEmpty())
            {
                target = targets[targetsWidget->currentRow()];
            }
            else
            {
                qCDebug(PLUGIN_FILETEMPLATES) << "Did not select anything, not adding to a target";
                return;
            }
        }
        else {
            qCDebug(PLUGIN_FILETEMPLATES) << "Canceled select target dialog, not adding to a target";
            return;
        }
    }
    else
    {
        // No target, not doing anything
        qCDebug(PLUGIN_FILETEMPLATES) << "No possible targets for URL" << url;
        return;
    }

    Q_ASSERT(target);

    QList<ProjectFileItem*> fileItems;
    for (const QUrl& fileUrl : fileUrls) {
        const auto items = project->itemsForPath(IndexedString(KIO::upUrl(fileUrl)));
        for (ProjectBaseItem* item : items) {
            if (ProjectFolderItem* folder = item->folder())
            {
                ///FIXME: use Path instead of QUrl in the template class assistant
                if (ProjectFileItem* file = project->projectFileManager()->addFile(Path(fileUrl), folder)) {
                    fileItems << file;
                    break;
                }
            }
        }
    }

    if (!fileItems.isEmpty()) {
        project->buildSystemManager()->addFilesToTarget(fileItems, target);
    }

}

TemplateClassAssistant::TemplateClassAssistant(ITemplateProvider& templateProvider, const QUrl& baseUrl,
                                               QWidget* parent)
    : KAssistantDialog(parent)
    , d(new TemplateClassAssistantPrivate(baseUrl))
{
    ZERO_PAGE(templateSelection)
    ZERO_PAGE(templateOptions)
    ZERO_PAGE(members)
    ZERO_PAGE(classIdentifier)
    ZERO_PAGE(overrides)
    ZERO_PAGE(license)
    ZERO_PAGE(output)
    ZERO_PAGE(testCases)

    setup(templateProvider);
}

TemplateClassAssistant::~TemplateClassAssistant()
{
    delete d;
}

void TemplateClassAssistant::setup(ITemplateProvider& templateProvider)
{
    if (d->baseUrl.isValid())
    {
        setWindowTitle(xi18nc("@title:window", "Create Files from Template in <filename>%1</filename>", d->baseUrl.toDisplayString(QUrl::PreferLocalFile)));
    }
    else
    {
        setWindowTitle(i18nc("@title:window", "Create Files from Template"));
    }

    d->templateSelectionPageWidget = new TemplateSelectionPage(templateProvider, this);
    connect(this, &TemplateClassAssistant::accepted, d->templateSelectionPageWidget, &TemplateSelectionPage::saveConfig);
    d->templateSelectionPage = addPage(d->templateSelectionPageWidget, i18nc("@title:tab", "Language and Template"));
    d->templateSelectionPage->setIcon(QIcon::fromTheme(QStringLiteral("project-development-new-template")));

    d->dummyPage = addPage(new QWidget(this), QStringLiteral("Dummy Page"));

    // KAssistantDialog creates a help button by default, no option to prevent that
    QPushButton *helpButton = button(QDialogButtonBox::Help);
    if (helpButton) {
        buttonBox()->removeButton(helpButton);
        delete helpButton;
    }
}

void TemplateClassAssistant::templateChosen(const QString& templateDescription)
{
    d->fileTemplate.setTemplateDescription(templateDescription);
    const auto type = d->fileTemplate.type();

    d->generator = nullptr;

    if (!d->fileTemplate.isValid())
    {
        return;
    }

    qCDebug(PLUGIN_FILETEMPLATES) << "Selected template" << templateDescription << "of type" << type;
    removePage(d->dummyPage);

    if (d->baseUrl.isValid())
    {
        setWindowTitle(xi18nc("@title:window", "Create Files from Template <filename>%1</filename> in <filename>%2</filename>",
                            d->fileTemplate.name(),
                            d->baseUrl.toDisplayString(QUrl::PreferLocalFile)));
    }
    else
    {
        setWindowTitle(xi18nc("@title:window", "Create Files from Template <filename>%1</filename>", d->fileTemplate.name()));
    }

    if (type == QLatin1String("Class"))
    {
        d->classIdentifierPageWidget = new ClassIdentifierPage(this);
        d->classIdentifierPage = addPage(d->classIdentifierPageWidget, i18nc("@title:tab", "Class Basics"));
        d->classIdentifierPage->setIcon(QIcon::fromTheme(QStringLiteral("classnew")));
        connect(d->classIdentifierPageWidget, &ClassIdentifierPage::isValid, this, &TemplateClassAssistant::setCurrentPageValid);
        setValid(d->classIdentifierPage, false);

        d->overridesPageWidget = new OverridesPage(this);
        d->overridesPage = addPage(d->overridesPageWidget, i18nc("@title:tab", "Override Methods"));
        d->overridesPage->setIcon(QIcon::fromTheme(QStringLiteral("code-class")));
        setValid(d->overridesPage, true);

        d->membersPageWidget = new ClassMembersPage(this);
        d->membersPage = addPage(d->membersPageWidget, i18nc("@title:tab", "Class Members"));
        d->membersPage->setIcon(QIcon::fromTheme(QStringLiteral("field")));
        setValid(d->membersPage, true);

        d->helper = nullptr;
        QString languageName = d->fileTemplate.languageName();
        auto language = ICore::self()->languageController()->language(languageName);
        if (language)
        {
            d->helper = language->createClassHelper();
        }

        if (!d->helper)
        {
            qCDebug(PLUGIN_FILETEMPLATES) << "No class creation helper for language" << languageName;
            d->helper = new DefaultCreateClassHelper;
        }

        d->generator = d->helper->createGenerator(d->baseUrl);
        Q_ASSERT(d->generator);
        d->generator->setTemplateDescription(d->fileTemplate);
        d->renderer = d->generator->renderer();
    }
    else
    {
        if (type == QLatin1String("Test"))
        {
            d->testCasesPageWidget = new TestCasesPage(this);
            d->testCasesPage = addPage(d->testCasesPageWidget, i18nc("@title:tab", "Test Cases"));
            connect(d->testCasesPageWidget, &TestCasesPage::isValid, this, &TemplateClassAssistant::setCurrentPageValid);
            setValid(d->testCasesPage, false);
        }

        d->renderer = new TemplateRenderer;
        d->renderer->setEmptyLinesPolicy(TemplateRenderer::TrimEmptyLines);
    }

    d->licensePageWidget = new LicensePage(this);
    d->licensePage = addPage(d->licensePageWidget, i18nc("@title:tab", "License"));
    d->licensePage->setIcon(QIcon::fromTheme(QStringLiteral("text-x-copying")));
    setValid(d->licensePage, true);

    d->outputPageWidget = new OutputPage(this);
    d->outputPageWidget->prepareForm(d->fileTemplate);
    d->outputPage = addPage(d->outputPageWidget, i18nc("@title:tab", "Output"));
    d->outputPage->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    connect(d->outputPageWidget, &OutputPage::isValid, this, &TemplateClassAssistant::setCurrentPageValid);
    setValid(d->outputPage, false);

    if (d->fileTemplate.hasCustomOptions())
    {
        qCDebug(PLUGIN_FILETEMPLATES) << "Class generator has custom options";
        d->templateOptionsPageWidget = new TemplateOptionsPage(this);
        d->templateOptionsPage = insertPage(d->outputPage, d->templateOptionsPageWidget,
                                            i18nc("@title:tab", "Template Options"));
    }

    setCurrentPage(d->templateSelectionPage);
}

void TemplateClassAssistant::next()
{
    qCDebug(PLUGIN_FILETEMPLATES) << currentPage()->name() << currentPage()->header();
    if (currentPage() == d->templateSelectionPage)
    {
        // We have chosen the template
        // Depending on the template's language, we can now create a helper
        QString description = d->templateSelectionPageWidget->selectedTemplate();
        templateChosen(description);
        if (!d->fileTemplate.isValid())
        {
            return;
        }
    }
    else if (currentPage() == d->classIdentifierPage)
    {
        d->generator->setIdentifier(d->classIdentifierPageWidget->identifier());
        d->generator->setBaseClasses(d->classIdentifierPageWidget->inheritanceList());
    }
    else if (currentPage() == d->overridesPage)
    {
        ClassDescription desc = d->generator->description();
        desc.methods.clear();
        const auto overrides = d->overridesPageWidget->selectedOverrides();
        desc.methods.reserve(overrides.size());
        for (const DeclarationPointer& declaration : overrides) {
            desc.methods << FunctionDescription(declaration);
        }
        d->generator->setDescription(desc);
    }
    else if (currentPage() == d->membersPage)
    {
        ClassDescription desc = d->generator->description();
        desc.members = d->membersPageWidget->members();
        d->generator->setDescription(desc);
    }
    else if (currentPage() == d->licensePage)
    {
        if (d->generator)
        {
            d->generator->setLicense(d->licensePageWidget->license());
        }
        else
        {
            d->renderer->addVariable(QStringLiteral("license"), d->licensePageWidget->license());
        }
    }
    else if (d->templateOptionsPage && (currentPage() == d->templateOptionsPage))
    {
        if (d->generator)
        {
            d->generator->addVariables(d->templateOptionsPageWidget->templateOptions());
        }
        else
        {
            d->renderer->addVariables(d->templateOptionsPageWidget->templateOptions());
        }
    }
    else if (currentPage() == d->testCasesPage)
    {
        d->renderer->addVariable(QStringLiteral("name"), d->testCasesPageWidget->name());
        d->renderer->addVariable(QStringLiteral("testCases"), d->testCasesPageWidget->testCases());
    }

    KAssistantDialog::next();

    if (currentPage() == d->classIdentifierPage)
    {
        d->classIdentifierPageWidget->setInheritanceList(d->fileTemplate.defaultBaseClasses());
    }
    else if (currentPage() == d->membersPage)
    {
        d->membersPageWidget->setMembers(d->generator->description().members);
    }
    else if (currentPage() == d->overridesPage)
    {
        d->overridesPageWidget->clear();
        d->overridesPageWidget->addCustomDeclarations(i18nc("@item default declarations", "Default"),
            d->helper->defaultMethods(d->generator->name()));
        d->overridesPageWidget->addBaseClasses(d->generator->directBaseClasses(),
                                               d->generator->allBaseClasses());
    }
    else if (d->templateOptionsPage && (currentPage() == d->templateOptionsPage))
    {
        d->templateOptionsPageWidget->load(d->fileTemplate, d->renderer);
    }
    else if (currentPage() == d->outputPage)
    {
        d->outputPageWidget->loadFileTemplate(d->fileTemplate, d->baseUrl, d->renderer);
    }

    if (auto* pageFocus = dynamic_cast<KDevelop::IPageFocus*>(currentPage()->widget())) {
        pageFocus->setFocusToFirstEditWidget();
    }
}

void TemplateClassAssistant::back()
{
    KAssistantDialog::back();
    if (currentPage() == d->templateSelectionPage)
    {
        REMOVE_PAGE(classIdentifier)
        REMOVE_PAGE(overrides)
        REMOVE_PAGE(members)
        REMOVE_PAGE(testCases)
        REMOVE_PAGE(output)
        REMOVE_PAGE(templateOptions)
        REMOVE_PAGE(license)

        delete d->helper;
        d->helper = nullptr;

        if (d->generator)
        {
            delete d->generator;
        }
        else
        {
            delete d->renderer;
        }
        d->generator = nullptr;
        d->renderer = nullptr;

        if (d->baseUrl.isValid())
        {
            setWindowTitle(xi18nc("@title:window", "Create Files from Template in <filename>%1</filename>", d->baseUrl.toDisplayString(QUrl::PreferLocalFile)));
        }
        else
        {
            setWindowTitle(i18nc("@title:window", "Create Files from Template"));
        }
        d->dummyPage = addPage(new QWidget(this), QStringLiteral("Dummy Page"));
    }
}

void TemplateClassAssistant::accept()
{
    // next() is not called for the last page (when the user clicks Finish), so we have to set output locations here
    const QHash<QString, QUrl> fileUrls = d->outputPageWidget->fileUrls();
    QHash<QString, KTextEditor::Cursor> filePositions = d->outputPageWidget->filePositions();

    DocumentChangeSet changes;
    if (d->generator)
    {
        QHash<QString, QUrl>::const_iterator it = fileUrls.constBegin();
        for (; it != fileUrls.constEnd(); ++it)
        {
            d->generator->setFileUrl(it.key(), it.value());
            d->generator->setFilePosition(it.key(), filePositions.value(it.key()));
        }

        d->generator->addVariables(d->templateOptions);
        changes = d->generator->generate();
    }
    else
    {
        changes = d->renderer->renderFileTemplate(d->fileTemplate, d->baseUrl, fileUrls);
    }

    d->addFilesToTarget(fileUrls);
    changes.applyAllChanges();

    // Open the generated files in the editor
    for (const QUrl& url : fileUrls) {
        ICore::self()->documentController()->openDocument(url);
    }

    KAssistantDialog::accept();
}

void TemplateClassAssistant::setCurrentPageValid(bool valid)
{
    setValid(currentPage(), valid);
}

QUrl TemplateClassAssistant::baseUrl() const
{
    return d->baseUrl;
}

#include "moc_templateclassassistant.cpp"
