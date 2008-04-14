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
#if defined( _MSC_VER) && _MSC_VER <= 1200
#pragma warning( disable: 4786 )// debug symbol truncated
#endif

// subversion api
#include "svn_client.h"

// svncpp
#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/exception.hpp"
#include "kdevsvncpp/pool.hpp"
#include "kdevsvncpp/revision.hpp"
#include "m_check.hpp"

namespace svn
{
  /**
   * lists properties in @a path no matter whether local or
   * repository
   *
   * @param path
   * @param revision
   * @param recurse
   * @return PropertiesList
   */
  PathPropertiesMapList
  Client::proplist (const Path & path,
                    const Revision & revision,
                    bool recurse)
  {
    Pool pool;
    apr_array_header_t * props;

    svn_error_t * error =
      svn_client_proplist (&props,
                           path.c_str (), 
                           revision.revision (),
                           recurse,
                           *m_context,
                           pool);
    if (error != NULL)
    {
      throw ClientException (error);
    }

    PathPropertiesMapList path_prop_map_list;
    for (int j = 0; j < props->nelts; ++j)
    {
      svn_client_proplist_item_t *item = 
        ((svn_client_proplist_item_t **)props->elts)[j];

      PropertiesMap prop_map;

      apr_hash_index_t *hi;
      for (hi = apr_hash_first (pool, item->prop_hash); hi; 
           hi = apr_hash_next (hi))
      {
        const void *key;
        void *val;

        apr_hash_this (hi, &key, NULL, &val);

        prop_map [std::string ((const char *)key)] = 
          std::string (((const svn_string_t *)val)->data);
      }

      path_prop_map_list.push_back (PathPropertiesMapEntry (item->node_name->data, prop_map));
    }

    return path_prop_map_list;
  }

  /**
   * lists properties in @a path no matter whether local or
   * repository
   *
   * @param path
   * @param revision
   * @param recurse
   * @return PropertiesList
   */
  
  PathPropertiesMapList
  Client::propget (const char *propName,
                   const Path &path,
                   const Revision &revision,
                   bool recurse)
  {
    Pool pool;

    apr_hash_t *props;
    svn_error_t * error =
      svn_client_propget (&props,
                          propName,
                          path.c_str (), 
                          revision.revision (),
                          recurse,
                          *m_context,
                          pool);
    if (error != NULL)
    {
      throw ClientException (error);
    }

    PathPropertiesMapList path_prop_map_list;


    apr_hash_index_t *hi;
    for (hi = apr_hash_first (pool, props); hi; 
         hi = apr_hash_next (hi))
    {
      PropertiesMap prop_map;

      const void *key;
      void *val;

      apr_hash_this (hi, &key, NULL, &val);

      prop_map [std::string (propName)] = std::string (((const svn_string_t *)val)->data);

      path_prop_map_list.push_back (PathPropertiesMapEntry ((const char *)key, prop_map));
    }

    return path_prop_map_list;
  }

  /**
   * set property in @a path no matter whether local or
   * repository
   *
   * @param path
   * @param revision
   * @param propName
   * @param propValue
   * @param recurse
   * @return PropertiesList
   */
  void
  Client::propset (const char * propName,
                   const char * propValue,
                   const Path & path,
                   const Revision & /*revision*/,
                   bool recurse,
                   bool skip_checks)
  {
    Pool pool;

    const svn_string_t * propval =
      svn_string_create ((const char *) propValue, pool);

    svn_error_t * error = 
      svn_client_propset2 (propName,
                           propval,
                           path.c_str (),
                           recurse,
                           skip_checks,
                           *m_context,
                           pool);
    if (error != NULL)
      throw ClientException (error);
  }

