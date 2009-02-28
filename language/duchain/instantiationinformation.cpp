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


namespace KDevelop {

DEFINE_LIST_MEMBER_HASH(InstantiationInformation, templateParameters, IndexedType)

QualifiedIdentifier InstantiationInformation::applyToIdentifier(const QualifiedIdentifier& id) const {
  QualifiedIdentifier ret;
  if(id.count() > 1) {
    ret = id;
    ret.pop();
    if(previousInstantiationInformation)
      ret = IndexedInstantiationInformation(previousInstantiationInformation).information().applyToIdentifier(ret);
  }

  Identifier lastId(id.last());

  KDevVarLengthArray<TypeIdentifier> oldTemplateIdentifiers;
  for(uint a = 0; a < lastId.templateIdentifiersCount(); ++a)
    oldTemplateIdentifiers.append(lastId.templateIdentifier(a));
  lastId.clearTemplateIdentifiers();

  for(uint a = 0; a < templateParametersSize(); ++a) {
    if(templateParameters()[a].type()) {
      TypeIdentifier id(templateParameters()[a].type()->toString());
      id.setIsExpression(true);
      lastId.appendTemplateIdentifier(id);
    }else{
      lastId.appendTemplateIdentifier(oldTemplateIdentifiers.size() > a ? oldTemplateIdentifiers[a] : TypeIdentifier());
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
    if(previousInstantiationInformation && !local)
        ret = IndexedInstantiationInformation(previousInstantiationInformation).information().toString() + "::";
    ret += '<';
    for(int a = 0; a < templateParametersSize(); ++a) {
        if(a)
            ret += ", ";
        if(templateParameters()[a].type())
          ret += templateParameters()[a].type()->toString();
    }
    ret += '>';
    return ret;
}

InstantiationInformation::InstantiationInformation() : previousInstantiationInformation(0) {
  initializeAppendedLists();
}

InstantiationInformation::InstantiationInformation(const InstantiationInformation& rhs) : previousInstantiationInformation(rhs.previousInstantiationInformation) {
  initializeAppendedLists();
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
  if(previousInstantiationInformation != rhs.previousInstantiationInformation)
    return false;
  return listsEqual(rhs);
}

uint InstantiationInformation::hash() const {
  uint ret = 0;
  FOREACH_FUNCTION(const IndexedType& param, templateParameters) {
    ret = (ret + param.hash()) * 117;
  }

  return (ret + previousInstantiationInformation) * 31;
}

KDevelop::ItemRepository<InstantiationInformation, AppendedListItemRequest<InstantiationInformation> > instantiationInformationRepository("C++ Instantiation Information Repository");


const uint standardInstantiationInformationIndex = instantiationInformationRepository.index( InstantiationInformation() );

IndexedInstantiationInformation::IndexedInstantiationInformation() : m_index(0) {
}

IndexedInstantiationInformation::IndexedInstantiationInformation(uint index) : m_index(index) {
  if(m_index == standardInstantiationInformationIndex)
    m_index = 0;
}

bool IndexedInstantiationInformation::isValid() const {
  return m_index;
}

const InstantiationInformation& IndexedInstantiationInformation::information() const {
  return *instantiationInformationRepository.itemFromIndex(m_index ? m_index : standardInstantiationInformationIndex);
}

IndexedInstantiationInformation InstantiationInformation::indexed() const {
  return IndexedInstantiationInformation(instantiationInformationRepository.index(*this));
}
}
