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

#include "ducontext.h"
#include "topducontext.h"
#include "declaration.h"
#include "duchainpointer.h"
#include "parsingenvironment.h"
#include "identifier.h"
#include "functiondefinition.h"

namespace KDevelop {

QString shortLabel(KDevelop::DUContext* context) {
  return QString("q%1").arg((quint64)context);
}

QString shortLabel(KDevelop::Declaration* declaration) {
  return QString("q%1").arg((quint64)declaration);
}

QString rangeToString( const KTextEditor::Range& r ) {
  return QString("%1:%2->%3:%4").arg(r.start().line()).arg(r.start().column()).arg(r.end().line()).arg(r.end().column());
}
  

class DumpDotGraphPrivate
{
public:

  QString dotGraphInternal(KDevelop::DUContext* contex, bool isMaster, bool shortened);

  void addDeclaration(QTextStream& stream, Declaration* decl);
  
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

  Declaration* declarationForDefinition = 0;
  if(dynamic_cast<FunctionDefinition*>(dec))
    declarationForDefinition = static_cast<FunctionDefinition*>(dec)->declaration(m_topContext);

  if(!declarationForDefinition) {
    //Declaration
    stream << shortLabel(dec) <<
        "[shape=box, label=\"" <<
        dec->toString() << " [" <<
        dec->qualifiedIdentifier().toString() << "]" << " " <<
        rangeToString(dec->range().castToSimpleRange()) << "\"];\n";
    stream << shortLabel(dec->context()) << " -> " << shortLabel(dec) << "[color=green];\n";
    if( dec->internalContext() )
      stream << shortLabel(dec) << " -> " << shortLabel(dec->internalContext()) << "[label=\"internal\", color=blue];\n";
  }else{
    //Definition
    stream << shortLabel(dec) <<  "[shape=regular,color=yellow,label=\"" << declarationForDefinition->toString() << " "<< rangeToString(dec->range().castToSimpleRange()) <<  "\"];\n";
    stream << shortLabel(dec->context()) << " -> " << shortLabel(dec) << ";\n";
    
    stream << shortLabel(dec) << " -> " << shortLabel(declarationForDefinition) << "[label=\"defines\",color=green];\n";
    addDeclaration(stream, declarationForDefinition);

    if( dec->internalContext() )
      stream << shortLabel(dec) << " -> " << shortLabel(dec->internalContext()) << "[label=\"internal\", color=blue];\n";
  }
}


QString DumpDotGraphPrivate::dotGraphInternal(KDevelop::DUContext* context, bool isMaster, bool shortened) {
  if( m_hadObjects.contains(context) )
    return QString();

  m_hadObjects[context] = true;

  QTextStream stream;
  QString ret;
  stream.setString(&ret, QIODevice::WriteOnly);

  if( isMaster )
    stream << "Digraph chain {\n";
  
  QString shape = "parallelogram";
  //QString shape = "box";
  QString label = "unknown";
  
  if( dynamic_cast<TopDUContext*>(context) )
  {
    TopDUContext* topCtx = static_cast<TopDUContext*>(context);
    if( topCtx->parsingEnvironmentFile() ) {
      QString file( topCtx->parsingEnvironmentFile()->url().str() );

      KUrl url = KUrl(file);
      if(topCtx->parsingEnvironmentFile() && topCtx->parsingEnvironmentFile()->isProxyContext())
        url.addPath("_[proxy]_");
      
      //Find the context this one is derived from. If there is one, connect it with a line, and shorten the url.
      if( m_hadVersions.contains(url) ) {
        stream << shortLabel(context) << " -> " << m_hadVersions[url] << "[color=blue,label=\"version\"];\n";
        file = KUrl(file).fileName();
      } else {
        m_hadVersions[url] = shortLabel(context);
      }
      
      label = file;
      
      if( topCtx->importers().count() != 0 )
        label += QString(" imported by %1").arg(topCtx->importers().count());
    } else {
      label = "unknown file";
    }
    if(topCtx->parsingEnvironmentFile() && topCtx->parsingEnvironmentFile()->isProxyContext())
      label = "Proxy-context " + label;
  }else{
    label = /*"context " + */context->localScopeIdentifier().toString();
    label += ' ' + rangeToString(context->range().castToSimpleRange());
  }

  //label = QString("%1 ").arg((size_t)context) + label;

  if( isMaster && !dynamic_cast<TopDUContext*>(context) ) {
    //Also draw contexts that import this one
    foreach( DUContext* ctx, context->importers() )
      stream << dotGraphInternal(ctx, false, true);
  }
  
  foreach (const DUContext::Import &parent, context->importedParentContexts()) {
    if( parent.context(m_topContext) ) {
      stream << dotGraphInternal(parent.context(m_topContext), false, true);
      QString label = "imports";
      if( (!dynamic_cast<TopDUContext*>(parent.context(m_topContext)) || !dynamic_cast<TopDUContext*>(context)) && !(parent.context(m_topContext)->url() == context->url()) ) {
        label += " from " + KUrl(parent.context(m_topContext)->url().str()).fileName() + " to " + KUrl(context->url().str()).fileName();
      }
      
      stream << shortLabel(context) << " -> " << shortLabel(parent.context(m_topContext)) << "[style=dotted,label=\"" << label  << "\"];\n";
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
  }

  if( context->owner() ) {
    addDeclaration(stream, context->owner());
  }

  stream << shortLabel(context) << "[shape=" << shape << ",label=\"" << label << "\"" << (isMaster ? QString("color=red") : QString("color=blue")) << "];\n";

  if( isMaster )
    stream << "}\n";
    
  return ret;
}
}
