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
#if defined( _MSC_VER) && _MSC_VER <= 1200
#pragma warning( disable: 4786 )// debug symbol truncated
#endif

// subversion api
#include "svn_client.h"
#include "svn_path.h"
#include "svn_sorts.h"
#include "svn_version.h"
//#include "svn_utf.h"

// svncpp
#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/dirent.hpp"
#include "kdevsvncpp/exception.hpp"


#if SVN_VER_MAJOR == 1 && SVN_VER_MINOR < 8

static int
compare_items_as_paths(const svn_sort__item_t *a, const svn_sort__item_t *b)
{
  return svn_path_compare_paths((const char *)a->key, (const char *)b->key);
}

namespace svn
{
  DirEntries
  Client::list(const char * pathOrUrl,
               svn_opt_revision_t * revision,
               bool recurse) throw(ClientException)
  {
    Pool pool;

    apr_hash_t * hash;
    svn_error_t * error =
      svn_client_ls(&hash,
                    pathOrUrl,
                    revision,
                    recurse,
                    *m_context,
                    pool);

    if (error != 0)
      throw ClientException(error);

    apr_array_header_t *
    array = svn_sort__hash(
              hash, compare_items_as_paths, pool);

    DirEntries entries;

    for (int i = 0; i < array->nelts; ++i)
    {
      const char *entryname;
      svn_dirent_t *dirent;
      svn_sort__item_t *item;

      item = &APR_ARRAY_IDX(array, i, svn_sort__item_t);

      entryname = static_cast<const char *>(item->key);

      dirent = static_cast<svn_dirent_t *>
               (apr_hash_get(hash, entryname, item->klen));

      entries.push_back(DirEntry(entryname, dirent));
    }

    return entries;
  }
}

#else

#include <algorithm>

static svn_error_t* store_entry(
        void *baton,
        const char *path,
        const svn_dirent_t *dirent,
        const svn_lock_t *,
        const char *abs_path,
        const char *,
        const char *,
        apr_pool_t *scratch_pool)
{
  svn::DirEntries *entries = reinterpret_cast<svn::DirEntries*>(baton);
  if (path[0] == '\0') {
    if (dirent->kind == svn_node_file) {
      // for compatibility with svn_client_ls behaviour, listing a file
      // stores that file name
      entries->push_back(svn::DirEntry(svn_path_basename(abs_path, scratch_pool), dirent));
    }
  } else {
    entries->push_back(svn::DirEntry(path, dirent));
  }
  return SVN_NO_ERROR;
}

static bool sort_by_path(svn::DirEntry const& a, svn::DirEntry const& b)
{
  return svn_path_compare_paths(a.name(), b.name()) < 0;
}

namespace svn
{
  DirEntries
  Client::list(const char * pathOrUrl,
               svn_opt_revision_t * revision,
               bool recurse) throw(ClientException)
  {
    Pool pool;
    DirEntries entries;

    svn_error_t * error =
      svn_client_list3(pathOrUrl,
                       revision,
                       revision,
                       SVN_DEPTH_INFINITY_OR_IMMEDIATES(recurse),
                       SVN_DIRENT_ALL,
                       FALSE, // fetch locks
                       FALSE, // include externals
                       &store_entry,
                       &entries,
                       *m_context,
                       pool);

    if (error != SVN_NO_ERROR)
      throw ClientException(error);

    std::sort(entries.begin(), entries.end(), &sort_by_path);

    return entries;
  }
}

#endif

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
