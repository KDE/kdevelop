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

#include <klocale.h>
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
#include "language/duchain/indexedstring.h"

#include "classmodel.h"
#include "classtree.h"
#include <language/interfaces/codecontext.h>

K_PLUGIN_FACTORY(KDevClassBrowserFactory, registerPlugin<ClassBrowserPlugin>(); )
K_EXPORT_PLUGIN(KDevClassBrowserFactory(KAboutData("kdevclassbrowser","kdevclassbrowser",ki18n("Class Browser"), "0.1", ki18n("Browser for all known classes"), KAboutData::License_GPL)))

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
    : KDevelop::IPlugin(KDevClassBrowserFactory::componentData(), parent)
    , m_factory(new ClassBrowserFactory(this))
    , m_model(new ClassModel())
{
  core()->uiController()->addToolView(i18n("Classes"), m_factory);
  setXMLFile( "kdevclassbrowser.rc" );

  //connect(core()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), m_model, SLOT(documentActivated(KDevelop::IDocument*)));
}

ClassBrowserPlugin::~ClassBrowserPlugin()
{
}

void ClassBrowserPlugin::unload()
{
  core()->uiController()->removeToolView(m_factory);
}

ClassModel* ClassBrowserPlugin::model() const
{
  return m_model;
}

KDevelop::ContextMenuExtension ClassBrowserPlugin::contextMenuExtension( KDevelop::Context* context)
{
  KDevelop::ContextMenuExtension menuExt = KDevelop::IPlugin::contextMenuExtension( context );

  if( context->type() != KDevelop::Context::CodeContext )
      return menuExt;

  KDevelop::DeclarationContext *codeContext = dynamic_cast<KDevelop::DeclarationContext*>(context);
  if (!codeContext)
      return menuExt;

  DUChainReadLocker readLock(DUChain::lock());

  IndexedDeclaration decl = codeContext->declaration();

  DUChainBasePointer base(decl.data());
  
  if (base) {
    QAction* openDec = new QAction(i18n("Open &Declaration"), this);
    connect(openDec, SIGNAL(triggered(bool)), this, SLOT(openDeclaration()));
    openDec->setData(QVariant::fromValue(base));
//     menuExt.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, openDec);

    QAction* openDef = new QAction(i18n("Open De&finition"), this);
    connect(openDef, SIGNAL(triggered(bool)), this, SLOT(openDefinition()));
    openDef->setData(QVariant::fromValue(base));
//     menuExt.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, openDef);

    Declaration* dec = 0;

    if (DUContext* d = dynamic_cast<DUContext*>(base.data())) {
      dec = d->owner();
    } else if (0 != (dec = dynamic_cast<Declaration*>(base.data()))) {
  // ### do something here
    }

    /*if(!model()->definitionForObject(base))
      openDef->setEnabled(false);
    if(!model()->declarationForObject(base))
      openDec->setEnabled(false);*/
  }

  return menuExt;
}

void ClassBrowserPlugin::openDeclaration()
{
  Q_ASSERT(qobject_cast<QAction*>(sender()));

  DUChainReadLocker readLock(DUChain::lock());

  QAction* a = static_cast<QAction*>(sender());

  Q_ASSERT(a->data().canConvert<DUChainBasePointer>());

  DUChainBasePointer base = qvariant_cast<DUChainBasePointer>(a->data());
  if (base) {
    /*Declaration* dec = model()->declarationForObject(base);

    if (dec) {
      KUrl url( dec->url().str() );
      KTextEditor::Range range = dec->range().textRange();

      readLock.unlock();

      ICore::self()->documentController()->openDocument(url, range.start());

    } else {
      kDebug() << "No declaration for base object" << base;
    }*/

  } else {
    kDebug() << "Base object has disappeared from the duchain";
  }
}

void ClassBrowserPlugin::openDefinition()
{
  Q_ASSERT(qobject_cast<QAction*>(sender()));

  DUChainReadLocker readLock(DUChain::lock());

  DUChainBasePointer base = qvariant_cast<DUChainBasePointer>(static_cast<QAction*>(sender())->data());
  if (base) {
    /*Declaration* def = model()->definitionForObject(base);

    if (def) {
      KUrl url(def->url().str());
      KTextEditor::Range range = def->range().textRange();

      readLock.unlock();

      ICore::self()->documentController()->openDocument(url, range.start());
    }*/
  }
}


#include "classbrowserplugin.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
