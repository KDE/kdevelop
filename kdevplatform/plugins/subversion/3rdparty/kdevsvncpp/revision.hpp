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

#ifndef _SVNCPP_REVISION_HPP_
#define _SVNCPP_REVISION_HPP_

// subversion api
#include "svn_types.h"
#include "svn_opt.h"

// svncpp
#include "datetime.hpp"

namespace svn
{
  /**
   * Class that encapsulates svn_opt_revnum_t.
   *
   * @see svn_opt_revnum_t
   */
  class Revision
  {
  private:
    svn_opt_revision_t m_revision;

    void
    init(const svn_opt_revision_t * revision);

  public:
    static const Revision START;
    static const Revision BASE;
    static const Revision HEAD;
    static const Revision WORKING;
    static const Revision UNSPECIFIED;

    /**
     * Constructor
     *
     * @param revision revision information
     */
    Revision(const svn_opt_revision_t * revision);

    /**
     * Constructor
     *
     * @param revnum revision number
     */
    Revision(const svn_revnum_t revnum);

    /**
     * Constructor
     *
     * @param kind
     */
    Revision(const svn_opt_revision_kind kind = svn_opt_revision_unspecified);

    /**
     * Constructor
     *
     * @param dateTime DateTime wrapper for apr_time_t
     */
    Revision(const DateTime dateTime);

    /**
     * Copy constructor
     *
     * @param revision Source
     */
    Revision(const Revision & revision);

    /**
     * @return revision information
     */
    const svn_opt_revision_t *
    revision() const;

    /**
     * @see revision (). Same function
     * but with operator overloading
     */
    operator svn_opt_revision_t * ()
    {
      return &m_revision;
    }

    /**
     * @return revision numver
     */
    svn_revnum_t
    revnum() const;

    /**
     * @return revision kind
     */
    svn_opt_revision_kind
    kind() const;

    /**
     * @see kind (). Same function
     * but with operator overloading
     */
    operator svn_opt_revision_kind() const
    {
      return kind();
    }

    /**
     * @return date
     */
    apr_time_t
    date() const;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
