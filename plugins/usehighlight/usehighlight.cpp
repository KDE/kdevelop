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
#include <icore.h>
#include <idocumentcontroller.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevUseHighlightFactory, registerPlugin<UseHighlightPlugin>(); )
K_EXPORT_PLUGIN(KDevUseHighlightFactory("kdevusehighlight"))

UseHighlightPlugin::UseHighlightPlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(KDevUseHighlightFactory::componentData(), parent)
{
  connect( core()->documentController(), SIGNAL( documentLoaded( KDevelop::IDocument* ) ), this, SLOT( documentLoaded( KDevelop::IDocument* ) ) );
  connect( core()->documentController(), SIGNAL( documentClosed( KDevelop::IDocument* ) ), this, SLOT( documentClosed( KDevelop::IDocument* ) ) );
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


KTextEditor::Attribute::Ptr highlightedUseAttribute() {
  static KTextEditor::Attribute::Ptr standardAttribute = KTextEditor::Attribute::Ptr();
  if( !standardAttribute ) {
    standardAttribute = KTextEditor::Attribute::Ptr( new KTextEditor::Attribute() );
    standardAttribute->setBackgroundFillWhitespace(true);
    standardAttribute->setBackground(Qt::yellow);
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

void UseHighlightPlugin::changeHighlight( KTextEditor::SmartRange* range, bool highlight, bool declaration ) {
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
      attrib = highlightedUseAttribute();
  }else{
    if( m_backups.contains(range) ) {
      attrib = m_backups[range];
      m_backups.remove(range);
    }
    range->removeWatcher(this);
  }
  
  range->setAttribute(attrib);
}

void UseHighlightPlugin::changeHighlight( KTextEditor::View* view, KDevelop::Declaration* decl, bool highlight ) {
  if( !view || !decl ) {
    kDebug() << "invalid view/declaration";
    return;
  }
  
  KTextEditor::SmartRange* range = decl->smartRange();
  if( range )
    changeHighlight( range, highlight, true );
  

  foreach( Use* use, decl->uses() ) {
    KTextEditor::SmartRange* range = use->smartRange();
    if( !range )
      continue;
    changeHighlight( range, highlight, false );
  }
}

void UseHighlightPlugin::updateViews()
{
  foreach( KTextEditor::View* view, m_updateViews ) {
    KTextEditor::Cursor c = view->cursorPosition();
    ///Find either a Declaration, or a use, that is in the Range.

    ///Pick a non-proxy context
    KDevelop::TopDUContext* chosen = 0;
    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    
    Declaration* foundDeclaration = 0;
    
    QList<KDevelop::TopDUContext*> contexts = KDevelop::DUChain::self()->chainsForDocument(view->document()->url());
    foreach( KDevelop::TopDUContext* ctx, contexts )
      if( !(ctx->flags() & KDevelop::TopDUContext::ProxyContextFlag) )
        chosen = ctx;

    if( chosen )
    {
      DUContext* ctx = chosen->findContextAt(c);
      if( ctx ) {
        //Try finding a declaration under the cursor
        foreach( Declaration* decl, ctx->localDeclarations() ) {
          if( decl->textRange().contains(c) ) {
            foundDeclaration = decl;
            break;
          }
        }
        //Try finding a use under the cursor
        foreach( Use* use, ctx->uses() ) {
          if( use->textRange().contains(c) ) {
            kDebug() << "found use" << use->textRange();
            foundDeclaration = use->declaration();
            break;
          }
        }
      } else {
        kDebug() << "Found specific context for cursor " << c;
      }
    } else {
      kDebug() << "Found no valid context";
    }
    
    if( m_highlightedDeclarations.contains(view) ) {
      ///Step 1: unhighlight the old uses
      changeHighlight( view, m_highlightedDeclarations[view].data(), false );
    }
    if( foundDeclaration ) {
      kDebug() << "found declaration";
      m_highlightedDeclarations[view] = foundDeclaration;
      changeHighlight( view, foundDeclaration, true );
    }else{
      kDebug() << "not found declaration";
      m_highlightedDeclarations.remove(view);
    }
  }
}

void UseHighlightPlugin::documentLoaded( KDevelop::IDocument* document )
{
  kDebug() << "connecting document";
  connect( document->textDocument(), SIGNAL(destroyed( QObject* )), this, SLOT( documentDestroyed( QObject* ) ) );
  connect( document->textDocument(), SIGNAL( viewCreated( KTextEditor::Document* , KTextEditor::View* ) ), this, SLOT( viewCreated( KTextEditor::Document*, KTextEditor::View* ) ) );

  foreach( KTextEditor::View* view, document->textDocument()->views() )
    viewCreated( document->textDocument(), view );
}

void UseHighlightPlugin::documentClosed( KDevelop::IDocument* document )
{
}

void UseHighlightPlugin::documentDestroyed( QObject* obj )
{
}

void UseHighlightPlugin::viewDestroyed( QObject* obj )
{
  m_highlightedDeclarations.remove( (KTextEditor::View*)obj );
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
