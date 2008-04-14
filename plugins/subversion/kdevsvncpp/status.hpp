/*
 * ====================================================================
 * Copyright (c) 2002-2008 The RapidSvn Group.  All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library (in the file LGPL.txt); if not, 
 * write to the Free Software Foundation, Inc., 51 Franklin St, 
 * Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */
#ifndef _SVNCPP_STATUS_HPP_
#define _SVNCPP_STATUS_HPP_

// subversion api
#include "svn_wc.h"

// svncpp
#include "svncpp/entry.hpp"
#include "svncpp/pool.hpp"

namespace svn
{
  /**
   * Subversion status API.
   *
   * @see svn_wc.hpp
   */
  class Status
  {
  public:
    /**
     * default constructor
     *
     * @param path
     * @param status
     */
    Status (const char * path = 0, const svn_wc_status2_t * status = 0);

    /**
     * copy constructor
     */
    Status (const Status & src);

    /**
     * destructor
     */
    ~Status ();

    /**
     * @return path of status entry
     */
    const char *
    path () const;

    /**
     * @return entry for this path
     * @retval entry.isValid () = false item is not versioned
     */
    const Entry 
    entry () const;

    /**
     * @return file status property enum of the "textual" component. 
     */
    const svn_wc_status_kind 
    textStatus () const;

    /**
     * @return file status property enum of the "property" component. 
     */
    const svn_wc_status_kind 
    propStatus () const;

    /**
     * @retval TRUE if under version control
     */
    const bool 
    isVersioned () const;

    /**
     * @retval TRUE if copied
     */
    const bool 
    isCopied () const;

    /**
     * @retval TRUE if switched
     */
    const bool
    isSwitched () const;

    /**
     * @return the entry's text status in the repository
     */
    const svn_wc_status_kind
    reposTextStatus () const;

    /**
     * @return the entry's prop status in the repository
     */
    const svn_wc_status_kind
    reposPropStatus () const;

    /**
     * @return true if locked
     */
    const bool
    isLocked () const;

    /**
     * @return true if has repository lock (not local)
     */
    const bool
    isRepLock () const;

    /**
     * @return lock token or null if not locked
     */
    const char *
    lockToken () const;

    /**
     * @return lock owner or null if not locked
     */
    const char *
    lockOwner () const;

    /**
     * @return comment lock, null or no comment
     */
    const char *
    lockComment () const;

    /**
     * @return lock creation date or 0 if not locked
     */
    const apr_time_t
    lockCreationDate () const;

    /**
     * assignment operator
     */
    Status &
    operator = (const Status &);

    /** 
     * check whether the instance contains real data
     * or nothing if it has constructed with the default
     * constructor
     */
    bool
    isset () const;

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
