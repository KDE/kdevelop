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

#include "usehighlight.h"

#include <QTimer>
#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
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
#include <interfaces/ilanguagesupport.h>
#include <ilanguagecontroller.h>
#include <backgroundparser/backgroundparser.h>
#include <backgroundparser/parsejob.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevUseHighlightFactory, registerPlugin<UseHighlightPlugin>(); )
K_EXPORT_PLUGIN(KDevUseHighlightFactory("kdevusehighlight"))

UseHighlightPlugin::UseHighlightPlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(KDevUseHighlightFactory::componentData(), parent)
{
  connect( core()->documentController(), SIGNAL( documentLoaded( KDevelop::IDocument* ) ), this, SLOT( documentLoaded( KDevelop::IDocument* ) ) );
  connect( core()->documentController(), SIGNAL( documentClosed( KDevelop::IDocument* ) ), this, SLOT( documentClosed( KDevelop::IDocument* ) ) );
  connect( core()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), this, SLOT(parseJobFinished(KDevelop::ParseJob*)));
  
  m_updateTimer = new QTimer(this);
  m_updateTimer->setSingleShot(true);
  connect( m_updateTimer, SIGNAL( timeout() ), this, SLOT( updateViews() ) );
}

UseHighlightPlugin::~UseHighlightPlugin()
{
}

void UseHighlightPlugin::unload()
{
}

void UseHighlightPlugin::rangeDeleted( KTextEditor::SmartRange *range ) {
  m_backups.remove( range );
}

void UseHighlightPlugin::mouseEnteredRange( KTextEditor::SmartRange* range, KTextEditor::View* view ) {
  m_mouseHoverCursor = SimpleCursor(range->start());
  m_mouseHoverDocument = view->document()->url();
  m_updateViews << view;
}

void UseHighlightPlugin::mouseExitedRange( KTextEditor::SmartRange* /*range*/, KTextEditor::View* view ) {
  m_mouseHoverCursor = SimpleCursor::invalid();
  m_mouseHoverDocument = KUrl();
  m_updateViews << view;
}

KTextEditor::Attribute::Ptr highlightedUseAttribute(bool mouseHighlight) {
  if(!mouseHighlight) {
    static KTextEditor::Attribute::Ptr standardAttribute = KTextEditor::Attribute::Ptr();
    if( !standardAttribute ) {
      standardAttribute = KTextEditor::Attribute::Ptr( new KTextEditor::Attribute() );
      standardAttribute->setBackgroundFillWhitespace(true);
      standardAttribute->setBackground(Qt::yellow);
    }
    return standardAttribute;
  }else{
    static KTextEditor::Attribute::Ptr standardAttribute = KTextEditor::Attribute::Ptr();
    if( !standardAttribute ) {
      standardAttribute = KTextEditor::Attribute::Ptr( new KTextEditor::Attribute() );
      standardAttribute->setBackgroundFillWhitespace(true);
      standardAttribute->setBackground(Qt::darkYellow);
    }
    return standardAttribute;
  }
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

void UseHighlightPlugin::changeHighlight( KTextEditor::SmartRange* range, bool highlight, bool declaration, bool mouseHighlight ) {
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

void UseHighlightPlugin::changeHighlight( KTextEditor::View* view, KDevelop::Declaration* decl, bool highlight, bool mouseHighlight ) {
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

void UseHighlightPlugin::updateViews()
{
  foreach( KTextEditor::View* view, m_updateViews ) {
    SimpleCursor c = SimpleCursor(view->cursorPosition());
    ///Find either a Declaration, or a use, that is in the Range.

    bool mouseHighlight = false;

    if( view->document()->url() == m_mouseHoverDocument && m_mouseHoverCursor.isValid() ) {
      c = m_mouseHoverCursor;
      mouseHighlight = true;
    }

    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    Declaration* foundDeclaration = DUChainUtils::declarationForDefinition( DUChainUtils::itemUnderCursor(view->document()->url(), c) );

    if( m_highlightedDeclarations.contains(view) ) {
      ///Step 1: unhighlight the old uses
      changeHighlight( view, m_highlightedDeclarations[view].data(), false, mouseHighlight );
    }
    if( foundDeclaration ) {
      kDebug() << "found declaration";
      m_highlightedDeclarations[view] = foundDeclaration;
      changeHighlight( view, foundDeclaration, true, mouseHighlight );
    }else{
      kDebug() << "not found declaration";
      m_highlightedDeclarations.remove(view);
    }
  }
  m_updateViews.clear();
}

void UseHighlightPlugin::parseJobFinished(KDevelop::ParseJob* job)
{
  KDevelop::DUChainWriteLocker lock( DUChain::lock() );
  registerAsRangeWatcher(job->duChain());
}

void UseHighlightPlugin::registerAsRangeWatcher(KDevelop::DUChainBase* base)
{
  if(base->smartRange()) {
    base->smartRange()->removeWatcher(this); //Make sure we're never registered twice
    base->smartRange()->addWatcher(this);
  }
}

void UseHighlightPlugin::registerAsRangeWatcher(KDevelop::DUContext* ctx)
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

void UseHighlightPlugin::documentLoaded( KDevelop::IDocument* document )
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

void UseHighlightPlugin::documentClosed( KDevelop::IDocument* document )
{
}

void UseHighlightPlugin::documentDestroyed( QObject* obj )
{
}

void UseHighlightPlugin::viewDestroyed( QObject* obj )
{
  m_highlightedDeclarations.remove(static_cast<KTextEditor::View*>(obj));
  m_updateViews.remove(static_cast<KTextEditor::View*>(obj));
}

void UseHighlightPlugin::cursorPositionChanged( KTextEditor::View* view, const KTextEditor::Cursor& newPosition )
{
  kDebug() << "cursorPositionChanged";
  m_updateViews.insert(view);
  m_updateTimer->start(100);
}

void UseHighlightPlugin::viewCreated( KTextEditor::Document* , KTextEditor::View* v )
{
  kDebug() << "connecting view";
  
  disconnect( v, SIGNAL( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ), this, SLOT( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ) ); ///Just to make sure that multiple connections don't happen
  connect( v, SIGNAL( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ), this, SLOT( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ) );
  connect( v, SIGNAL(destroyed( QObject* )), this, SLOT( viewDestroyed( QObject* ) ) );
}


#include "usehighlight.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
