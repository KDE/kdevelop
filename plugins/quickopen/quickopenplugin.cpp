/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "quickopenplugin.h"

#include "quickopenwidget.h"

#include <cassert>
#include <typeinfo>
#include <QKeyEvent>
#include <QApplication>
#include <QMetaObject>
#include <QScreen>
#include <QWidgetAction>
#include <QAction>

#include <KLocalizedString>
#include <KPluginFactory>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KParts/MainWindow>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KActionCollection>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/types/identifiedtype.h>
#include <serialization/indexedstring.h>
#include <language/duchain/types/functiontype.h>

#include "quickopenmodel.h"
#include "projectfilequickopen.h"
#include "projectitemquickopen.h"
#include "declarationlistquickopen.h"
#include "documentationquickopenprovider.h"
#include "actionsquickopenprovider.h"
#include "debug.h"
#include <language/duchain/functiondefinition.h>
#include <interfaces/contextmenuextension.h>
#include <language/interfaces/codecontext.h>


using namespace KDevelop;

const bool noHtmlDestriptionInOutline = true;

namespace {
namespace Strings {
QString scopeProjectName()       { return i18nc("@item quick open scope", "Project"); }
QString scopeIncludesName()      { return i18nc("@item quick open scope", "Includes"); }
QString scopeIncludersName()     { return i18nc("@item quick open scope", "Includers"); }
QString scopeCurrentlyOpenName() { return i18nc("@item quick open scope", "Currently Open"); }

QString typeFilesName()         { return i18nc("@item quick open item type", "Files"); }
QString typeFunctionsName()     { return i18nc("@item quick open item type", "Functions"); }
QString typeClassesName()       { return i18nc("@item quick open item type", "Classes"); }
QString typeDocumentationName() { return i18nc("@item quick open item type", "Documentation"); }
QString typeActionsName()       { return i18nc("@item quick open item type", "Actions"); }

}
}


class QuickOpenWidgetCreator
{
public:
    virtual ~QuickOpenWidgetCreator()
    {
    }
    virtual QuickOpenWidget* createWidget() = 0;
    virtual QString objectNameForLine() = 0;
    virtual void widgetShown()
    {
    }
};

class StandardQuickOpenWidgetCreator
    : public QuickOpenWidgetCreator
{
public:
    StandardQuickOpenWidgetCreator(const QStringList& items, const QStringList& scopes)
        : m_items(items)
        , m_scopes(scopes)
    {
    }

    QString objectNameForLine() override
    {
        return QStringLiteral("Quickopen");
    }

    void setItems(const QStringList& scopes, const QStringList& items)
    {
        m_scopes = scopes;
        m_items = items;
    }

    QuickOpenWidget* createWidget() override
    {
        QStringList useItems = m_items;
        if (useItems.isEmpty()) {
            useItems = QuickOpenPlugin::self()->lastUsedItems;
        }

        QStringList useScopes = m_scopes;
        if (useScopes.isEmpty()) {
            useScopes = QuickOpenPlugin::self()->lastUsedScopes;
        }

        return new QuickOpenWidget(QuickOpenPlugin::self()->m_model, QuickOpenPlugin::self()->lastUsedItems, useScopes, false, true);
    }

    QStringList m_items;
    QStringList m_scopes;
};

class OutlineFilter
    : public DUChainUtils::DUChainItemFilter
{
public:
    enum OutlineMode { Functions, FunctionsAndClasses };
    explicit OutlineFilter(QVector<DUChainItem>& _items, OutlineMode _mode = FunctionsAndClasses) : items(_items)
        , mode(_mode)
    {
    }
    bool accept(Declaration* decl) override
    {
        if (decl->range().isEmpty()) {
            return false;
        }
        bool collectable = mode == Functions ? decl->isFunctionDeclaration() : (decl->isFunctionDeclaration() || (decl->internalContext() && decl->internalContext()->type() == DUContext::Class));
        if (collectable) {
            DUChainItem item;
            item.m_item = IndexedDeclaration(decl);
            item.m_text = decl->toString();
            items << item;

            return true;
        } else {
            return false;
        }
    }
    bool accept(DUContext* ctx) override
    {
        if (ctx->type() == DUContext::Class || ctx->type() == DUContext::Namespace || ctx->type() == DUContext::Global || ctx->type() == DUContext::Other || ctx->type() == DUContext::Helper) {
            return true;
        } else {
            return false;
        }
    }
    QVector<DUChainItem>& items;
    OutlineMode mode;
};

K_PLUGIN_FACTORY_WITH_JSON(KDevQuickOpenFactory, "kdevquickopen.json", registerPlugin<QuickOpenPlugin>(); )

Declaration * cursorDeclaration() {
    KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
    if (!view) {
        return nullptr;
    }

    KDevelop::DUChainReadLocker lock(DUChain::lock());

    return DUChainUtils::declarationForDefinition(DUChainUtils::itemUnderCursor(view->document()->url(), KTextEditor::Cursor(view->cursorPosition())).declaration);
}

