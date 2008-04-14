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
#include "svn_path.h"
#include "svn_sorts.h"
//#include "svn_utf.h"

// svncpp
#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/dirent.hpp"
#include "kdevsvncpp/exception.hpp"


static int
compare_items_as_paths (const svn_sort__item_t *a, const svn_sort__item_t *b)
{
  return svn_path_compare_paths ((const char *)a->key, (const char *)b->key);
}

namespace svn
{
  DirEntries
  Client::list (const char * pathOrUrl,
                svn_opt_revision_t * revision,
                bool recurse) throw (ClientException)
  {
    Pool pool;

    apr_hash_t * hash;
    svn_error_t * error = 
      svn_client_ls (&hash, 
                     pathOrUrl, 
                     revision, 
                     recurse, 
                     *m_context, 
                     pool);

    if (error != 0)
      throw ClientException (error);

    apr_array_header_t * 
      array = svn_sort__hash (
        hash, compare_items_as_paths, pool);

    DirEntries entries;

    for (int i = 0; i < array->nelts; ++i)
    {
      const char *entryname;
      svn_dirent_t *dirent;
      svn_sort__item_t *item;
     
      item = &APR_ARRAY_IDX (array, i, svn_sort__item_t);

      entryname = static_cast<const char *>(item->key);

      dirent = static_cast<svn_dirent_t *> 
        (apr_hash_get (hash, entryname, item->klen));

      entries.push_back (DirEntry (entryname, dirent));
    }

    return entries;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
