/*
  * This file is part of KDevelop
 *
 * Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include "contextbrowser.h"

#include <QTimer>
#include <QApplication>
#include <klocale.h>
#include <kaction.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>
#include <KTextEditor/TextHintInterface>
#include <kactioncollection.h>
#include <kdebug.h>
#include <duchain/duchainlock.h>
#include <duchain/duchain.h>
#include <duchain/ducontext.h>
#include <duchain/declaration.h>
#include <duchain/use.h>
#include <duchain/duchainutils.h>
#include <icore.h>
#include <idocumentcontroller.h>
#include <ilanguage.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/ilanguagesupport.h>
#include <ilanguagecontroller.h>
#include <backgroundparser/backgroundparser.h>
#include <backgroundparser/parsejob.h>
#include "contextbrowserview.h"

const unsigned int highlightingTimeout = 150;

class ContextBrowserViewFactory: public KDevelop::IToolViewFactory
{
public:
    ContextBrowserViewFactory(ContextBrowserPlugin *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget */*parent*/ = 0)
    {
        return new ContextBrowserView(m_plugin);
    }

    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::LeftDockWidgetArea;
    }

    virtual QString id() const
    {
        return "org.kdevelop.DUChainView";
    }

private:
    ContextBrowserPlugin *m_plugin;
};

K_PLUGIN_FACTORY(ContextBrowserFactory, registerPlugin<ContextBrowserPlugin>(); )
K_EXPORT_PLUGIN(ContextBrowserFactory("kdevcontextbrowser"))

ContextBrowserPlugin::ContextBrowserPlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(ContextBrowserFactory::componentData(), parent)
    , m_viewFactory(new ContextBrowserViewFactory(this))
{
  setXMLFile( "kdevcontextbrowser.rc" );

  KActionCollection* actions = actionCollection();

  core()->uiController()->addToolView(i18n("Context Browser"), m_viewFactory);
  
  connect( core()->documentController(), SIGNAL( documentLoaded( KDevelop::IDocument* ) ), this, SLOT( documentLoaded( KDevelop::IDocument* ) ) );
  connect( core()->documentController(), SIGNAL( documentClosed( KDevelop::IDocument* ) ), this, SLOT( documentClosed( KDevelop::IDocument* ) ) );
  connect( core()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), this, SLOT(parseJobFinished(KDevelop::ParseJob*)));

  m_updateTimer = new QTimer(this);
  m_updateTimer->setSingleShot(true);
  connect( m_updateTimer, SIGNAL( timeout() ), this, SLOT( updateViews() ) );

  QAction* previousContext = actions->addAction("previous_context");
  previousContext->setText( i18n("&Previous Context") );
  previousContext->setShortcut( Qt::META | Qt::Key_Left );
  connect(previousContext, SIGNAL(triggered(bool)), this, SIGNAL(previousContextShortcut()));

  QAction* nextContext = actions->addAction("next_context");
  nextContext->setText( i18n("&Next Context") );
  nextContext->setShortcut( Qt::META | Qt::Key_Right );
  connect(nextContext, SIGNAL(triggered(bool)), this, SIGNAL(nextContextShortcut()));
}

ContextBrowserPlugin::~ContextBrowserPlugin()
{
  ///@todo unregister from all ranges that were registered earlier! Else we may get a crash within kate while destruction.
}

void ContextBrowserPlugin::unload()
{
  core()->uiController()->removeToolView(m_viewFactory);
}

void ContextBrowserPlugin::rangeDeleted( KTextEditor::SmartRange *range ) {
  m_backups.remove( range );
  
  for(QMap<KTextEditor::View*, KTextEditor::SmartRange*>::iterator it = m_highlightedRange.begin(); it != m_highlightedRange.end(); ++it)
    if(*it == range) {
      m_highlightedRange.erase(it);
      return;
    }
}

///@todo this doesn't work, we use TextHintInterface instead atm.
void ContextBrowserPlugin::mouseEnteredRange( KTextEditor::SmartRange* range, KTextEditor::View* view ) {
  kDebug() << "entered";
  m_mouseHoverCursor = SimpleCursor(range->start());
  m_mouseHoverDocument = view->document()->url();
  m_updateViews << view;
  m_updateTimer->start(1);
}

