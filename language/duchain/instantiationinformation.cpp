/* This file is part of KDevelop
    Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "./instantiationinformation.h"
#include "identifier.h"
#include "repositories/itemrepository.h"
#include "types/typeutils.h"
#include <typeinfo>
#include "types/typealiastype.h"
#include "repositories/typerepository.h"


namespace KDevelop {

DEFINE_LIST_MEMBER_HASH(InstantiationInformation, templateParameters, IndexedType)

QualifiedIdentifier InstantiationInformation::applyToIdentifier(const QualifiedIdentifier& id) const {
  QualifiedIdentifier ret;
  if(id.count() > 1) {
    ret = id;
    ret.pop();
    if(previousInstantiationInformation.index())
      ret = previousInstantiationInformation.information().applyToIdentifier(ret);
  }

  Identifier lastId(id.last());

  KDevVarLengthArray<IndexedTypeIdentifier> oldTemplateIdentifiers;
  for(uint a = 0; a < lastId.templateIdentifiersCount(); ++a)
    oldTemplateIdentifiers.append(lastId.templateIdentifier(a)); 
  lastId.clearTemplateIdentifiers();

  for(uint a = 0; a < templateParametersSize(); ++a) {
    if(templateParameters()[a].abstractType()) {
      lastId.appendTemplateIdentifier(IndexedTypeIdentifier(templateParameters()[a].abstractType()->toString(), true));
    }else{
      lastId.appendTemplateIdentifier((uint) oldTemplateIdentifiers.size() > a ? oldTemplateIdentifiers[a] : IndexedTypeIdentifier());
    }
  }

  for(int a = templateParametersSize(); a < oldTemplateIdentifiers.size(); ++a)
    lastId.appendTemplateIdentifier(oldTemplateIdentifiers[a]);

  ret.push(lastId);
  return ret;
}

void InstantiationInformation::addTemplateParameter(KDevelop::AbstractType::Ptr type) {
  templateParametersList().append(type->indexed());
}

QString InstantiationInformation::toString(bool local) const {
    QString ret;
    if(previousInstantiationInformation.index() && !local)
        ret = previousInstantiationInformation.information().toString() + "::";
    ret += '<';
    for(uint a = 0; a < templateParametersSize(); ++a) {
        if(a)
            ret += ", ";
        if(templateParameters()[a].abstractType())
          ret += templateParameters()[a].abstractType()->toString();
    }
    ret += '>';
    return ret;
}

InstantiationInformation::InstantiationInformation() : m_refCount(0) {
  initializeAppendedLists();
}

InstantiationInformation::InstantiationInformation(const InstantiationInformation& rhs, bool dynamic) : previousInstantiationInformation(rhs.previousInstantiationInformation), m_refCount(0) {
  initializeAppendedLists(dynamic);
  copyListsFrom(rhs);
}

InstantiationInformation::~InstantiationInformation() {
  freeAppendedLists();
}

InstantiationInformation& InstantiationInformation::operator=(const InstantiationInformation& rhs) {
  previousInstantiationInformation = rhs.previousInstantiationInformation;
  copyListsFrom(rhs);
  return *this;
}

bool InstantiationInformation::operator==(const InstantiationInformation& rhs) const {
  if(!(previousInstantiationInformation == rhs.previousInstantiationInformation))
    return false;
  return listsEqual(rhs);
}

uint InstantiationInformation::hash() const {
  KDevHash kdevhash;
  FOREACH_FUNCTION(const IndexedType& param, templateParameters) {
    kdevhash << param.hash();
  }
  return kdevhash << previousInstantiationInformation.index();
}

AbstractRepositoryManager* returnTypeRepository() {
  return typeRepositoryManager();
}

static KDevelop::RepositoryManager<KDevelop::ItemRepository<InstantiationInformation, AppendedListItemRequest<InstantiationInformation> > >& instantiationInformationRepository() {
    static KDevelop::RepositoryManager<KDevelop::ItemRepository<InstantiationInformation, AppendedListItemRequest<InstantiationInformation> > > instantiationInformationRepositoryObject("Instantiation Information Repository", 1, &returnTypeRepository);
    return instantiationInformationRepositoryObject;
}

uint standardInstantiationInformationIndex() {
  static uint idx = instantiationInformationRepository()->index( InstantiationInformation() );
  return idx;
}

void initInstantiationInformationRepository() {
  standardInstantiationInformationIndex();
}

IndexedInstantiationInformation::IndexedInstantiationInformation() : m_index(0) {
}

IndexedInstantiationInformation::IndexedInstantiationInformation(uint index) : m_index(index) {
  if(m_index == standardInstantiationInformationIndex())
    m_index = 0;
  
  if(m_index && shouldDoDUChainReferenceCounting(this))
  {
    QMutexLocker lock(instantiationInformationRepository()->mutex());
    increase(instantiationInformationRepository()->dynamicItemFromIndexSimple(m_index)->m_refCount, m_index);
  }
}

IndexedInstantiationInformation::IndexedInstantiationInformation(const IndexedInstantiationInformation& rhs) : m_index(rhs.m_index) {
  if(m_index && shouldDoDUChainReferenceCounting(this))
  {
    QMutexLocker lock(instantiationInformationRepository()->mutex());
    increase(instantiationInformationRepository()->dynamicItemFromIndexSimple(m_index)->m_refCount, m_index);
  }
}

IndexedInstantiationInformation& IndexedInstantiationInformation::operator=(const IndexedInstantiationInformation& rhs) {

  if(m_index && shouldDoDUChainReferenceCounting(this))
  {
    QMutexLocker lock(instantiationInformationRepository()->mutex());
    decrease(instantiationInformationRepository()->dynamicItemFromIndexSimple(m_index)->m_refCount, m_index);
  }
  
  m_index = rhs.m_index;
  
  if(m_index && shouldDoDUChainReferenceCounting(this))
  {
    QMutexLocker lock(instantiationInformationRepository()->mutex());
    increase(instantiationInformationRepository()->dynamicItemFromIndexSimple(m_index)->m_refCount, m_index);
  }
  return *this;
}

IndexedInstantiationInformation::~IndexedInstantiationInformation() {
  if(m_index && shouldDoDUChainReferenceCounting(this))
  {
    QMutexLocker lock(instantiationInformationRepository()->mutex());
    decrease(instantiationInformationRepository()->dynamicItemFromIndexSimple(m_index)->m_refCount, m_index);
  }
}

bool IndexedInstantiationInformation::isValid() const {
  return m_index;
}

const InstantiationInformation& IndexedInstantiationInformation::information() const {
  return *instantiationInformationRepository()->itemFromIndex(m_index ? m_index : standardInstantiationInformationIndex());
}

IndexedInstantiationInformation InstantiationInformation::indexed() const {
  return IndexedInstantiationInformation(instantiationInformationRepository()->index(*this));
}
}
