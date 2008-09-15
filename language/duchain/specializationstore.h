/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef SPECIALIZATIONSTORE_H
#define SPECIALIZATIONSTORE_H

#include <QtCore/qhash.h>
#include "../languageexport.h"

namespace KDevelop {
  class DeclarationId;
  
  /**
   * This class allows dynamic management of "current" specializations for declarations.
   * The specializations will be applied in editors, and wherever it makes sense.
   * For example, this is used in C++ to get code-completion and use-building within
   * instantiated template-classes/functions.
   */
  class KDEVPLATFORMLANGUAGE_EXPORT SpecializationStore {
    public:
      static SpecializationStore& self();
      
      /**
       * Adds/updates the current specialization for the given declaration-id
       * */
      void set(DeclarationId declaration, uint specialization);
      /**
       * Gets the registered specialization for the given declaration-id, or zero.
       */
      uint get(DeclarationId declaration);
      /**
       * Clears the specialization registered for the given declaration-id
       */
      void clear(DeclarationId declaration);
      /**
       * Clears all registered specializations
       */
      void clear();
      
    private:
      SpecializationStore();
      ~SpecializationStore();
      QHash<DeclarationId, uint> m_specializations;
  };
}

#endif
