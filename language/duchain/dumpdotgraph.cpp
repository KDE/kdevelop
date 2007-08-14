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

struct DumpDotGraphPrivate {
  QString dotGraphInternal(KDevelop::DUContext* contex, bool isMaster);
  QMap<KUrl,QString> m_hadVersions;
  QMap<KDevelop::DUContext*, bool> m_hadContexts;
  bool m_shortened;
};

QString DumpDotGraph::dotGraph(KDevelop::TopDUContext* context, bool shortened ) {
  d->m_shortened = shortened;
  d->m_hadContexts.clear();
  d->m_hadVersions.clear();
  return d->dotGraphInternal(context, true);
}

DumpDotGraph::DumpDotGraph() : d(new DumpDotGraphPrivate()) {
  
}

DumpDotGraph::~DumpDotGraph() {
  delete d;
}

QString DumpDotGraphPrivate::dotGraphInternal(KDevelop::DUContext* context, bool isMaster) {
  QTextStream stream;
  QString ret;
  stream.setString(&ret, QIODevice::WriteOnly);

  //Only work on each context once
  if( m_hadContexts.contains(context) )
    return QString();
  
  m_hadContexts[context] = true;
  
  if( isMaster )
    stream << "Digraph chain {\n";
  
  QString shape = "box";
  QString label = "unknown";
  
  if( dynamic_cast<TopDUContext*>(context) ) {
    shape = "parallelogram";
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
    label = "context " + context->scopeIdentifier().toString();
  }

  foreach (DUContextPointer parent, context->importedParentContexts()) {
    if( parent ) {
      stream << dotGraphInternal(parent.data(), false);
      stream << shortLabel(context) << " -> " << shortLabel(parent.data()) << "[style=dotted,label=\"imports\"];\n";
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

      stream << shortLabel(dec) <<
          "[shape=distortion,label=\"" <<
          dec->toString() << " [" <<
          dec->qualifiedIdentifier().toString() << "]" /*<<
          dec->textRange().()*/ << "\"];\n";
      stream << shortLabel(context) << " -> " << shortLabel(dec) << "[color=green];\n";
      if( dec->internalContext() )
        stream << shortLabel(dec) << " -> " << shortLabel(dec->internalContext()) << "[label=\"internal\", color=blue];\n";
    }
  }

  if( !context->localDefinitions().isEmpty() ) {
    label += QString(", %1 Df.").arg(context->localDefinitions().count());
  }
  
  if(!m_shortened ) {
    foreach (Definition* def, context->localDefinitions()) {

      stream << shortLabel(def) <<  "[shape=regular,label=\"" << (def->declaration() ? def->declaration()->toString() : QString("no declaration")) << /*dec->textRange() <<*/ "\"];\n";
      stream << shortLabel(context) << " -> " << shortLabel(def) << ";\n";
      if( def->declaration() )
        stream << shortLabel(def) << " -> " << shortLabel(def->declaration()) << "[label=\"defines\",color=green];\n";
    }
  }

  stream << shortLabel(context) << "[shape=" << shape << ",label=\"" << label << "\"" << (isMaster ? QString("color=red") : QString()) << "];\n";

  if( isMaster )
    stream << "}\n";
    
  return ret;
}
}
