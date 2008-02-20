/***************************************************************************
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dumpdotgraph.h"

#include <ducontext.h>
#include <topducontext.h>
#include <declaration.h>
#include <definition.h>
#include <duchainpointer.h>
#include <parsingenvironment.h>
#include <identifier.h>

namespace KDevelop {

QString shortLabel(KDevelop::DUContext* context) {
  return QString("q%1").arg((quint64)context);
}

QString shortLabel(KDevelop::Declaration* declaration) {
  return QString("q%1").arg((quint64)declaration);
}

QString shortLabel(KDevelop::Definition* definition) {
  return QString("q%1").arg((quint64)definition);
}

QString rangeToString( const KTextEditor::Range& r ) {
  return QString("%1:%2->%3:%4").arg(r.start().line()).arg(r.start().column()).arg(r.end().line()).arg(r.end().column());
}
  

struct DumpDotGraphPrivate {
  QString dotGraphInternal(KDevelop::DUContext* contex, bool isMaster, bool shortened);

  void addDeclaration(QTextStream& stream, Declaration* decl);
  void addDefinition(QTextStream& stream, Definition* decl);
  
  QMap<KUrl,QString> m_hadVersions;
  QMap<KDevelop::DUChainBase*, bool> m_hadObjects;
  TopDUContext* m_topContext;
};

QString DumpDotGraph::dotGraph(KDevelop::DUContext* context, bool shortened ) {
  d->m_hadObjects.clear();
  d->m_hadVersions.clear();
  d->m_topContext = context->topContext(); ///@todo maybe get this as a parameter
  return d->dotGraphInternal(context, true, shortened);
}

DumpDotGraph::DumpDotGraph() : d(new DumpDotGraphPrivate()) {
  
}

DumpDotGraph::~DumpDotGraph() {
  delete d;
}

void DumpDotGraphPrivate::addDeclaration(QTextStream& stream, Declaration* dec) {
  if( m_hadObjects.contains(dec) )
    return;

  m_hadObjects[dec] = true;

  stream << shortLabel(dec) <<
      "[shape=distortion,label=\"" <<
      dec->toString() << " [" <<
      dec->qualifiedIdentifier().toString() << "]" << " " <<
      rangeToString(dec->range().textRange()) << "\"];\n";
  stream << shortLabel(dec->context()) << " -> " << shortLabel(dec) << "[color=green];\n";
  if( dec->internalContext() )
    stream << shortLabel(dec) << " -> " << shortLabel(dec->internalContext()) << "[label=\"internal\", color=blue];\n";
}

void DumpDotGraphPrivate::addDefinition(QTextStream& stream, Definition* def) {
  if( m_hadObjects.contains(def) )
    return;

  m_hadObjects[def] = true;
  
  stream << shortLabel(def) <<  "[shape=regular,color=yellow,label=\"" << (def->declaration(m_topContext) ? def->declaration(m_topContext)->toString() : QString("no declaration")) << " "<< rangeToString(def->range().textRange()) <<  "\"];\n";
  stream << shortLabel(def->context()) << " -> " << shortLabel(def) << ";\n";
  if( def->declaration(m_topContext) ) {
    stream << shortLabel(def) << " -> " << shortLabel(def->declaration(m_topContext)) << "[label=\"defines\",color=green];\n";
    addDeclaration(stream, def->declaration(m_topContext));
  }

  if( def->internalContext() )
    stream << shortLabel(def) << " -> " << shortLabel(def->internalContext()) << "[label=\"internal\", color=blue];\n";
}


QString DumpDotGraphPrivate::dotGraphInternal(KDevelop::DUContext* context, bool isMaster, bool shortened) {
  if( m_hadObjects.contains(context) )
    return QString();

  m_hadObjects[context] = true;
  
  QTextStream stream;
  QString ret;
  stream.setString(&ret, QIODevice::WriteOnly);

  //Only work on each context once
  if( m_hadObjects.contains(context) )
    return QString();
  
  m_hadObjects[context] = true;
  
  if( isMaster )
    stream << "Digraph chain {\n";
  
  QString shape = "parallelogram";
  //QString shape = "box";
  QString label = "unknown";
  
  if( dynamic_cast<TopDUContext*>(context) )
  {
    TopDUContext* topCtx = static_cast<TopDUContext*>(context);
    if( topCtx->parsingEnvironmentFile() ) {
      IdentifiedFile file( topCtx->parsingEnvironmentFile()->identity() );

      KUrl url = KUrl(file.url().str());
      if(topCtx->flags() & TopDUContext::ProxyContextFlag)
        url.addPath("_[proxy]_");
      
      //Find the context this one is derived from. If there is one, connect it with a line, and shorten the url.
      if( m_hadVersions.contains(url) ) {
        stream << shortLabel(context) << " -> " << m_hadVersions[url] << "[color=blue,label=\"version\"];\n";
        file = IdentifiedFile( HashedString( KUrl(file.url().str()).fileName() ), file.identity() );
      } else {
        m_hadVersions[url] = shortLabel(context);
      }
      
      label = file.toString();
      
      if( topCtx->importedChildContexts().count() != 0 )
        label += QString(" imported by %1").arg(topCtx->importedChildContexts().count());
    } else {
      label = "unknown file";
    }
    if(topCtx->flags() & TopDUContext::ProxyContextFlag)
      label = "Proxy-context " + label;
  }else{
    label = /*"context " + */context->localScopeIdentifier().toString();
    label += ' ' + rangeToString(context->range().textRange());
  }

  //label = QString("%1 ").arg((size_t)context) + label;

  if( isMaster && !dynamic_cast<TopDUContext*>(context) ) {
    //Also draw contexts that import this one
    foreach( DUContext* ctx, context->importedChildContexts() )
      stream << dotGraphInternal(ctx, false, true);
  }
  
  foreach (DUContextPointer parent, context->importedParentContexts()) {
    if( parent ) {
      stream << dotGraphInternal(parent.data(), false, true);
      QString label = "imports";
      if( (!dynamic_cast<TopDUContext*>(parent.data()) || !dynamic_cast<TopDUContext*>(context)) && !(parent->url() == context->url()) ) {
        label += " from " + KUrl(parent->url().str()).fileName() + " to " + KUrl(context->url().str()).fileName();
      }
      
      stream << shortLabel(context) << " -> " << shortLabel(parent.data()) << "[style=dotted,label=\"" << label  << "\"];\n";
    }
  }

  if( !context->childContexts().isEmpty() )
    label += QString(", %1 C.").arg(context->childContexts().count());

  if( !shortened ) {
    foreach (DUContext* child, context->childContexts()) {
      stream << dotGraphInternal(child, false, false);
      stream << shortLabel(context) << " -> " << shortLabel(child) << "[style=dotted,color=green];\n";
    }
  }

  if( !context->localDeclarations().isEmpty() )
    label += QString(", %1 D.").arg(context->localDeclarations().count());
  
  if(!shortened )
  {
    foreach (Declaration* dec, context->localDeclarations())
      addDeclaration(stream, dec);
    
    foreach (Definition* def, context->localDefinitions())
      addDefinition(stream, def);
  }

  if( context->owner() ) {
    if( context->owner()->asDeclaration() )
      addDeclaration(stream, context->owner()->asDeclaration());
    else if(context->owner()->asDefinition() )
      addDefinition(stream, context->owner()->asDefinition());
  }

  if( !context->localDefinitions().isEmpty() ) {
    label += QString(", %1 Df.").arg(context->localDefinitions().count());
  }
  
  stream << shortLabel(context) << "[shape=" << shape << ",label=\"" << label << "\"" << (isMaster ? QString("color=red") : QString("color=blue")) << "];\n";

  if( isMaster )
    stream << "}\n";
    
  return ret;
}
}
