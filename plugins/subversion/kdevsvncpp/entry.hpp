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
#ifndef _SVNCPP_ENTRY_HPP_
#define _SVNCPP_ENTRY_HPP_

// subversion api
#include "svn_wc.h"

// svncpp
#include "kdevsvncpp/pool.hpp"


namespace svn
{
  /**
   * C++ API for Subversion. 
   * This class wraps around @a svn_wc_entry_t.
   */
  class Entry
  {
  public:
    /**
     * default constructor. if @a src is set,
     * copy its contents. 
     *
     * If @a src is not set (=0) this will be
     * a non-versioned entry. This can be checked
     * later with @a isValid ().
     *
     * @param src another entry to copy from
     */
    Entry (const svn_wc_entry_t * src = 0);

    /**
     * copy constructor
     */
    Entry (const Entry & src);

    /**
     * destructor 
     */
    virtual ~Entry ();

    /**
     * returns whether this is a valid=versioned
     * entry.    
     *
     * @return is entry valid
     * @retval true valid entry
     * @retval false invalid or unversioned entry
     */
    bool isValid () const
    {
      return m_valid;
    }

    /**
     * @return entry's name
     */
    const char *
    name () const
    {
      return m_entry->name;
    }

    /**
     * @return base revision
     */
    const svn_revnum_t 
    revision () const
    {
      return m_entry->revision;
    }

    /**
     * @return url in repository
     */
    const char * 
    url () const
    {
      return m_entry->url;
    }

    /**
     * @return canonical repository url
     */
    const char *
    repos () const
    {
      return m_entry->repos;
    }

    /**
     * @return repository uuid
     */
    const char *
    uuid () const
    {
      return m_entry->uuid;
    }

    /**
     * @return node kind (file, dir, ...)
     */
    const svn_node_kind_t
    kind () const
    {
      return m_entry->kind;
    }

    /**
     * @return scheduling (add, delete, replace)
     */
    const svn_wc_schedule_t
    schedule () const
    {
      return m_entry->schedule;
    }

    /**
     * @return TRUE if copied
     */
    const bool 
    isCopied () const
    {
      return m_entry->copied != 0;
    }
    
    /**
     * @return true if deleted
     */
    const bool
    isDeleted () const
    {
      return m_entry->deleted != 0;
    }

    /**
     * @return true if deleted
     */
    const bool
    isAbsent () const
    {
      return m_entry->absent != 0;
    }

    /**
     * @return copyfrom location
     */
    const char *
    copyfromUrl () const
    {
      return m_entry->copyfrom_url;
    }

    /**
     * @return copyfrom revision
     */
    const svn_revnum_t
    copyfromRev () const
    {
      return m_entry->copyfrom_rev;
    }

    /**
     * @return old version of conflicted file
     */
    const char *
    conflictOld () const
    {
      return m_entry->conflict_old;
    }

    /**
     * @return new version of conflicted file
     */
    const char *
    conflictNew () const
    {
      return m_entry->conflict_new;
    }
    
    /**
     * @return working version of conflicted file
     */
    const char *
    conflictWrk () const
    {
      return m_entry->conflict_wrk;
    }

    /**
     * @return property reject file
     */
    const char *
    prejfile () const
    {
      return m_entry->prejfile;
    }

    /**
     * @return last up-to-date time for text contents
     * @retval 0 no information available
     */
    const apr_time_t
    textTime () const
    {
      return m_entry->text_time;
    }
    
    /**
     * @return last up-to-date time for properties
     * @retval 0 no information available
     */
    const apr_time_t
    propTime () const
    {
      return m_entry->prop_time;
    }

    /**
     * @return base64 encoded checksum
     * @retval NULL for backwards compatibility
     */
    const char *
    checksum () const
    {
      return m_entry->checksum;
    }

    /**
     * @return last revision this was changed
     */
    const svn_revnum_t
    cmtRev () const
    {
      return m_entry->cmt_rev;
    }

    /**
     * @return last date this was changed
     */
    const apr_time_t
    cmtDate () const
    {
      return m_entry->cmt_date;
    }

    /**
     * @return last commit author of this file
     */
    const char *
    cmtAuthor () const
    {
      return m_entry->cmt_author;
    }

    /**
     * @return svn_wc_entry_t for this class
     */
    operator svn_wc_entry_t * () const
    {
      return m_entry;
    }

    /**
     * assignment operator
     */
    Entry &
    operator = (const Entry &);

  private:
    svn_wc_entry_t * m_entry;
    Pool m_pool;
    bool m_valid;

    /**
     * initializes the members
     */
    void 
    init (const svn_wc_entry_t * src);
 };

}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
