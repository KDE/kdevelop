/*
 * ====================================================================
 * Copyright (c) 2002-2009 The RapidSvn Group.  All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the file GPL.txt.  
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */

#ifndef _SVNCPP_TARGETS_HPP_
#define _SVNCPP_TARGETS_HPP_

// apr api
#include "apr_tables.h"

// svncpp
#include "svncpp/path.hpp"

namespace svn
{
  // forward declarations
  class Pool;


  /**
   * Encapsulation for Subversion target arrays handling
   */
  class Targets
  {
  public:
    /**
     * Constructor
     *
     * @param targets vector of paths
     */
    Targets(const PathVector & targets);

    /**
     * Constructor from an APR array containing
     * char *.
     *
     * @param targets APR array header
     */
    Targets(const apr_array_header_t * targets);

    /**
     * Constructor. Initializes list with just
     * one entry
     *
     * @param target
     */
    Targets(const char * target = 0);

    /**
     * Copy Constructor
     *
     * @param targets Source
     */
    Targets(const Targets & targets);

    /**
     * Destructor
     */
    virtual ~Targets();

    /**
     * Returns an apr array containing
     * char *.
     *
     * @param pool Pool used for conversion
     */
    const apr_array_header_t *
    array(const Pool & pool) const;

    /**
     * Returns a vector of paths
     *
     * @return vector of paths
     */
    const PathVector &
    targets() const;

    /**
     * @return the number of targets
     */
    size_t size() const;

    /**
     * operator to return the vector
     *
     * @return vector with targets
     */
    operator const PathVector & () const
    {
      return m_targets;
    }

    /**
     * returns one single target. in fact,
     * the first in the vector, it there are more
     * than one. if there is no target returns
     * an empty path
     *
     * @return single path
     */
    const Path
    target() const;

    /**
     * adds a @ref Path to the existing entries
     *
     * @remark you have to make sure the path is
     *         unique
     */
    void
    push_back(const Path & path);


    /**
     * clears all entries
     */
    void
    clear();

    /**
     * reserve the size for following calls
     * to @ref push_back
     */
    void
    reserve(size_t size);

  private:
    PathVector m_targets;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
