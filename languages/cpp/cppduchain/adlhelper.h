/*
    Copyright (C) 2010 Ciprian Ciubotariu <cheepeero@gmx.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef CPP_ADLHELPER_H
#define CPP_ADLHELPER_H

#include <QSet>
#include "overloadresolution.h"

namespace Cpp
{

/**
 * @brief Used for implementing ADL lookup.
 * See ISO 14882.2003, section 3.4.2 Argument-dependent name lookup [basic.lookup.koenig]
 *
 * Add all function arguments (or their types) to an object.
 * Uninteresting arguments and types are skipped by the helper.
 * If an argument is found to match conditions for ADL it is added to the associated namespace list.
 *
 * @todo Optimize. Seen types/classes/function declarations (which involve more searches)
 * can be stored somewhere and their associated namespace list reused.
 */
class ADLHelper
{
public:
  /**
   * @copydoc Cpp::OverloadResolver::OverloadResolver()
   */
  ADLHelper( DUContextPointer context, TopDUContextPointer topContext );

  /** @brief Adds an function argument for lookup. */
  void addArgument( const OverloadResolver::Parameter & argument );

  /** @brief Adds an function argument type for lookup. */
  void addArgumentType( const AbstractType::Ptr type );

  /** @brief Retrieves the list of associated namespaces . */
  QSet<Declaration*> associatedNamespaces() const;

private:

  void addAssociatedClass( Declaration * declaration );
  void addAssociatedFunction( Declaration * declaration );

  /**
   * @brief Adds an associated namespace by identifier.
   * All namespace declarations matching the given identifier are added.
   */
  void addAssociatedNamespace( const QualifiedIdentifier & identifier );

  /** @brief Adds an associated namespace declaration. */
  void addAssociatedNamespace( Declaration * declaration );

  /** @brief Namespaces associated with the name lookup. */
  QSet<Declaration*> m_associatedNamespaces;

  DUContextPointer m_context;
  TopDUContextPointer m_topContext;
};

}

#endif // CPP_ADLHELPER_H