///The first definition that belongs to a context that surrounds the current cursor
Declaration* cursorContextDeclaration()
{
    KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
    if (!view) {
        return nullptr;
    }

    KDevelop::DUChainReadLocker lock(DUChain::lock());

    TopDUContext* ctx = DUChainUtils::standardContextForUrl(view->document()->url());
    if (!ctx) {
        return nullptr;
    }

    KTextEditor::Cursor cursor(view->cursorPosition());

    DUContext* subCtx = ctx->findContext(ctx->transformToLocalRevision(cursor));

    while (subCtx && !subCtx->owner())
        subCtx = subCtx->parentContext();

    Declaration* definition = nullptr;

    if (!subCtx || !subCtx->owner()) {
        definition = DUChainUtils::declarationInLine(cursor, ctx);
    } else {
        definition = subCtx->owner();
    }

    if (!definition) {
        return nullptr;
    }

    return definition;
}

//Returns only the name, no template-parameters or scope
QString cursorItemText()
{
    KDevelop::DUChainReadLocker lock(DUChain::lock());

    Declaration* decl = cursorDeclaration();
    if (!decl) {
        return QString();
    }

    IDocument* doc = ICore::self()->documentController()->activeDocument();
    if (!doc) {
        return QString();
    }

    TopDUContext* context = DUChainUtils::standardContextForUrl(doc->url());

    if (!context) {
        qCDebug(PLUGIN_QUICKOPEN) << "Got no standard context";
        return QString();
    }

    AbstractType::Ptr t = decl->abstractType();
    auto* idType = dynamic_cast<IdentifiedType*>(t.data());
    if (idType && idType->declaration(context)) {
        decl = idType->declaration(context);
    }

    if (!decl->qualifiedIdentifier().isEmpty()) {
        return decl->qualifiedIdentifier().last().identifier().str();
    }

    return QString();
}

QuickOpenLineEdit* QuickOpenPlugin::createQuickOpenLineWidget()
{
    return new QuickOpenLineEdit(new StandardQuickOpenWidgetCreator(QStringList(), QStringList()));
}

QuickOpenLineEdit* QuickOpenPlugin::quickOpenLine(const QString& name)
{
    const QList<QuickOpenLineEdit*> lines = ICore::self()->uiController()->activeMainWindow()->findChildren<QuickOpenLineEdit*>(name);
    for (QuickOpenLineEdit* line : lines) {
        if (line->isVisible()) {
            return line;
        }
    }

    return nullptr;
}

static QuickOpenPlugin* staticQuickOpenPlugin = nullptr;

QuickOpenPlugin* QuickOpenPlugin::self()
{
    return staticQuickOpenPlugin;
}

