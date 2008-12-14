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
#include "duchainlock.h"
#include "topducontextdata.h"
#include <language/backgroundparser/parsejob.h>
#include <editor/modificationrevisionset.h>

#define ENSURE_READ_LOCKED   if(indexedTopContext().isValid()) { ENSURE_CHAIN_READ_LOCKED }
#define ENSURE_WRITE_LOCKED   if(indexedTopContext().isValid()) { ENSURE_CHAIN_READ_LOCKED }


namespace KDevelop
{
REGISTER_DUCHAIN_ITEM(ParsingEnvironmentFile);

TopDUContext::Features ParsingEnvironmentFile::features() const {
  ENSURE_READ_LOCKED
  
  return d_func()->m_features;
}

void ParsingEnvironmentFile::setFeatures(TopDUContext::Features features) {
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_features = features;
}

ParsingEnvironment::ParsingEnvironment() {
}

ParsingEnvironment::~ParsingEnvironment() {
}

IndexedString ParsingEnvironmentFile::url() const {
  ENSURE_READ_LOCKED
  return d_func()->m_url;
}

bool ParsingEnvironmentFile::needsUpdate() const {
  ENSURE_READ_LOCKED
  return d_func()->m_allModificationRevisions.needsUpdate();
}

bool ParsingEnvironmentFile::matchEnvironment(const ParsingEnvironment* /*environment*/) const {
  ENSURE_READ_LOCKED
  return true;
}

void ParsingEnvironmentFile::setTopContext(KDevelop::IndexedTopDUContext context) {
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_topContext = context;
}

KDevelop::IndexedTopDUContext ParsingEnvironmentFile::indexedTopContext() const {
  return d_func()->m_topContext;
}

const ModificationRevisionSet& ParsingEnvironmentFile::allModificationRevisions() const {
  ENSURE_READ_LOCKED
  return d_func()->m_allModificationRevisions;
}

void ParsingEnvironmentFile::addModificationRevisions(const ModificationRevisionSet& revisions) {
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_allModificationRevisions += revisions;
}

ParsingEnvironmentFile::ParsingEnvironmentFile(ParsingEnvironmentFileData& data, const IndexedString& url) : DUChainBase(data) {

  d_func_dynamic()->m_url = url;
  d_func_dynamic()->m_modificationTime = ModificationRevision::revisionForFile(url);
  
  addModificationRevision(url, d_func_dynamic()->m_modificationTime);
  Q_ASSERT(d_func()->m_allModificationRevisions.index());
}

ParsingEnvironmentFile::ParsingEnvironmentFile(const IndexedString& url) : DUChainBase(*new ParsingEnvironmentFileData()) {
  d_func_dynamic()->setClassId(this);

  d_func_dynamic()->m_url = url;
  d_func_dynamic()->m_modificationTime = ModificationRevision::revisionForFile(url);
  
  addModificationRevision(url, d_func_dynamic()->m_modificationTime);
  Q_ASSERT(d_func()->m_allModificationRevisions.index());
}

TopDUContext* ParsingEnvironmentFile::topContext() const {
  ENSURE_READ_LOCKED
  return indexedTopContext().data();
}

ParsingEnvironmentFile::~ParsingEnvironmentFile() {
}

ParsingEnvironmentFile::ParsingEnvironmentFile(ParsingEnvironmentFileData& data) : DUChainBase(data) {
  //If this triggers, the item has most probably not been initialized with the correct constructor that takes an IndexedString.
  Q_ASSERT(d_func()->m_allModificationRevisions.index());
}

int ParsingEnvironment::type() const {
  return StandardParsingEnvironment;
}

int ParsingEnvironmentFile::type() const {
  ENSURE_READ_LOCKED
  return StandardParsingEnvironment;
}

bool ParsingEnvironmentFile::isProxyContext() const {
  ENSURE_READ_LOCKED
  return d_func()->m_isProxyContext;
}

void ParsingEnvironmentFile::setIsProxyContext(bool is) {
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_isProxyContext = is;
}

QList< KSharedPtr<ParsingEnvironmentFile> > ParsingEnvironmentFile::imports() {
  ENSURE_READ_LOCKED

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
  foreach(const IndexedDUContext &ctx, imp)
    ret << DUChain::self()->environmentFileForDocument(ctx.topContextIndex());
  return ret;
}

QList< KSharedPtr<ParsingEnvironmentFile> > ParsingEnvironmentFile::importers() {
  ENSURE_READ_LOCKED
  
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
  foreach(const IndexedDUContext &ctx, imp)
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
  
  ///@todo Before recursing, do a fast check whether one of the imports has special rules stored. Else it's not neede.
  if(minimumFeatures == TopDUContext::AllDeclarationsContextsAndUsesForRecursive || ParseJob::hasStaticMinimumFeatures())
    foreach(const ParsingEnvironmentFilePointer &import, file->imports())
      if(!featuresMatch(import, minimumFeatures == TopDUContext::AllDeclarationsContextsAndUsesForRecursive ? minimumFeatures : ((TopDUContext::Features)0), checked))
        return false;
  
  return true;
}

bool ParsingEnvironmentFile::featuresSatisfied(TopDUContext::Features minimumFeatures) {
  ENSURE_READ_LOCKED
  QSet<ParsingEnvironmentFilePointer> checked;
  return featuresMatch(ParsingEnvironmentFilePointer(this), minimumFeatures, checked);
}

void ParsingEnvironmentFile::clearModificationRevisions() {
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_allModificationRevisions.clear();
  d_func_dynamic()->m_allModificationRevisions.addModificationRevision(d_func()->m_url, d_func()->m_modificationTime);
}

void ParsingEnvironmentFile::addModificationRevision(const IndexedString& url, const ModificationRevision& revision) {
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_allModificationRevisions.addModificationRevision(url, revision);
  {
    //Test
    Q_ASSERT(d_func_dynamic()->m_allModificationRevisions.index());
    Q_ASSERT(d_func_dynamic()->m_allModificationRevisions.removeModificationRevision(url, revision) );
    d_func_dynamic()->m_allModificationRevisions.addModificationRevision(url, revision);
  }
}

void ParsingEnvironmentFile::setModificationRevision( const KDevelop::ModificationRevision& rev ) {
  ENSURE_WRITE_LOCKED

  Q_ASSERT(d_func_dynamic()->m_allModificationRevisions.index());
  Q_ASSERT( d_func_dynamic()->m_allModificationRevisions.removeModificationRevision(d_func()->m_url, d_func()->m_modificationTime) );
  
  #ifdef LEXERCACHE_DEBUG
  if(debugging()) {
  kDebug( 9007 ) <<  id(this) << "setting modification-revision" << rev.toString();
  }
#endif
  d_func_dynamic()->m_modificationTime = rev;
#ifdef LEXERCACHE_DEBUG
  if(debugging()) {
  kDebug( 9007 ) <<  id(this) << "new modification-revision" << m_modificationTime;
  }
#endif
  d_func_dynamic()->m_allModificationRevisions.addModificationRevision(d_func()->m_url, d_func()->m_modificationTime);
}

KDevelop::ModificationRevision ParsingEnvironmentFile::modificationRevision() const {
  ENSURE_READ_LOCKED
  return d_func()->m_modificationTime;
}

} //KDevelop
