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

#ifndef _SVNCPP_STATUS_SELECTION_HPP_
#define _SVNCPP_STATUS_SELECTION_HPP_

// svncpp
#include "svncpp/status.hpp"
#include "svncpp/path.hpp"


namespace svn
{
  // forward declarations
  class Pool;
  class Targets;

  /**
   * Container for a vector full of @ref Status
   */
  class StatusSel
  {
  public:
    /**
     * default constructor
     */
    StatusSel();

    /**
     * Destructor
     */
    virtual ~ StatusSel();

    /**
     * Copy Constructor
     *
     * @param src Source
     */
    StatusSel(const StatusSel & src);

    /**
     * Assignment operator
     */
    StatusSel &
    operator = (const StatusSel & src);

    /**
     * Returns an apr array containing
     * char *.
     *
     * @param pool Pool used for conversion
     */
    const apr_array_header_t *
    array(const Pool & pool) const;

    /**
     * Returns a vector of statuses
     *
     * @return vector of statuses
     */
    const StatusVector &
    statusVector() const;

    /**
     * Returns a vector of paths
     *
     * @return vector of paths
     */
    const Targets &
    targets() const;

    /**
     * returns the first target in the list
     * or an empty Path if no entries
     * are present
     * @return the first @ref Path in the list
     */
    const Path &
    target() const;

    /**
     * @return the number of targets
     */
    size_t size() const;

    /**
     * reserves @a size
     */
    void
    reserve(size_t size);

    /**
     * add and check the next entry
     *
     * @param status @ref Status to add
     */
    void
    push_back(const Status & status);

    /**
     * cleans out all entries
     */
    void
    clear();

    /**
     * operator to return the vector
     *
     * @return vector with targets
     */
    operator const PathVector & () const;

    /** at least one target is a file */
    bool
    hasFiles() const;

    /** at least one target is a directory */
    bool
    hasDirs() const;

    /** at least one target is versioned */
    bool
    hasVersioned() const;

    /** at least one target is unversioned */
    bool
    hasUnversioned() const;

    /** at least one target is a repository URL */
    bool
    hasUrl() const;

    /** at least one target is a local file or dir */
    bool
    hasLocal() const;

  private:
    struct Data;
    Data * m;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