  /**
   * delete property in @a path no matter whether local or
   * repository
   *
   * @param path
   * @param revision
   * @param propName
   * @param propValue
   * @param recurse
   * @return PropertiesList
   */
  void
  Client::propdel (const char *propName,
                   const Path &path,
                   const Revision & /*revision*/,
                   bool recurse)
  {
    Pool pool;

    svn_error_t * error = 
      svn_client_propset (propName, 
                          NULL, // value = NULL
                          path.c_str (),
                          recurse,
                          pool);
    if (error != NULL)
      throw ClientException (error);
  }
  
//--------------------------------------------------------------------------------
//
//    revprop functions
//
//--------------------------------------------------------------------------------
  /**
   * lists revision properties in @a path no matter whether local or
   * repository
   *
   * @param path
   * @param revision
   * @param recurse
   * @return PropertiesList
   */
  std::pair<svn_revnum_t,PropertiesMap>
  Client::revproplist (const Path &path,
                       const Revision &revision)
  {
    Pool pool;

    apr_hash_t * props;
    svn_revnum_t revnum;
    svn_error_t * error =
      svn_client_revprop_list (&props,
                               path.c_str (), 
                               revision.revision (),
                               &revnum,
                               *m_context,
                               pool);
    if(error != NULL)
    {
      throw ClientException (error);
    }

    PropertiesMap prop_map;

    apr_hash_index_t *hi;
    for (hi = apr_hash_first (pool, props); hi; 
         hi = apr_hash_next (hi))
    {
      const void *key;
      void *val;

      apr_hash_this (hi, &key, NULL, &val);

      prop_map [std::string ((const char *)key)] = 
        std::string (((const svn_string_t *)val)->data);
    }

    return std::pair<svn_revnum_t,PropertiesMap> (revnum, prop_map);
  }

  /**
   * lists one revision property in @a path no matter whether local or
   * repository
   *
   * @param path
   * @param revision
   * @param recurse
   * @return PropertiesList
   */
  
  std::pair<svn_revnum_t,std::string>
  Client::revpropget (const char *propName,
                      const Path &path,
                      const Revision &revision)
  {
    Pool pool;

    svn_string_t *propval;
    svn_revnum_t revnum;
    svn_error_t * error =
      svn_client_revprop_get (propName,
                              &propval,
                              path.c_str (), 
                              revision.revision (),
                              &revnum,
                              *m_context,
                              pool);
    if (error != NULL)
    {
      throw ClientException (error);
    }

    // if the property does not exist NULL is returned
    if (propval == NULL)
      return std::pair<svn_revnum_t,std::string> (0, std::string());

    return std::pair<svn_revnum_t,std::string> (revnum, std::string (propval->data));
  }

  /**
   * set property in @a path no matter whether local or
   * repository
   *
   * @param path
   * @param revision
   * @param propName
   * @param propValue
   * @param recurse
   * @param revprop
   * @return PropertiesList
   */
  svn_revnum_t
  Client::revpropset (const char *propName,
                      const char *propValue,
                      const Path &path,
                      const Revision &revision,
                      bool force)
  {
    Pool pool;

    const svn_string_t * propval 
      = svn_string_create ((const char *) propValue, pool);

    svn_revnum_t revnum;
    svn_error_t * error = 
      svn_client_revprop_set (propName,
                              propval,
                              path.c_str (),
                              revision.revision (),
                              &revnum,
                              force,
                              *m_context,
                              pool);
    if(error != NULL)
      throw ClientException (error);

    return revnum;
  }

  /**
   * delete property in @a path no matter whether local or
   * repository
   *
   * @param path
   * @param revision
   * @param propName
   * @param propValue
   * @param recurse
   * @param revprop
   * @return PropertiesList
   */
  svn_revnum_t
  Client::revpropdel (const char *propName,
                      const Path &path,
                      const Revision &revision,
                      bool force)
  {
    Pool pool;

    svn_revnum_t revnum;
    svn_error_t * error = 
      svn_client_revprop_set (propName, 
                              NULL, // value = NULL
                              path.c_str (),
                              revision.revision (),
                              &revnum,
                              force,
                              *m_context,
                              pool);
    if (error != NULL)
      throw ClientException (error);

    return revnum;
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