void QuickOpenPlugin::createActionsForMainWindow(Sublime::MainWindow* /*window*/, QString& xmlFile, KActionCollection& actions)
{
    xmlFile = QStringLiteral("kdevquickopen.rc");

    QAction* quickOpen = actions.addAction(QStringLiteral("quick_open"));
    quickOpen->setText(i18nc("@action", "&Quick Open"));
    quickOpen->setIcon(QIcon::fromTheme(QStringLiteral("quickopen")));
    actions.setDefaultShortcut(quickOpen, Qt::CTRL | Qt::ALT | Qt::Key_Q);
    connect(quickOpen, &QAction::triggered, this, &QuickOpenPlugin::quickOpen);

    QAction* quickOpenFile = actions.addAction(QStringLiteral("quick_open_file"));
    quickOpenFile->setText(i18nc("@action", "Quick Open &File"));
    quickOpenFile->setIcon(QIcon::fromTheme(QStringLiteral("quickopen-file")));
    actions.setDefaultShortcut(quickOpenFile, Qt::CTRL | Qt::ALT | Qt::Key_O);
    connect(quickOpenFile, &QAction::triggered, this, &QuickOpenPlugin::quickOpenFile);

    QAction* quickOpenClass = actions.addAction(QStringLiteral("quick_open_class"));
    quickOpenClass->setText(i18nc("@action", "Quick Open &Class"));
    quickOpenClass->setIcon(QIcon::fromTheme(QStringLiteral("quickopen-class")));
    actions.setDefaultShortcut(quickOpenClass, Qt::CTRL | Qt::ALT | Qt::Key_C);
    connect(quickOpenClass, &QAction::triggered, this, &QuickOpenPlugin::quickOpenClass);

    QAction* quickOpenFunction = actions.addAction(QStringLiteral("quick_open_function"));
    quickOpenFunction->setText(i18nc("@action", "Quick Open &Function"));
    quickOpenFunction->setIcon(QIcon::fromTheme(QStringLiteral("quickopen-function")));
    actions.setDefaultShortcut(quickOpenFunction, Qt::CTRL | Qt::ALT | Qt::Key_M);
    connect(quickOpenFunction, &QAction::triggered, this, &QuickOpenPlugin::quickOpenFunction);

    QAction* quickOpenAlreadyOpen = actions.addAction(QStringLiteral("quick_open_already_open"));
    quickOpenAlreadyOpen->setText(i18nc("@action", "Quick Open &Already Open File"));
    quickOpenAlreadyOpen->setIcon(QIcon::fromTheme(QStringLiteral("quickopen-file")));
    connect(quickOpenAlreadyOpen, &QAction::triggered, this, &QuickOpenPlugin::quickOpenOpenFile);

    QAction* quickOpenDocumentation = actions.addAction(QStringLiteral("quick_open_documentation"));
    quickOpenDocumentation->setText(i18nc("@action", "Quick Open &Documentation"));
    quickOpenDocumentation->setIcon(QIcon::fromTheme(QStringLiteral("quickopen-documentation")));
    actions.setDefaultShortcut(quickOpenDocumentation, Qt::CTRL | Qt::ALT | Qt::Key_D);
    connect(quickOpenDocumentation, &QAction::triggered, this, &QuickOpenPlugin::quickOpenDocumentation);

    QAction* quickOpenActions = actions.addAction(QStringLiteral("quick_open_actions"));
    quickOpenActions->setText(i18nc("@action", "Quick Open &Actions"));
    actions.setDefaultShortcut(quickOpenActions, Qt::CTRL | Qt::ALT | Qt::Key_A);
    connect(quickOpenActions, &QAction::triggered, this, &QuickOpenPlugin::quickOpenActions);

    m_quickOpenDeclaration = actions.addAction(QStringLiteral("quick_open_jump_declaration"));
    m_quickOpenDeclaration->setText(i18nc("@action", "Jump to Declaration"));
    m_quickOpenDeclaration->setIcon(QIcon::fromTheme(QStringLiteral("go-jump-declaration")));
    actions.setDefaultShortcut(m_quickOpenDeclaration, Qt::CTRL | Qt::Key_Period);
    connect(m_quickOpenDeclaration, &QAction::triggered, this, &QuickOpenPlugin::quickOpenDeclaration, Qt::QueuedConnection);

    m_quickOpenDefinition = actions.addAction(QStringLiteral("quick_open_jump_definition"));
    m_quickOpenDefinition->setText(i18nc("@action", "Jump to Definition"));
    m_quickOpenDefinition->setIcon(QIcon::fromTheme(QStringLiteral("go-jump-definition")));
    connect(m_quickOpenDefinition, &QAction::triggered, this, &QuickOpenPlugin::quickOpenDefinition, Qt::QueuedConnection);

    auto* quickOpenLine = new QWidgetAction(this);
    quickOpenLine->setText(i18nc("@action", "Embedded Quick Open"));
    //     actions.setDefaultShortcut( quickOpenLine, Qt::CTRL | Qt::ALT | Qt::Key_E );
//     connect(quickOpenLine, SIGNAL(triggered(bool)), this, SLOT(quickOpenLine(bool)));
    quickOpenLine->setDefaultWidget(createQuickOpenLineWidget());
    actions.addAction(QStringLiteral("quick_open_line"), quickOpenLine);

    QAction* quickOpenNextFunction = actions.addAction(QStringLiteral("quick_open_next_function"));
    quickOpenNextFunction->setText(i18nc("@action jump to", "Next Function"));
    actions.setDefaultShortcut(quickOpenNextFunction, Qt::CTRL | Qt::ALT | Qt::Key_PageDown);
    connect(quickOpenNextFunction, &QAction::triggered, this, &QuickOpenPlugin::nextFunction);

    QAction* quickOpenPrevFunction = actions.addAction(QStringLiteral("quick_open_prev_function"));
    quickOpenPrevFunction->setText(i18nc("@action jump to", "Previous Function"));
    actions.setDefaultShortcut(quickOpenPrevFunction, Qt::CTRL | Qt::ALT | Qt::Key_PageUp);
    connect(quickOpenPrevFunction, &QAction::triggered, this, &QuickOpenPlugin::previousFunction);

    QAction* quickOpenNavigateFunctions = actions.addAction(QStringLiteral("quick_open_outline"));
    quickOpenNavigateFunctions->setText(i18nc("@action open outline quick open menu", "Outline"));
    actions.setDefaultShortcut(quickOpenNavigateFunctions, Qt::CTRL | Qt::ALT | Qt::Key_N);
    connect(quickOpenNavigateFunctions, &QAction::triggered, this, &QuickOpenPlugin::quickOpenNavigateFunctions);
}