void ContextBrowserPlugin::mouseExitedRange( KTextEditor::SmartRange* /*range*/, KTextEditor::View* view ) {
  clearMouseHover();
  m_updateViews << view;
  m_updateTimer->start(1);
}

void ContextBrowserPlugin::textHintRequested(const KTextEditor::Cursor& cursor, QString&) {
  kDebug() << "textHintRequested";
  m_mouseHoverCursor = SimpleCursor(cursor);
  KTextEditor::View* view = dynamic_cast<KTextEditor::View*>(sender());
  if(!view) {
    kWarning() << "could not cast to view";
  }else{
    m_mouseHoverDocument = view->document()->url();
    m_updateViews << view;
  }
  m_updateTimer->start(1);
}

void ContextBrowserPlugin::clearMouseHover() {
  m_mouseHoverCursor = SimpleCursor::invalid();
  m_mouseHoverDocument = KUrl();
}


KTextEditor::Attribute::Ptr highlightedUseAttribute(bool /*mouseHighlight*/) {
  static KTextEditor::Attribute::Ptr standardAttribute = KTextEditor::Attribute::Ptr();
  if( !standardAttribute ) {
    standardAttribute = KTextEditor::Attribute::Ptr( new KTextEditor::Attribute() );
    standardAttribute->setBackgroundFillWhitespace(true);
    standardAttribute->setBackground(Qt::yellow);//QApplication::palette().toolTipBase());
  }
  return standardAttribute;
}

KTextEditor::Attribute::Ptr highlightedDeclarationAttribute() {
  static KTextEditor::Attribute::Ptr standardAttribute = KTextEditor::Attribute::Ptr();
  if( !standardAttribute ) {
    standardAttribute = KTextEditor::Attribute::Ptr( new KTextEditor::Attribute() );
    standardAttribute->setBackgroundFillWhitespace(true);
    standardAttribute->setBackground(Qt::red);
  }
  return standardAttribute;
}

KTextEditor::Attribute::Ptr highlightedSpecialObjectAttribute() {
  static KTextEditor::Attribute::Ptr standardAttribute = KTextEditor::Attribute::Ptr();
  if( !standardAttribute ) {
    standardAttribute = KTextEditor::Attribute::Ptr( new KTextEditor::Attribute() );
    standardAttribute->setBackgroundFillWhitespace(true);
    standardAttribute->setBackground(Qt::red);
  }
  return standardAttribute;
}

void ContextBrowserPlugin::changeHighlight( KTextEditor::SmartRange* range, bool highlight, bool /*declaration*/, bool mouseHighlight ) {
  if( !range )
    return;

  KTextEditor::Attribute::Ptr attrib;
  if( highlight ) {
    if( !m_backups.contains(range) ) {
      m_backups[range] = range->attribute();
      range->addWatcher(this);
    }
/*    if( declaration )
      attrib = highlightedDeclarationAttribute();
    else*/
      attrib = highlightedUseAttribute(mouseHighlight);
  }else{
    if( m_backups.contains(range) ) {
      attrib = m_backups[range];
      m_backups.remove(range);
    }
    range->removeWatcher(this);
  }
  
  range->setAttribute(attrib);
}

void ContextBrowserPlugin::changeHighlight( KTextEditor::View* view, KDevelop::Declaration* decl, bool highlight, bool mouseHighlight ) {
  if( !view || !decl ) {
    kDebug() << "invalid view/declaration";
    return;
  }
  
  KTextEditor::SmartRange* range = decl->smartRange();
  if( range )
    changeHighlight( range, highlight, true, mouseHighlight );
  

  QList<KTextEditor::SmartRange*> uses;
  {
    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    uses = decl->smartUses();
  }

  foreach(KTextEditor::SmartRange* range, uses)
    changeHighlight( range, highlight, false, mouseHighlight );
  
  if( decl->definition() && decl->definition()->smartRange() ) {
    changeHighlight( decl->definition()->smartRange(), highlight, false, mouseHighlight );
  }
}

QWidget* masterWidget(QWidget* w) {
  while(w && w->parent() && qobject_cast<QWidget*>(w->parent()))
    w = qobject_cast<QWidget*>(w->parent());
  return w;
}

