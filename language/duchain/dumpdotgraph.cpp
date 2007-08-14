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
  return QString("q%1").arg((uint)context);
}

QString shortLabel(KDevelop::Declaration* declaration) {
  return QString("q%1").arg((uint)declaration);
}

QString shortLabel(KDevelop::Definition* definition) {
  return QString("q%1").arg((uint)definition);
}

QString rangeToString( const KTextEditor::Range& r ) {
  return QString("%1:%2->%3:%4").arg(r.start().line()).arg(r.start().column()).arg(r.end().line()).arg(r.end().column());
}
  

struct DumpDotGraphPrivate {
  QString dotGraphInternal(KDevelop::DUContext* contex, bool isMaster);

  void addDeclaraation(QTextStream& stream, Declaration* decl);
  
  QMap<KUrl,QString> m_hadVersions;
  QMap<KDevelop::DUChainBase*, bool> m_hadObjects;
  bool m_shortened;
};

QString DumpDotGraph::dotGraph(KDevelop::DUContext* context, bool shortened ) {
  d->m_shortened = shortened;
  d->m_hadObjects.clear();
  d->m_hadVersions.clear();
  return d->dotGraphInternal(context, true);
}

DumpDotGraph::DumpDotGraph() : d(new DumpDotGraphPrivate()) {
  
}

DumpDotGraph::~DumpDotGraph() {
  delete d;
}

void DumpDotGraphPrivate::addDeclaraation(QTextStream& stream, Declaration* dec) {
  if( m_hadObjects.contains(dec) )
    return;

  m_hadObjects[dec] = true;

  stream << shortLabel(dec) <<
      "[shape=distortion,label=\"" <<
      dec->toString() << " [" <<
      dec->qualifiedIdentifier().toString() << "]" << " " <<
      rangeToString(dec->textRange()) << "\"];\n";
  stream << shortLabel(dec->context()) << " -> " << shortLabel(dec) << "[color=green];\n";
  if( dec->internalContext() )
    stream << shortLabel(dec) << " -> " << shortLabel(dec->internalContext()) << "[label=\"internal\", color=blue];\n";
}

QString DumpDotGraphPrivate::dotGraphInternal(KDevelop::DUContext* context, bool isMaster) {
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
  
  if( dynamic_cast<TopDUContext*>(context) ) {
    if( static_cast<TopDUContext*>(context)->parsingEnvironmentFile() ) {
      IdentifiedFile file( static_cast<TopDUContext*>(context)->parsingEnvironmentFile()->identity() );

      //Find the context this one is derived from. If there is one, connect it with a line, and shorten the url.
      if( m_hadVersions.contains(file.url()) ) {
        stream << shortLabel(context) << " -> " << m_hadVersions[file.url()] << "[color=blue,label=\"version\"];\n";
        file = IdentifiedFile( KUrl(file.url().fileName()), file.identity() );
      } else {
        m_hadVersions[file.url()] = shortLabel(context);
      }
      label = file.toString();
    } else {
      label = "unknown file";
    }
  }else{
    label = /*"context " + */context->localScopeIdentifier().toString();
  }

  label += " " + rangeToString(context->textRange());

  if( isMaster && !dynamic_cast<TopDUContext*>(context) ) {
    //Also draw contexts that import this one
    foreach( DUContext* ctx, context->importedChildContexts() )
      stream << dotGraphInternal(ctx, false);
  }
  
  foreach (DUContextPointer parent, context->importedParentContexts()) {
    if( parent ) {
      stream << dotGraphInternal(parent.data(), false);
      QString label = "imports";
      if( (!dynamic_cast<TopDUContext*>(parent.data()) || !dynamic_cast<TopDUContext*>(context)) && parent->url() != context->url() ) {
        label += " from " + parent->url().fileName() + " to " + context->url().fileName();
      }
      
      stream << shortLabel(context) << " -> " << shortLabel(parent.data()) << "[style=dotted,label=\"" << label  << "\"];\n";
    }
  }

  if( !context->childContexts().isEmpty() )
    label += QString(", %1 C.").arg(context->childContexts().count());

  if( !m_shortened ) {
    foreach (DUContext* child, context->childContexts()) {
      stream << dotGraphInternal(child, false);
      stream << shortLabel(context) << " -> " << shortLabel(child) << "[style=dotted,color=green];\n";
    }
  }

  if( !context->localDeclarations().isEmpty() )
    label += QString(", %1 D.").arg(context->localDeclarations().count());
  
  if(!m_shortened ) {
    foreach (Declaration* dec, context->localDeclarations()) {
      addDeclaraation(stream, dec);
    }
  }

  if( context->declaration() )
    addDeclaraation(stream, context->declaration());

  if( !context->localDefinitions().isEmpty() ) {
    label += QString(", %1 Df.").arg(context->localDefinitions().count());
  }
  
  if(!m_shortened ) {
    foreach (Definition* def, context->localDefinitions()) {

      stream << shortLabel(def) <<  "[shape=regular,label=\"" << (def->declaration() ? def->declaration()->toString() : QString("no declaration")) << " "<< rangeToString(def->textRange()) <<  "\"];\n";
      stream << shortLabel(context) << " -> " << shortLabel(def) << ";\n";
      if( def->declaration() )
        stream << shortLabel(def) << " -> " << shortLabel(def->declaration()) << "[label=\"defines\",color=green];\n";
    }
  }

  stream << shortLabel(context) << "[shape=" << shape << ",label=\"" << label << "\"" << (isMaster ? QString("color=red") : QString("color=blue")) << "];\n";

  if( isMaster )
    stream << "}\n";
    
  return ret;
}
}
