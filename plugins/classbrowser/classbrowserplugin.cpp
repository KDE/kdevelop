/*
  * This file is part of KDevelop
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Lior Mualem <lior.m.kde@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "classbrowserplugin.h"

#include <QAction>

#include <KLocalizedString>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>

#include "interfaces/icore.h"
#include "interfaces/iuicontroller.h"
#include "interfaces/idocumentcontroller.h"
#include "interfaces/contextmenuextension.h"

#include "language/interfaces/codecontext.h"
#include "language/duchain/duchainbase.h"
#include "language/duchain/duchain.h"
#include "language/duchain/duchainlock.h"
#include "language/duchain/declaration.h"
#include <serialization/indexedstring.h>

#include "debug.h"
#include "classmodel.h"
#include "classtree.h"
#include "classwidget.h"
#include <language/interfaces/editorcontext.h>
#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <interfaces/iprojectcontroller.h>

Q_LOGGING_CATEGORY(PLUGIN_CLASSBROWSER, "kdevplatform.plugins.classbrowser")
K_PLUGIN_FACTORY_WITH_JSON(KDevClassBrowserFactory, "kdevclassbrowser.json", registerPlugin<ClassBrowserPlugin>(); )

using namespace KDevelop;

class ClassBrowserFactory: public KDevelop::IToolViewFactory
{
public:
  ClassBrowserFactory(ClassBrowserPlugin *plugin): m_plugin(plugin) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new ClassWidget(parent, m_plugin);
  }

  virtual Qt::DockWidgetArea defaultPosition()
  {
    return Qt::LeftDockWidgetArea;
  }

  virtual QString id() const
  {
    return "org.kdevelop.ClassBrowserView";
  }

private:
  ClassBrowserPlugin *m_plugin;
};

ClassBrowserPlugin::ClassBrowserPlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin("kdevclassbrowser", parent)
    , m_factory(new ClassBrowserFactory(this))
    , m_activeClassTree(0)
{
  core()->uiController()->addToolView(i18n("Classes"), m_factory);
  setXMLFile( "kdevclassbrowser.rc" );

  m_findInBrowser = new QAction(i18n("Find in &Class Browser"), this);
  connect(m_findInBrowser, &QAction::triggered, this, &ClassBrowserPlugin::findInClassBrowser);
}

ClassBrowserPlugin::~ClassBrowserPlugin()
{
}

void ClassBrowserPlugin::unload()
{
  core()->uiController()->removeToolView(m_factory);
}

KDevelop::ContextMenuExtension ClassBrowserPlugin::contextMenuExtension( KDevelop::Context* context)
{
  KDevelop::ContextMenuExtension menuExt = KDevelop::IPlugin::contextMenuExtension( context );

  // No context menu if we don't have a class browser at hand.
  if ( m_activeClassTree == 0 )
    return menuExt;

  KDevelop::DeclarationContext *codeContext = dynamic_cast<KDevelop::DeclarationContext*>(context);

  if (!codeContext)
      return menuExt;

  DUChainReadLocker readLock(DUChain::lock());
  Declaration* decl(codeContext->declaration().data());

  if (decl)
  {
    if(decl->inSymbolTable()) {
      if(!ClassTree::populatingClassBrowserContextMenu() && ICore::self()->projectController()->findProjectForUrl(decl->url().toUrl()) &&
        decl->kind() == Declaration::Type && decl->internalContext() && decl->internalContext()->type() == DUContext::Class) {
        //Currently "Find in Class Browser" seems to only work for classes, so only show it in that case

        m_findInBrowser->setData(QVariant::fromValue(DUChainBasePointer(decl)));
        menuExt.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, m_findInBrowser);
      }
    }
  }

  return menuExt;
}

void ClassBrowserPlugin::findInClassBrowser()
{
  ICore::self()->uiController()->findToolView(i18n("Classes"), m_factory, KDevelop::IUiController::CreateAndRaise);

  Q_ASSERT(qobject_cast<QAction*>(sender()));

  if ( m_activeClassTree == 0 )
    return;

  DUChainReadLocker readLock(DUChain::lock());

  QAction* a = static_cast<QAction*>(sender());

  Q_ASSERT(a->data().canConvert<DUChainBasePointer>());

  DeclarationPointer decl = qvariant_cast<DUChainBasePointer>(a->data()).dynamicCast<Declaration>();
  if (decl)
    m_activeClassTree->highlightIdentifier(decl->qualifiedIdentifier());
}

void ClassBrowserPlugin::showDefinition(DeclarationPointer declaration)
{
  DUChainReadLocker readLock(DUChain::lock());

  if ( !declaration )
    return;

  Declaration* decl = declaration.data();
  // If it's a function, find the function definition to go to the actual declaration.
  if ( decl && decl->isFunctionDeclaration() )
  {
    FunctionDefinition* funcDefinition = dynamic_cast<FunctionDefinition*>(decl);
    if ( funcDefinition == 0 )
      funcDefinition = FunctionDefinition::definition(decl);
    if ( funcDefinition )
      decl = funcDefinition;
  }

  if (decl)
  {
    QUrl url = decl->url().toUrl();
    KTextEditor::Range range = decl->rangeInCurrentRevision();

    readLock.unlock();

    ICore::self()->documentController()->openDocument(url, range.start());
  }
}

#include "classbrowserplugin.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