void ContextBrowserPlugin::updateViews()
{
  foreach( KTextEditor::View* view, m_updateViews ) {
    SimpleCursor c = SimpleCursor(view->cursorPosition());

    ///First: Check whether there is a special language object
    ///@todo Maybe make this optional, because it can be slow

    ///Find either a Declaration, or a use, that is in the Range.

    bool mouseHighlight = false;

    if( view->document()->url() == m_mouseHoverDocument && m_mouseHoverCursor.isValid() ) {
      c = m_mouseHoverCursor;
      mouseHighlight = true;
    }

    KTextEditor::SmartInterface* iface = dynamic_cast<KTextEditor::SmartInterface*>(view->document());

    bool foundSpecialObject = false;
    KDevelop::ILanguage* pickedLanguage = 0;
    
    if(iface) {
      //Delete old special highlighting
      if(m_highlightedRange.contains(view)) {
        QMutexLocker lock(iface ? iface->smartMutex() : 0);

        Q_ASSERT(m_highlightedRange[view]->document() == view->document());
        
        delete m_highlightedRange[view];
        m_highlightedRange.remove(view);
      }
      
      //Eventually find a special language object, and directly highlight it
      QList<KDevelop::ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(view->document()->url());

      foreach( KDevelop::ILanguage* language, languages) {
        SimpleRange r = language->languageSupport()->specialLanguageObjectRange(view->document()->url(), c);
        if(r.isValid()) {
          //We've got our range. Highlight it and continue.

          m_highlightedRange[view] = iface->newSmartRange( view->document()->documentRange() );
          ///@todo This is a workaround for a probable bug, the range is not highlighted without a big master-range
          ///@todo Currently this ihghlighitng does no twork at all!
          KTextEditor::SmartRange* highlightRange = iface->newSmartRange( r.textRange(), m_highlightedRange[view] );
          
          highlightRange->setAttribute( highlightedSpecialObjectAttribute() );
/*          m_highlightedRange[view]*/

          m_highlightedRange[view]->addWatcher( this );
          
          foundSpecialObject = true;
          pickedLanguage = language;

          break;
        }
      }
    }
    
    KDevelop::DUChainReadLocker lock( DUChain::lock(), 100 );
    if(!lock.locked()) {
      kDebug() << "Failed to lock du-chain in time";
      continue;
    }

    TopDUContext* topContext = DUChainUtils::standardContextForUrl(view->document()->url());

    ///unhighlight the old uses
    if( m_highlightedDeclarations.contains(view) )
      changeHighlight( view, m_highlightedDeclarations[view].data(), false, mouseHighlight );

    Declaration* foundDeclaration = 0;
    
    if(!foundSpecialObject) {
      //If we haven't found a special language object, search for a use/declaration and eventually highlight it
      foundDeclaration = DUChainUtils::declarationForDefinition( DUChainUtils::itemUnderCursor(view->document()->url(), c) );

      if( foundDeclaration ) {
        m_highlightedDeclarations[view] = foundDeclaration;
        changeHighlight( view, foundDeclaration, true, mouseHighlight );
      }else{
        kDebug() << "not found declaration";
        m_highlightedDeclarations.remove(view);
      }
    }
    
    ///Update the context widget, and maybe show a tooltip
    if(foundDeclaration || foundSpecialObject) {
      if(mouseHighlight || (view->isActiveView() && core()->documentController()->activeDocument() && core()->documentController()->activeDocument()->textDocument() == view->document())) {
        //Update the context browser, but only if this view is active or has been mouse-hovered
        bool foundVisibleContextView = false;
        foreach(ContextBrowserView* contextView, m_views) {
          if(masterWidget(contextView) == masterWidget(view)) {
            if(foundDeclaration)
              contextView->declarationWidget()->setDeclaration(foundDeclaration, topContext);
            else
              contextView->declarationWidget()->setSpecialNavigationWidget(pickedLanguage->languageSupport()->specialLanguageObjectNavigationWidget(view->document()->url(), c));
            
            if(contextView->isVisible())
              foundVisibleContextView = true;
          }
        }
        if(mouseHighlight && !foundVisibleContextView) {
          ///@todo show tooltip
        }
      }
    }
  
    ///Update the context information
    if(topContext && !mouseHighlight) {
      DUContext* ctx = topContext->findContextAt(c);
      while(ctx && (ctx->type() == DUContext::Template || ctx->type() == DUContext::Helper || ctx->localScopeIdentifier().isEmpty()) && ctx->parentContext())
        ctx = ctx->parentContext();
      if(ctx)
        foreach(ContextBrowserView* contextView, m_views)
          if(masterWidget(contextView) == masterWidget(view))
            contextView->contextWidget()->setContext(ctx, c);
    }
  
  }
  m_updateViews.clear();
}

