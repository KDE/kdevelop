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

// svncpp
#include "kdevsvncpp/pool.hpp"


/**
 * SvnCpp namespace
 */
namespace svn
{
  static bool m_initialized = false;

  inline static apr_pool_t * 
  pool_create (apr_pool_t * parent)
  {
    // CAUTION: this is not thread-safe!!!
    if (!m_initialized)
    {
      m_initialized = true;
      apr_pool_initialize ();
    }

    return svn_pool_create (parent);
  }

  Pool::Pool (apr_pool_t * parent)
    : m_parent (parent), m_pool (pool_create (parent))
  {
  }

  Pool::~Pool ()
  {
    if(m_pool)
    {
      svn_pool_destroy (m_pool);
    }
  }

  apr_pool_t *
  Pool::pool () const
  {
    return m_pool;
  }

  void
  Pool::renew ()
  {
    if (m_pool)
    {
      svn_pool_destroy (m_pool);
    }
    m_pool = pool_create (m_parent);
  }

//TODO
//   apr_pool_t *
//   Pool::operator=(const Pool & pool)
//   {
//     return 
//     if (this == &path)
//       return *this;
//     m_path = path.c_str();
//     return *this;
//   }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
