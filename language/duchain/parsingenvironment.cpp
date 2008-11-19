/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "parsingenvironment.h"
#include "topducontext.h"
#include "duchainregister.h"
#include "topducontextdynamicdata.h"
#include "duchain.h"
#include "topducontextdata.h"
#include <language/backgroundparser/parsejob.h>

namespace KDevelop
{
// REGISTER_DUCHAIN_ITEM(ParsingEnvironmentFile);

TopDUContext::Features ParsingEnvironmentFile::features() const {
  return d_func()->m_features;
}

void ParsingEnvironmentFile::setFeatures(TopDUContext::Features features) {
  d_func_dynamic()->m_features = features;
}

ParsingEnvironment::ParsingEnvironment() {
}

ParsingEnvironment::~ParsingEnvironment() {
}

ParsingEnvironmentFile::ParsingEnvironmentFile() : DUChainBase(*new ParsingEnvironmentFileData()) {
  d_func_dynamic()->setClassId(this);
}

TopDUContext* ParsingEnvironmentFile::topContext() const {
  return indexedTopContext().data();
}

ParsingEnvironmentFile::~ParsingEnvironmentFile() {
}

ParsingEnvironmentFile::ParsingEnvironmentFile(ParsingEnvironmentFileData& data) : DUChainBase(data) {
}

int ParsingEnvironment::type() const {
  return StandardParsingEnvironment;
}

int ParsingEnvironmentFile::type() const {
  return StandardParsingEnvironment;
}

bool ParsingEnvironmentFile::isProxyContext() const {
  return d_func()->m_isProxyContext;
}

void ParsingEnvironmentFile::setIsProxyContext(bool is) {
  d_func_dynamic()->m_isProxyContext = is;
}

QList< KSharedPtr<ParsingEnvironmentFile> > ParsingEnvironmentFile::imports() {
  
  QList<IndexedDUContext> imp;
  IndexedTopDUContext top = indexedTopContext();
  if(top.isLoaded()) {
    TopDUContext* topCtx = top.data();
    FOREACH_FUNCTION(const DUContext::Import& import, topCtx->d_func()->m_importedContexts)
      imp << import.indexedContext();
  }else{
    imp = TopDUContextDynamicData::loadImports(top.index());
  }
  
  QList< KSharedPtr<ParsingEnvironmentFile> > ret;
  foreach(IndexedDUContext ctx, imp)
    ret << DUChain::self()->environmentFileForDocument(ctx.topContextIndex());
  return ret;
}

///Returns the parsing-environment informations of all importers of the coupled TopDUContext. This is more efficient than
///loading the top-context and finding out, because when a top-context is loaded, also all its recursive imports are loaded
QList< KSharedPtr<ParsingEnvironmentFile> > ParsingEnvironmentFile::importers() {
  
  QList<IndexedDUContext> imp;
  IndexedTopDUContext top = indexedTopContext();
  if(top.isLoaded()) {
    TopDUContext* topCtx = top.data();
    FOREACH_FUNCTION(const IndexedDUContext& ctx, topCtx->d_func()->m_importers)
      imp << ctx;
  }else{
    imp = TopDUContextDynamicData::loadImporters(top.index());
  }
  
  QList< KSharedPtr<ParsingEnvironmentFile> > ret;
  foreach(IndexedDUContext ctx, imp)
    ret << DUChain::self()->environmentFileForDocument(ctx.topContextIndex());
  return ret;
}

///Makes sure the the file has the correct features attached, and if minimumFeatures contains AllDeclarationsContextsAndUsesForRecursive, then also checks all imports.
static bool featuresMatch(ParsingEnvironmentFilePointer file, TopDUContext::Features minimumFeatures, QSet<ParsingEnvironmentFilePointer>& checked) {
  if(checked.contains(file))
    return true;
  
  checked.insert(file);
  
  ///Locally we don't require the "recursive" condition, that only counts when we also have imports
  TopDUContext::Features localRequired = (TopDUContext::Features)(minimumFeatures & TopDUContext::AllDeclarationsContextsAndUses);
  localRequired = (TopDUContext::Features) (localRequired | ParseJob::staticMinimumFeatures(file->url()));
  if(!((file->features() & localRequired) == localRequired )) {
    return false;
  }
  
  if(minimumFeatures == TopDUContext::AllDeclarationsContextsAndUsesForRecursive || ParseJob::hasStaticMinimumFeatures())
    foreach(ParsingEnvironmentFilePointer import, file->imports())
      if(!featuresMatch(import, minimumFeatures == TopDUContext::AllDeclarationsContextsAndUsesForRecursive ? minimumFeatures : ((TopDUContext::Features)0), checked))
        return false;
  
  return true;
}

bool ParsingEnvironmentFile::featuresSatisfied(TopDUContext::Features minimumFeatures) {
  QSet<ParsingEnvironmentFilePointer> checked;
  return featuresMatch(ParsingEnvironmentFilePointer(this), minimumFeatures, checked);
}


} //KDevelop