void ContextBrowserPlugin::parseJobFinished(KDevelop::ParseJob* job)
{
  KDevelop::DUChainWriteLocker lock( DUChain::lock() );
  registerAsRangeWatcher(job->duChain());
}

void ContextBrowserPlugin::registerAsRangeWatcher(KDevelop::DUChainBase* base)
{
  if(base->smartRange()) {
    base->smartRange()->removeWatcher(this); //Make sure we're never registered twice
    base->smartRange()->addWatcher(this);
  }
}

void ContextBrowserPlugin::registerAsRangeWatcher(KDevelop::DUContext* ctx)
{
  if(!ctx)
    return;
  if(dynamic_cast<TopDUContext*>(ctx) && static_cast<TopDUContext*>(ctx)->flags() & TopDUContext::ProxyContextFlag && !ctx->importedParentContexts().isEmpty())
    return registerAsRangeWatcher(ctx->importedParentContexts()[0].data());
  
  foreach(Declaration* decl, ctx->localDeclarations())
    registerAsRangeWatcher(decl);

  for(int a = 0; a < ctx->uses().size(); ++a) {
    KTextEditor::SmartRange* range = ctx->useSmartRange(a);
    if(range) {
      range->removeWatcher(this); //Make sure we're never registered twice
      range->addWatcher(this);
    }
  }

  foreach(DUContext* child, ctx->childContexts())
    registerAsRangeWatcher(child);
}

void ContextBrowserPlugin::documentLoaded( KDevelop::IDocument* document )
{
  kDebug() << "connecting document";
  connect( document->textDocument(), SIGNAL(destroyed( QObject* )), this, SLOT( documentDestroyed( QObject* ) ) );
  connect( document->textDocument(), SIGNAL( viewCreated( KTextEditor::Document* , KTextEditor::View* ) ), this, SLOT( viewCreated( KTextEditor::Document*, KTextEditor::View* ) ) );

  if (document->textDocument())
    foreach( KTextEditor::View* view, document->textDocument()->views() )
      viewCreated( document->textDocument(), view );

  KDevelop::DUChainWriteLocker lock( DUChain::lock() );
  QList<TopDUContext*> chains = DUChain::self()->chainsForDocument( document->url() );

  foreach( TopDUContext* chain, chains )
    registerAsRangeWatcher( chain );
}

void ContextBrowserPlugin::documentClosed( KDevelop::IDocument* document )
{
}

void ContextBrowserPlugin::documentDestroyed( QObject* obj )
{
}

void ContextBrowserPlugin::viewDestroyed( QObject* obj )
{
  m_highlightedDeclarations.remove(static_cast<KTextEditor::View*>(obj));
  m_updateViews.remove(static_cast<KTextEditor::View*>(obj));
}

void ContextBrowserPlugin::cursorPositionChanged( KTextEditor::View* view, const KTextEditor::Cursor& newPosition )
{
  kDebug() << "cursorPositionChanged";
  clearMouseHover();
  m_updateViews.insert(view);
  m_updateTimer->start(highlightingTimeout/2);
}

void ContextBrowserPlugin::viewCreated( KTextEditor::Document* , KTextEditor::View* v )
{
  kDebug() << "connecting view";
  
  disconnect( v, SIGNAL( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ), this, SLOT( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ) ); ///Just to make sure that multiple connections don't happen
  connect( v, SIGNAL( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ), this, SLOT( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ) );
  connect( v, SIGNAL(destroyed( QObject* )), this, SLOT( viewDestroyed( QObject* ) ) );
  
  KTextEditor::TextHintInterface *iface = dynamic_cast<KTextEditor::TextHintInterface*>(v);
  if( !iface )
      return;

  iface->enableTextHints(highlightingTimeout);

  connect(v, SIGNAL(needTextHint(const KTextEditor::Cursor&, QString&)), this, SLOT(textHintRequested(const KTextEditor::Cursor&, QString&)));    
}

void ContextBrowserPlugin::registerToolView(ContextBrowserView* view)
{
  m_views << view;
}

void ContextBrowserPlugin::unRegisterToolView(ContextBrowserView* view)
{
  m_views.removeAll(view);
}


#include "contextbrowser.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