QuickOpenPlugin::QuickOpenPlugin(QObject* parent,
                                 const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevquickopen"), parent)
{
    staticQuickOpenPlugin = this;
    m_model = new QuickOpenModel(nullptr);

    KConfigGroup quickopengrp = KSharedConfig::openConfig()->group(QStringLiteral("QuickOpen"));
    lastUsedScopes = quickopengrp.readEntry("SelectedScopes", QStringList{
        Strings::scopeProjectName(),
        Strings::scopeIncludesName(),
        Strings::scopeIncludersName(),
        Strings::scopeCurrentlyOpenName(),
    });
    lastUsedItems = quickopengrp.readEntry("SelectedItems", QStringList());

    {
        m_openFilesData = new OpenFilesDataProvider();
        const QStringList scopes(Strings::scopeCurrentlyOpenName());
        const QStringList types(Strings::typeFilesName());
        m_model->registerProvider(scopes, types, m_openFilesData);
    }
    {
        m_projectFileData = new ProjectFileDataProvider();
        const QStringList scopes(Strings::scopeProjectName());
        const QStringList types(Strings::typeFilesName());
        m_model->registerProvider(scopes, types, m_projectFileData);
    }
    {
        m_projectItemData = new ProjectItemDataProvider(this);
        const QStringList scopes(Strings::scopeProjectName());
        const QStringList types = ProjectItemDataProvider::supportedItemTypes();
        m_model->registerProvider(scopes, types, m_projectItemData);
    }
    {
        m_documentationItemData = new DocumentationQuickOpenProvider;
        const QStringList scopes(Strings::scopeIncludesName());
        const QStringList types(Strings::typeDocumentationName());
        m_model->registerProvider(scopes, types, m_documentationItemData);
    }
    {
        m_actionsItemData = new ActionsQuickOpenProvider;
        const QStringList scopes(Strings::scopeIncludesName());
        const QStringList types(Strings::typeActionsName());
        m_model->registerProvider(scopes, types, m_actionsItemData);
    }
}

QuickOpenPlugin::~QuickOpenPlugin()
{
    freeModel();

    delete m_model;
    delete m_projectFileData;
    delete m_projectItemData;
    delete m_openFilesData;
    delete m_documentationItemData;
    delete m_actionsItemData;
}

void QuickOpenPlugin::unload()
{
}

ContextMenuExtension QuickOpenPlugin::contextMenuExtension(Context* context, QWidget* parent)
{
    KDevelop::ContextMenuExtension menuExt = KDevelop::IPlugin::contextMenuExtension(context, parent);

    auto* codeContext = dynamic_cast<KDevelop::DeclarationContext*>(context);

    if (!codeContext) {
        return menuExt;
    }

    DUChainReadLocker readLock;
    Declaration* decl(codeContext->declaration().data());

    if (decl) {
        const bool isDef = FunctionDefinition::definition(decl);
        if (codeContext->use().isValid() || !isDef) {
            menuExt.addAction(KDevelop::ContextMenuExtension::NavigationGroup, m_quickOpenDeclaration);
        }

        if (isDef) {
            menuExt.addAction(KDevelop::ContextMenuExtension::NavigationGroup, m_quickOpenDefinition);
        }
    }

    return menuExt;
}

void QuickOpenPlugin::showQuickOpen(const QStringList& items)
{
    if (!freeModel()) {
        return;
    }

    QStringList initialItems = items;

    QStringList useScopes = lastUsedScopes;

    const QString scopeCurrentlyOpenName = Strings::scopeCurrentlyOpenName();
    if (!useScopes.contains(scopeCurrentlyOpenName)) {
        useScopes << scopeCurrentlyOpenName;
    }

    showQuickOpenWidget(initialItems, useScopes, false);
}

void QuickOpenPlugin::showQuickOpen(ModelTypes modes)
{
    if (!freeModel()) {
        return;
    }

    QStringList initialItems;
    if (modes & Files || modes & OpenFiles) {
        initialItems << Strings::typeFilesName();
    }

    if (modes & Functions) {
        initialItems << Strings::typeFunctionsName();
    }

    if (modes & Classes) {
        initialItems << Strings::typeClassesName();
    }

    QStringList useScopes;
    if (modes != OpenFiles) {
        useScopes = lastUsedScopes;
    }

    if ((modes & OpenFiles)) {
        const QString currentlyOpen = Strings::scopeCurrentlyOpenName();
        if (!useScopes.contains(currentlyOpen)) {
            useScopes << currentlyOpen;
        }
    }

    bool preselectText = (!(modes & Files) || modes == QuickOpenPlugin::All);
    showQuickOpenWidget(initialItems, useScopes, preselectText);
}

void QuickOpenPlugin::showQuickOpenWidget(const QStringList& items, const QStringList& scopes, bool preselectText)
{
    auto* dialog = new QuickOpenWidgetDialog(i18nc("@title:window", "Quick Open"), m_model, items, scopes);
    m_currentWidgetHandler = dialog;
    if (preselectText) {
        KDevelop::IDocument* currentDoc = core()->documentController()->activeDocument();
        if (currentDoc && currentDoc->textDocument()) {
            QString preselected = currentDoc->textSelection().isEmpty() ? currentDoc->textWord() : currentDoc->textDocument()->text(currentDoc->textSelection());
            dialog->widget()->setPreselectedText(preselected);
        }
    }

    connect(dialog->widget(), &QuickOpenWidget::scopesChanged, this, &QuickOpenPlugin::storeScopes);
    //Not connecting itemsChanged to storeItems, as showQuickOpen doesn't use lastUsedItems and so shouldn't store item changes
    //connect( dialog->widget(), SIGNAL(itemsChanged(QStringList)), this, SLOT(storeItems(QStringList)) );
    dialog->widget()->ui.itemsButton->setEnabled(false);

    if (quickOpenLine()) {
        quickOpenLine()->showWithWidget(dialog->widget());
        dialog->deleteLater();
    } else {
        dialog->run();
    }
}

void QuickOpenPlugin::storeScopes(const QStringList& scopes)
{
    lastUsedScopes = scopes;
    KConfigGroup grp = KSharedConfig::openConfig()->group(QStringLiteral("QuickOpen"));
    grp.writeEntry("SelectedScopes", scopes);
}

void QuickOpenPlugin::storeItems(const QStringList& items)
{
    lastUsedItems = items;
    KConfigGroup grp = KSharedConfig::openConfig()->group(QStringLiteral("QuickOpen"));
    grp.writeEntry("SelectedItems", items);
}

void QuickOpenPlugin::quickOpen()
{
    if (quickOpenLine()) { //Same as clicking on Quick Open
        quickOpenLine()->setFocus();
    } else {
        showQuickOpen(All);
    }
}

void QuickOpenPlugin::quickOpenFile()
{
    showQuickOpen(( ModelTypes )(Files | OpenFiles));
}

void QuickOpenPlugin::quickOpenFunction()
{
    showQuickOpen(Functions);
}

void QuickOpenPlugin::quickOpenClass()
{
    showQuickOpen(Classes);
}

void QuickOpenPlugin::quickOpenOpenFile()
{
    showQuickOpen(OpenFiles);
}

void QuickOpenPlugin::quickOpenDocumentation()
{
    const QStringList scopes(Strings::scopeIncludesName());
    const QStringList types(Strings::typeDocumentationName());
    showQuickOpenWidget(types, scopes, true);
}

void QuickOpenPlugin::quickOpenActions()
{
    const QStringList scopes(Strings::scopeIncludesName());
    const QStringList types(Strings::typeActionsName());
    showQuickOpenWidget(types, scopes, true);
}

QSet<KDevelop::IndexedString> QuickOpenPlugin::fileSet() const
{
    return m_model->fileSet();
}

void QuickOpenPlugin::registerProvider(const QStringList& scope, const QStringList& type, KDevelop::QuickOpenDataProviderBase* provider)
{
    m_model->registerProvider(scope, type, provider);
}

bool QuickOpenPlugin::removeProvider(KDevelop::QuickOpenDataProviderBase* provider)
{
    m_model->removeProvider(provider);
    return true;
}

void QuickOpenPlugin::quickOpenDeclaration()
{
    if (jumpToSpecialObject()) {
        return;
    }

    KDevelop::DUChainReadLocker lock(DUChain::lock());
    Declaration* decl = cursorDeclaration();

    if (!decl) {
        qCDebug(PLUGIN_QUICKOPEN) << "Found no declaration for cursor, cannot jump";
        return;
    }
    decl->activateSpecialization();

    IndexedString u = decl->url();
    KTextEditor::Cursor c = decl->rangeInCurrentRevision().start();

    if (u.isEmpty()) {
        qCDebug(PLUGIN_QUICKOPEN) << "Got empty url for declaration" << decl->toString();
        return;
    }

    lock.unlock();
    core()->documentController()->openDocument(u.toUrl(), c);
}

QWidget* QuickOpenPlugin::specialObjectNavigationWidget() const
{
    KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
    if (!view) {
        return nullptr;
    }

    QUrl url = ICore::self()->documentController()->activeDocument()->url();

    const auto languages = ICore::self()->languageController()->languagesForUrl(url);
    for (const auto language : languages) {
        QWidget* w = language->specialLanguageObjectNavigationWidget(url, view->cursorPosition()).first;
        if (w) {
            return w;
        }
    }

    return nullptr;
}

QPair<QUrl, KTextEditor::Cursor> QuickOpenPlugin::specialObjectJumpPosition() const
{
    KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
    if (!view) {
        return qMakePair(QUrl(), KTextEditor::Cursor());
    }

    QUrl url = ICore::self()->documentController()->activeDocument()->url();
    const auto languages = ICore::self()->languageController()->languagesForUrl(url);
    for (const auto language : languages) {
        QPair<QUrl, KTextEditor::Cursor> pos = language->specialLanguageObjectJumpCursor(url, KTextEditor::Cursor(view->cursorPosition()));
        if (pos.second.isValid()) {
            return pos;
        }
    }

    return qMakePair(QUrl(), KTextEditor::Cursor::invalid());
}

bool QuickOpenPlugin::jumpToSpecialObject()
{
    QPair<QUrl, KTextEditor::Cursor> pos = specialObjectJumpPosition();
    if (pos.second.isValid()) {
        if (pos.first.isEmpty()) {
            qCDebug(PLUGIN_QUICKOPEN) << "Got empty url for special language object";
            return false;
        }

        ICore::self()->documentController()->openDocument(pos.first, pos.second);
        return true;
    }
    return false;
}

void QuickOpenPlugin::quickOpenDefinition()
{
    if (jumpToSpecialObject()) {
        return;
    }

    KDevelop::DUChainReadLocker lock(DUChain::lock());
    Declaration* decl = cursorDeclaration();

    if (!decl) {
        qCDebug(PLUGIN_QUICKOPEN) << "Found no declaration for cursor, cannot jump";
        return;
    }

    IndexedString u = decl->url();
    KTextEditor::Cursor c = decl->rangeInCurrentRevision().start();
    if (auto* def = FunctionDefinition::definition(decl)) {
        def->activateSpecialization();
        u = def->url();
        c = def->rangeInCurrentRevision().start();
    } else {
        qCDebug(PLUGIN_QUICKOPEN) << "Found no definition for declaration";
        decl->activateSpecialization();
    }

    if (u.isEmpty()) {
        qCDebug(PLUGIN_QUICKOPEN) << "Got empty url for declaration" << decl->toString();
        return;
    }

    lock.unlock();
    core()->documentController()->openDocument(u.toUrl(), c);
}

bool QuickOpenPlugin::freeModel()
{
    if (m_currentWidgetHandler) {
        delete m_currentWidgetHandler;
    }
    m_currentWidgetHandler = nullptr;

    return true;
}

void QuickOpenPlugin::nextFunction()
{
    jumpToNearestFunction(NextFunction);
}

void QuickOpenPlugin::previousFunction()
{
    jumpToNearestFunction(PreviousFunction);
}

void QuickOpenPlugin::jumpToNearestFunction(QuickOpenPlugin::FunctionJumpDirection direction)
{
    IDocument* doc = ICore::self()->documentController()->activeDocument();
    if (!doc) {
        qCDebug(PLUGIN_QUICKOPEN) << "No active document";
        return;
    }

    KDevelop::DUChainReadLocker lock(DUChain::lock());

    TopDUContext* context = DUChainUtils::standardContextForUrl(doc->url());

    if (!context) {
        qCDebug(PLUGIN_QUICKOPEN) << "Got no standard context";
        return;
    }

    QVector<DUChainItem> items;
    OutlineFilter filter(items, OutlineFilter::Functions);
    DUChainUtils::collectItems(context, filter);

    CursorInRevision cursor = context->transformToLocalRevision(KTextEditor::Cursor(doc->cursorPosition()));
    if (!cursor.isValid()) {
        return;
    }

    Declaration* nearestDeclBefore = nullptr;
    int distanceBefore = INT_MIN;
    Declaration* nearestDeclAfter = nullptr;
    int distanceAfter = INT_MAX;

    for (auto& item : std::as_const(items)) {
        Declaration* decl = item.m_item.data();

        int distance = decl->range().start.line - cursor.line;
        if (distance < 0 && distance >= distanceBefore) {
            distanceBefore = distance;
            nearestDeclBefore = decl;
        } else if (distance > 0 && distance <= distanceAfter) {
            distanceAfter = distance;
            nearestDeclAfter = decl;
        }
    }

    CursorInRevision c = CursorInRevision::invalid();
    if (direction == QuickOpenPlugin::NextFunction && nearestDeclAfter) {
        c = nearestDeclAfter->range().start;
    } else if (direction == QuickOpenPlugin::PreviousFunction && nearestDeclBefore) {
        c = nearestDeclBefore->range().start;
    }

    KTextEditor::Cursor textCursor = KTextEditor::Cursor::invalid();
    if (c.isValid()) {
        textCursor = context->transformFromLocalRevision(c);
    }

    lock.unlock();
    if (textCursor.isValid()) {
        core()->documentController()->openDocument(doc->url(), textCursor);
    } else {
        qCDebug(PLUGIN_QUICKOPEN) << "No declaration to jump to";
    }
}

struct CreateOutlineDialog
{
    void start()
    {
        if (!QuickOpenPlugin::self()->freeModel()) {
            return;
        }

        IDocument* doc = ICore::self()->documentController()->activeDocument();
        if (!doc) {
            qCDebug(PLUGIN_QUICKOPEN) << "No active document";
            return;
        }

        DUChainReadLocker lock;

        TopDUContext* context = DUChainUtils::standardContextForUrl(doc->url());

        if (!context) {
            qCDebug(PLUGIN_QUICKOPEN) << "Got no standard context";
            return;
        }

        model = new QuickOpenModel(nullptr);

        OutlineFilter filter(items);

        DUChainUtils::collectItems(context, filter);

        if (noHtmlDestriptionInOutline) {
            for (auto& item : items) {
                item.m_noHtmlDestription = true;
            }
        }

        cursorDecl = cursorContextDeclaration();

        model->registerProvider(QStringList(), QStringList(), new DeclarationListDataProvider(QuickOpenPlugin::self(), items, true));

        dialog = new QuickOpenWidgetDialog(i18nc("@title:window", "Outline"), model, QStringList(), QStringList(), true);
        dialog->widget()->setSortingEnabled(true);

        model->setParent(dialog->widget());
    }
    void finish()
    {
        //Select the declaration that contains the cursor
        if (cursorDecl.isValid() && dialog) {
            auto it = std::find_if(items.constBegin(), items.constEnd(),
                                   [this](const DUChainItem& item) { return item.m_item == cursorDecl; });
            if (it != items.constEnd()) {
                // Need to invoke the scrolling later. If we did it now, then it wouldn't have any effect,
                // apparently because the widget internals aren't initialized yet properly (although we've
                // already called 'widget->show()'.
                auto list = dialog->widget()->ui.list;
                const auto num = std::distance(items.constBegin(), it);
                QTimer::singleShot(0, list, [list, num]() {
                    const auto index = list->model()->index(num, 0, {});
                    list->setCurrentIndex(index);
                    list->scrollTo(index, QAbstractItemView::PositionAtCenter);
                });
            }
        }
    }
    QPointer<QuickOpenWidgetDialog> dialog;
    IndexedDeclaration cursorDecl;
    QVector<DUChainItem> items;
    QuickOpenModel* model = nullptr;
};

class OutlineQuickopenWidgetCreator
    : public QuickOpenWidgetCreator
{
public:
    OutlineQuickopenWidgetCreator(const QStringList& /*scopes*/, const QStringList& /*items*/) : m_creator(nullptr)
    {
    }

    ~OutlineQuickopenWidgetCreator() override
    {
        delete m_creator;
    }

    QuickOpenWidget* createWidget() override
    {
        delete m_creator;
        m_creator = new CreateOutlineDialog;
        m_creator->start();

        if (!m_creator->dialog) {
            return nullptr;
        }

        m_creator->dialog->deleteLater();
        return m_creator->dialog->widget();
    }

    void widgetShown() override
    {
        if (m_creator) {
            m_creator->finish();
            delete m_creator;
            m_creator = nullptr;
        }
    }

    QString objectNameForLine() override
    {
        return QStringLiteral("Outline");
    }

    CreateOutlineDialog* m_creator;
};

void QuickOpenPlugin::quickOpenNavigateFunctions()
{
    CreateOutlineDialog create;
    create.start();

    if (!create.dialog) {
        return;
    }

    m_currentWidgetHandler = create.dialog;

    QuickOpenLineEdit* line = quickOpenLine(QStringLiteral("Outline"));
    if (!line) {
        line  = quickOpenLine();
    }

    if (line) {
        line->showWithWidget(create.dialog->widget());
        create.dialog->deleteLater();
    } else {
        create.dialog->run();
    }

    create.finish();
}

QuickOpenLineEdit::QuickOpenLineEdit(QuickOpenWidgetCreator* creator) : m_widget(nullptr)
    , m_forceUpdate(false)
    , m_widgetCreator(creator)
{
    setFont(qApp->font("QToolButton"));
    setMinimumWidth(200);
    setMaximumWidth(400);

    deactivate();
    setPlaceholderText(i18nc("@info:placeholder", "Quick Open..."));
    setToolTip(i18nc("@info:tooltip", "Search for files, classes, functions and more,"
                    " allowing you to quickly navigate in your source code."));
    setObjectName(m_widgetCreator->objectNameForLine());
    setFocusPolicy(Qt::ClickFocus);
}

QuickOpenLineEdit::~QuickOpenLineEdit()
{
    delete m_widget;
    delete m_widgetCreator;
}

bool QuickOpenLineEdit::insideThis(QObject* object)
{
    while (object) {
        qCDebug(PLUGIN_QUICKOPEN) << object;
        if (object == this || object == m_widget) {
            return true;
        }
        object = object->parent();
    }
    return false;
}

void QuickOpenLineEdit::widgetDestroyed(QObject* obj)
{
    Q_UNUSED(obj);
    // need to use a queued connection here, because this function is called in ~QWidget!
    // => QuickOpenWidget instance is half-destructed => connections are not yet cleared
    // => clear() will trigger signals which will operate on the invalid QuickOpenWidget
    // So, just wait until properly destructed
    QMetaObject::invokeMethod(this, "deactivate", Qt::QueuedConnection);
}

void QuickOpenLineEdit::showWithWidget(QuickOpenWidget* widget)
{
    connect(widget, &QuickOpenWidget::destroyed, this, &QuickOpenLineEdit::widgetDestroyed);
    qCDebug(PLUGIN_QUICKOPEN) << "storing widget" << widget;
    deactivate();
    if (m_widget) {
        qCDebug(PLUGIN_QUICKOPEN) << "deleting" << m_widget;
        delete m_widget;
    }
    m_widget = widget;
    m_forceUpdate = true;
    setFocus();
}

void QuickOpenLineEdit::focusInEvent(QFocusEvent* ev)
{
    QLineEdit::focusInEvent(ev);
//       delete m_widget;
    qCDebug(PLUGIN_QUICKOPEN) << "got focus";
    qCDebug(PLUGIN_QUICKOPEN) << "old widget" << m_widget << "force update:" << m_forceUpdate;
    if (m_widget && !m_forceUpdate) {
        return;
    }

    if (!m_forceUpdate && !QuickOpenPlugin::self()->freeModel()) {
        deactivate();
        return;
    }

    m_forceUpdate = false;

    if (!m_widget) {
        m_widget = m_widgetCreator->createWidget();
        if (!m_widget) {
            deactivate();
            return;
        }
    }

    activate();

    m_widget->showStandardButtons(false);
    m_widget->showSearchField(false);

    m_widget->setParent(nullptr, Qt::ToolTip);
    m_widget->setFocusPolicy(Qt::NoFocus);
    m_widget->setAlternativeSearchField(this);

    QuickOpenPlugin::self()->m_currentWidgetHandler = m_widget;
    connect(m_widget.data(), &QuickOpenWidget::ready, this, &QuickOpenLineEdit::deactivate);

    connect(m_widget.data(), &QuickOpenWidget::scopesChanged, QuickOpenPlugin::self(), &QuickOpenPlugin::storeScopes);
    connect(m_widget.data(), &QuickOpenWidget::itemsChanged, QuickOpenPlugin::self(), &QuickOpenPlugin::storeItems);
    Q_ASSERT(m_widget->ui.searchLine == this);
    m_widget->prepareShow();
    QRect widgetGeometry = QRect(mapToGlobal(QPoint(0, height())), mapToGlobal(QPoint(width(), height() + 400)));
    widgetGeometry.setWidth(700); ///@todo Waste less space
    const auto screenGeom = screen()->geometry();
    if (widgetGeometry.right() > screenGeom.right()) {
        widgetGeometry.moveRight(screenGeom.right());
    }
    if (widgetGeometry.bottom() > screenGeom.bottom()) {
        widgetGeometry.moveBottom(mapToGlobal(QPoint(0, 0)).y());
    }
    m_widget->setGeometry(widgetGeometry);
    m_widget->show();

    m_widgetCreator->widgetShown();
}

void QuickOpenLineEdit::hideEvent(QHideEvent* ev)
{
    QWidget::hideEvent(ev);
    if (m_widget) {
        QMetaObject::invokeMethod(this, "checkFocus", Qt::QueuedConnection);
    }
//       deactivate();
}

bool QuickOpenLineEdit::eventFilter(QObject* obj, QEvent* e)
{
    if (!m_widget) {
        return QLineEdit::eventFilter(obj, e);
    }

    switch (e->type()) {
    case QEvent::KeyPress:
    case QEvent::ShortcutOverride:
        if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Escape) {
            deactivate();
            e->accept();
            return true; // eat event
        }
        break;
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        QMetaObject::invokeMethod(this, "checkFocus", Qt::QueuedConnection);
        break;
    // handle bug 260657 - "Outline menu doesn't follow main window on its move"
    case QEvent::Move: {
        if (QWidget* widget = qobject_cast<QWidget*>(obj)) {
            // close the outline menu in case a parent widget moved
            if (widget->isAncestorOf(this)) {
                qCDebug(PLUGIN_QUICKOPEN) << "closing because of parent widget move";
                deactivate();
            }
        }
        break;
    }
    case QEvent::FocusIn:
        if (qobject_cast<QWidget*>(obj)) {
            auto* focusEvent = dynamic_cast<QFocusEvent*>(e);
            Q_ASSERT(focusEvent);
            //Eat the focus event, keep the focus
            qCDebug(PLUGIN_QUICKOPEN) << "focus change" << "inside this: " << insideThis(obj) << "this" << this << "obj" << obj;
            if (obj == this) {
                break;
            }

            qCDebug(PLUGIN_QUICKOPEN) << "reason" << focusEvent->reason();
            if (focusEvent->reason() != Qt::MouseFocusReason && focusEvent->reason() != Qt::ActiveWindowFocusReason) {
                QMetaObject::invokeMethod(this, "checkFocus", Qt::QueuedConnection);
                break;
            }
            if (!insideThis(obj)) {
                deactivate();
            }
        } else if (obj != this) {
            QMetaObject::invokeMethod(this, "checkFocus", Qt::QueuedConnection);
        }
        break;
    default:
        break;
    }

    return QLineEdit::eventFilter(obj, e);
}
void QuickOpenLineEdit::activate()
{
    qCDebug(PLUGIN_QUICKOPEN) << "activating";
    qApp->installEventFilter(this);
}
void QuickOpenLineEdit::deactivate()
{
    qCDebug(PLUGIN_QUICKOPEN) << "deactivating";

    clear();

    if (m_widget || hasFocus()) {
        QMetaObject::invokeMethod(this, "checkFocus", Qt::QueuedConnection);
    }

    if (m_widget) {
        m_widget->deleteLater();
    }

    m_widget = nullptr;
    qApp->removeEventFilter(this);
}

