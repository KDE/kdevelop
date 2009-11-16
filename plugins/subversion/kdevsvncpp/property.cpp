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

// subversion api
#include "svn_client.h"
//#include "svn_utf.h"

// svncpp
#include "kdevsvncpp/exception.hpp"
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/pool.hpp"
#include "kdevsvncpp/property.hpp"
#include "kdevsvncpp/revision.hpp"
#include "m_check.hpp"

namespace svn
{
  PropertyEntry::PropertyEntry (const char * name, const char * value)
  {
    this->name = name;
    this->value = value;
  }

  Property::Property (Context * context, const Path & path)
    : m_context (context), m_path (path)
  {
    list ();
  }

  Property::~Property ()
  {
  }

  void
  Property::list ()
  {
    Pool pool;
    Revision revision;

    m_entries.clear ();
    apr_array_header_t * props;
    svn_error_t * error =
      svn_client_proplist (&props,
                           m_path.c_str (), 
                           revision,
                           false, /* recurse */
                           *m_context,
                           pool);
    if(error != NULL)
    {
      throw ClientException (error);
    }

    for (int j = 0; j < props->nelts; ++j)
    {
      svn_client_proplist_item_t *item = 
        ((svn_client_proplist_item_t **)props->elts)[j];

      apr_hash_index_t *hi;

      for (hi = apr_hash_first (pool, item->prop_hash); hi; 
           hi = apr_hash_next (hi))
      {
        const void *key;
        void *val;

        apr_hash_this (hi, &key, NULL, &val);

        m_entries.push_back (PropertyEntry (
          (const char *)key, getValue ((const char *)key).c_str ()));
      } 
    }
  }

  std::string 
  Property::getValue (const char * name)
  {
    Pool pool;
    Revision revision;

    apr_hash_t *props;
    svn_client_propget (&props, 
                        name, 
                        m_path.c_str (),
                        revision,
                        false, // recurse
                        *m_context,
                        pool);

    apr_hash_index_t *hi;
    hi = apr_hash_first (pool, props); 
    if( !hi )
    {
      return "";
    }

    const void *key;
    void *val;
    const svn_string_t *propval;
    apr_hash_this (hi, &key, NULL, &val);
    propval = (const svn_string_t *)val;

    return propval->data;
  }

  void
  Property::set (const char * name, const char * value)
  {
    Pool pool;

    const svn_string_t * propval =
      svn_string_create ((const char *) value, pool);

    bool recurse = false;
    bool skip_checks = false;

    svn_error_t * error = 
      svn_client_propset2 (name,
                           propval,
                           m_path.c_str (),
                           recurse,
                           skip_checks,
                           *m_context,
                           pool);

    if(error != NULL)
      throw ClientException (error);
  }

  void 
  Property::remove (const char * name)
  {
    Pool pool;

    svn_error_t * error = 
      svn_client_propset (name, 
                          NULL, // value = NULL
                          m_path.c_str (),
                          false, //dont recurse
                          pool);
    if(error != NULL)
      throw ClientException (error);
  }

}
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
