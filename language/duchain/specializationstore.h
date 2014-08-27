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

#ifndef KDEVPLATFORM_SPECIALIZATIONSTORE_H
#define KDEVPLATFORM_SPECIALIZATIONSTORE_H

#include <QtCore/QHash>

#include "instantiationinformation.h"

#include <language/languageexport.h>

namespace KDevelop {
  class DeclarationId;
  class Declaration;
  class DUContext;
  class TopDUContext;

  /**
   * This class allows dynamic management of "current" specializations for declarations.
   *
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
      void set(const DeclarationId& declaration, const IndexedInstantiationInformation& specialization);
      /**
       * Gets the registered specialization for the given declaration-id, or zero.
       */
      IndexedInstantiationInformation get(const DeclarationId& declaration);
      /**
       * Clears the specialization registered for the given declaration-id
       */
      void clear(const DeclarationId& declaration);
      /**
       * Clears all registered specializations
       */
      void clear();

      /**
       * Applies the known specializations for the given declaration using the Declaration::specialize() function.
       *
       * If no specializations are known, the original declaration is returned.
       *
       * @param declaration The declaration to specialize
       * @param source The top-context from where to start searching
       * @param recursive Whether parent-contexts should be checked for known specializations, and those applied.
       *                  This is a bit more expensive then just doing a local check.
       */
      KDevelop::Declaration* applySpecialization(KDevelop::Declaration* declaration,
                                                 KDevelop::TopDUContext* source, bool recursive = true);
      /**
       * Applies the known specializations for the given context using the DUContext::specialize() function.
       *
       * If no specializations are known, returns the original context.
       *
       * @param context The context to specialize
       * @param source The top-context from where to start searching
       * @param recursive Whether parent-contexts should be checked for known specializations, and those applied.
       *                  This is a bit more expensive then just doing a local check.
       */
      DUContext* applySpecialization(KDevelop::DUContext* context,
                                     KDevelop::TopDUContext* source,
                                     bool recursive = true);

    private:
      SpecializationStore();
      ~SpecializationStore();
      QHash<DeclarationId, IndexedInstantiationInformation> m_specializations;
  };
}

#endif