void QuickOpenLineEdit::checkFocus()
{
    qCDebug(PLUGIN_QUICKOPEN) << "checking focus" << m_widget;
    if (m_widget) {
        QWidget* focusWidget = QApplication::focusWidget();
        bool focusWidgetInsideThis = focusWidget ? insideThis(focusWidget) : false;
        if (QApplication::focusWindow() && isVisible() && !isHidden() && (!focusWidget || (focusWidget && focusWidgetInsideThis))) {
            qCDebug(PLUGIN_QUICKOPEN) << "setting focus to line edit";
            activateWindow();
            setFocus();
        } else {
            qCDebug(PLUGIN_QUICKOPEN) << "deactivating because check failed, focusWidget" << focusWidget << "insideThis" << focusWidgetInsideThis;
            deactivate();
        }
    } else {
        if (ICore::self()->documentController()->activeDocument()) {
            ICore::self()->documentController()->activateDocument(ICore::self()->documentController()->activeDocument());
        }

        //Make sure the focus is somewhere else, even if there is no active document
        setEnabled(false);
        setEnabled(true);
    }
}

QLineEdit* QuickOpenPlugin::createQuickOpenLine(const QStringList& scopes, const QStringList& type, IQuickOpen::QuickOpenType kind)
{
    if (kind == Outline) {
        return new QuickOpenLineEdit(new OutlineQuickopenWidgetCreator(scopes, type));
    } else {
        return new QuickOpenLineEdit(new StandardQuickOpenWidgetCreator(scopes, type));
    }
}

#include "quickopenplugin.moc"
#include "moc_quickopenplugin.cpp"
