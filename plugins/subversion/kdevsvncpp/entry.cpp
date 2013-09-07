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

// svncpp
#include "kdevsvncpp/entry.hpp"
#include "m_check.hpp"

namespace svn
{
  Entry::Entry(const svn_wc_entry_t * src)
      : m_entry(0), m_pool(0), m_valid(false)
  {
    init(src);
  }

  Entry::Entry(const Entry & src)
      : m_entry(0), m_pool(0), m_valid(false)
  {
    init(src);
  }

  Entry::~Entry()
  {
    // no need to explicitly de-allocate m_entry
    // since this will be handled by m_pool
  }

  void
  Entry::init(const svn_wc_entry_t * src)
  {
    if (src)
    {
      // copy the contents of src
      m_entry = svn_wc_entry_dup(src, m_pool);
      m_valid = true;
    }
    else
    {
      // create an empty entry
      m_entry = (svn_wc_entry_t*)
                apr_pcalloc(m_pool, sizeof(svn_wc_entry_t));
    }
  }

  Entry &
  Entry::operator = (const Entry & src)
  {
    if (this == &src)
      return *this;

    init(src);
    return *this;
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
