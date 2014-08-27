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

#ifndef KDEVPLATFORM_INSTANTIATIONINFORMATION_H
#define KDEVPLATFORM_INSTANTIATIONINFORMATION_H

#include <language/languageexport.h>
#include "types/abstracttype.h"
#include "types/indexedtype.h"
#include "appendedlist.h"

#include <serialization/referencecounting.h>

namespace KDevelop {
  class IndexedInstantiationInformation;
  class InstantiationInformation;
  class QualifiedIdentifier;

  KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(InstantiationInformation, templateParameters, IndexedType)

  class KDEVPLATFORMLANGUAGE_EXPORT IndexedInstantiationInformation : public ReferenceCountManager {
    public:
      IndexedInstantiationInformation();
      explicit IndexedInstantiationInformation(uint index);
      IndexedInstantiationInformation(const IndexedInstantiationInformation& rhs);
      IndexedInstantiationInformation& operator=(const IndexedInstantiationInformation& rhs);
      ~IndexedInstantiationInformation();

      const InstantiationInformation& information() const;

      uint hash() const {
        return m_index * 73;
      }

      //Is always zero for the empty information
      uint index() const {
        return m_index;
      }

      bool operator==(const IndexedInstantiationInformation& rhs) const {
        return m_index == rhs.m_index;
      }

      //Returns true if one of the values represented by this information is non-default
      bool isValid() const;

    private:
      uint m_index;
  };

  class KDEVPLATFORMLANGUAGE_EXPORT InstantiationInformation {
  public:

    InstantiationInformation();
    ///@todo include some information for instantiation only with default parameters
    InstantiationInformation(const InstantiationInformation& rhs, bool dynamic = true);

    ~InstantiationInformation();

    InstantiationInformation& operator=(const InstantiationInformation& rhs);

    bool operator==(const InstantiationInformation& rhs) const;

    uint hash() const;

    bool isValid() const {
      return previousInstantiationInformation.index() || templateParametersSize();
    }

    bool persistent() const {
      return (bool)m_refCount;
    }

    /**
     * Applies this instantiation information to the given QualifiedIdentifier.
     *
     * The template parameters of the qualified identifier will be marked as expressions,
     * thus the qualified identifier is not "clean".
     *
     * Should only be used for displaying.
     *
     * This only adds template-parameters in places where none are known yet.
     */
    QualifiedIdentifier applyToIdentifier(const QualifiedIdentifier& id) const;

    /**
     * @param local If this is true, only the template-parameters of this scope are printed,
     *              but not the parent ones
     */
    QString toString(bool local = false) const;

    /**
     * This must always be used to add new parameters.
     *
     * @warning Never use @c templateParametersList() directly.
     */
    void addTemplateParameter(AbstractType::Ptr type);

    /**
     * Instantiation-information for the surrounding context(see IndexedInstantiationInformation).
     */
    IndexedInstantiationInformation previousInstantiationInformation;

    START_APPENDED_LISTS(InstantiationInformation)

    static uint classSize() {
      return sizeof(InstantiationInformation);
    }

    short unsigned int itemSize() const {
      return dynamicSize();
    }

    /**
     * templateParameters contains the template-parameters used for the instantiation
     */
    APPENDED_LIST_FIRST(InstantiationInformation, IndexedType, templateParameters);

    END_APPENDED_LISTS(InstantiationInformation, templateParameters);

    IndexedInstantiationInformation indexed() const;
    private:
      friend class IndexedInstantiationInformation;
      uint m_refCount;
  };

  inline uint qHash(const IndexedInstantiationInformation& info) {
    return info.hash();
  }
  inline uint qHash(const InstantiationInformation& info) {
    return info.hash();
  }
}

Q_DECLARE_TYPEINFO(KDevelop::IndexedInstantiationInformation, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::InstantiationInformation, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_INSTANTIATIONINFORMATION_H
