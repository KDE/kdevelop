/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "classbrowserplugin.h"

#include <QAction>

#include <KLocalizedString>
#include <KPluginFactory>

#include "interfaces/icore.h"
#include "interfaces/iuicontroller.h"
#include "interfaces/idocumentcontroller.h"
#include "interfaces/contextmenuextension.h"

#include "language/interfaces/codecontext.h"
#include "language/duchain/duchainbase.h"
#include "language/duchain/duchain.h"
#include "language/duchain/duchainlock.h"
#include "language/duchain/declaration.h"

#include "debug.h"
#include "classtree.h"
#include "classwidget.h"
#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <interfaces/iprojectcontroller.h>

K_PLUGIN_FACTORY_WITH_JSON(KDevClassBrowserFactory, "kdevclassbrowser.json", registerPlugin<ClassBrowserPlugin>(); )

using namespace KDevelop;

class ClassBrowserFactory
    : public KDevelop::IToolViewFactory
{
public:
    explicit ClassBrowserFactory(ClassBrowserPlugin* plugin) : m_plugin(plugin) {}

    QWidget* create(QWidget* parent = nullptr) override
    {
        return new ClassWidget(parent, m_plugin);
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        return Qt::LeftDockWidgetArea;
    }

    QString id() const override
    {
        return QStringLiteral("org.kdevelop.ClassBrowserView");
    }

private:
    ClassBrowserPlugin* m_plugin;
};

ClassBrowserPlugin::ClassBrowserPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevclassbrowser"), parent, metaData)
    , m_factory(new ClassBrowserFactory(this))
    , m_activeClassTree(nullptr)
{
    core()->uiController()->addToolView(i18nc("@title:window", "Classes"), m_factory);
    setXMLFile(QStringLiteral("kdevclassbrowser.rc"));

    m_findInBrowser = new QAction(i18nc("@action", "Find in &Class Browser"), this);
    connect(m_findInBrowser, &QAction::triggered, this, &ClassBrowserPlugin::findInClassBrowser);
}

ClassBrowserPlugin::~ClassBrowserPlugin()
{
}

void ClassBrowserPlugin::unload()
{
    core()->uiController()->removeToolView(m_factory);
}

KDevelop::ContextMenuExtension ClassBrowserPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    KDevelop::ContextMenuExtension menuExt = KDevelop::IPlugin::contextMenuExtension(context, parent);

    // No context menu if we don't have a class browser at hand.
    if (m_activeClassTree == nullptr)
        return menuExt;

    auto* codeContext = dynamic_cast<KDevelop::DeclarationContext*>(context);

    if (!codeContext)
        return menuExt;

    DUChainReadLocker readLock(DUChain::lock());
    Declaration* decl(codeContext->declaration().data());

    if (decl) {
        if (decl->inSymbolTable()) {
            if (!ClassTree::populatingClassBrowserContextMenu() &&
                ICore::self()->projectController()->findProjectForUrl(decl->url().toUrl()) &&
                decl->kind() == Declaration::Type && decl->internalContext() &&
                decl->internalContext()->type() == DUContext::Class) {
                //Currently "Find in Class Browser" seems to only work for classes, so only show it in that case

                m_findInBrowser->setData(QVariant::fromValue(DUChainBasePointer(decl)));
                menuExt.addAction(KDevelop::ContextMenuExtension::NavigationGroup, m_findInBrowser);
            }
        }
    }

    return menuExt;
}

void ClassBrowserPlugin::findInClassBrowser()
{
    ICore::self()->uiController()->findToolView(i18nc("@title:window", "Classes"), m_factory, KDevelop::IUiController::CreateAndRaise);

    Q_ASSERT(qobject_cast<QAction*>(sender()));

    if (m_activeClassTree == nullptr)
        return;

    DUChainReadLocker readLock(DUChain::lock());

    auto* a = static_cast<QAction*>(sender());

    Q_ASSERT(a->data().canConvert<DUChainBasePointer>());

    DeclarationPointer decl = qvariant_cast<DUChainBasePointer>(a->data()).dynamicCast<Declaration>();
    if (decl)
        m_activeClassTree->highlightIdentifier(decl->qualifiedIdentifier());
}

void ClassBrowserPlugin::showDefinition(const DeclarationPointer& declaration)
{
    DUChainReadLocker readLock(DUChain::lock());

    if (!declaration)
        return;

    Declaration* decl = declaration.data();
    // If it's a function, find the function definition to go to the actual declaration.
    if (decl && decl->isFunctionDeclaration()) {
        if (auto* funcDefinition = FunctionDefinition::definition(decl))
            decl = funcDefinition;
    }

    if (decl) {
        QUrl url = decl->url().toUrl();
        KTextEditor::Range range = decl->rangeInCurrentRevision();

        readLock.unlock();

        ICore::self()->documentController()->openDocument(url, range.start());
    }
}

#include "classbrowserplugin.moc"
#include "moc_classbrowserplugin.cpp"
