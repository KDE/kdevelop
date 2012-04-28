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

// svncpp
#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/exception.hpp"
#include "kdevsvncpp/pool.hpp"
#include "kdevsvncpp/targets.hpp"
#include "m_check.hpp"

namespace svn
{
  svn_revnum_t
  Client::checkout(const char * url,
                   const Path & destPath,
                   const Revision & revision,
                   bool recurse,
                   bool ignore_externals,
                   const Revision & peg_revision) throw(ClientException)
  {
    Pool subPool;
    apr_pool_t * apr_pool = subPool.pool();
    svn_revnum_t revnum = 0;

    svn_error_t * error =
      svn_client_checkout2(&revnum,
                           url,
                           destPath.c_str(),
                           peg_revision.revision(),  // peg_revision
                           revision.revision(),  // revision
                           recurse,
                           ignore_externals,
                           *m_context,
                           apr_pool);

    if (error != NULL)
      throw ClientException(error);

    return revnum;
  }

  void
  Client::remove(const Path & path,
                 bool force) throw(ClientException)
  {
    Pool pool;
    Targets targets(path.c_str());
    svn_client_commit_info_t *commit_info = NULL;

    svn_error_t * error =
      svn_client_delete(&commit_info,
                        const_cast<apr_array_header_t*>(targets.array(pool)),
                        force,
                        *m_context,
                        pool);
    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::remove(const Targets & targets,
                 bool force) throw(ClientException)
  {
    Pool pool;
    svn_client_commit_info_t *commit_info = NULL;

    svn_error_t * error =
      svn_client_delete(&commit_info,
                        const_cast<apr_array_header_t*>(targets.array(pool)),
                        force,
                        *m_context,
                        pool);
    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::lock(const Targets & targets, bool force,
               const char * comment) throw(ClientException)
  {
    Pool pool;

    svn_error_t * error =
      svn_client_lock(const_cast<apr_array_header_t*>(targets.array(pool)),
                      comment,
                      force,
                      *m_context,
                      pool);
    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::unlock(const Targets & targets, bool force) throw(ClientException)
  {
    Pool pool;

    svn_error_t * error =
      svn_client_unlock(const_cast<apr_array_header_t*>(targets.array(pool)),
                        force,
                        *m_context,
                        pool);
    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::revert(const Targets & targets,
                 bool recurse) throw(ClientException)
  {
    Pool pool;

    svn_error_t * error =
      svn_client_revert((targets.array(pool)),
                        recurse,
                        *m_context,
                        pool);

    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::add(const Path & path,
              bool recurse) throw(ClientException)
  {
    Pool pool;

    // we do not need the newer version of this
    // function "svn_client_add2" or "svn_client_add3"
    // since RapidSVN doesnt even have a dialog
    // for adding false
    svn_error_t * error =
      svn_client_add(path.c_str(),
                     recurse,
                     *m_context,
                     pool);

    if (error != NULL)
      throw ClientException(error);
  }

  std::vector<svn_revnum_t>
  Client::update(const Targets & targets,
                 const Revision & revision,
                 bool recurse,
                 bool ignore_externals) throw(ClientException)
  {
    Pool pool;
    apr_array_header_t * result_revs;

    svn_error_t * error =
      svn_client_update2(&result_revs,
                         const_cast<apr_array_header_t*>(targets.array(pool)),
                         revision.revision(),
                         recurse,
                         ignore_externals,
                         *m_context,
                         pool);
    if (error != NULL)
      throw ClientException(error);

    std::vector<svn_revnum_t> revnums;
    int i;
    for (i = 0; i < result_revs->nelts; i++)
    {
      svn_revnum_t revnum=
        APR_ARRAY_IDX(result_revs, i, svn_revnum_t);

      revnums.push_back(revnum);
    }

    return revnums;
  }

  svn_revnum_t
  Client::update(const Path & path,
                 const Revision & revision,
                 bool recurse,
                 bool ignore_externals) throw(ClientException)
  {
    Targets targets(path.c_str());
    return update(targets, revision, recurse, ignore_externals)[0];
  }

  svn_revnum_t
  Client::commit(const Targets & targets,
                 const char * message,
                 bool recurse,
                 bool keep_locks) throw(ClientException)
  {
    Pool pool;

    m_context->setLogMessage(message);

    svn_client_commit_info_t *commit_info = NULL;

    svn_error_t * error =
      svn_client_commit2(&commit_info,
                         targets.array(pool),
                         recurse,
                         keep_locks,
                         *m_context,
                         pool);
    if (error != NULL)
      throw ClientException(error);

    if (commit_info && SVN_IS_VALID_REVNUM(commit_info->revision))
      return commit_info->revision;

    return -1;
  }


  void
  Client::copy(const Path & srcPath,
               const Revision & srcRevision,
               const Path & destPath) throw(ClientException)
  {
    Pool pool;
    svn_client_commit_info_t *commit_info = NULL;
    svn_error_t * error =
      svn_client_copy(&commit_info,
                      srcPath.c_str(),
                      srcRevision.revision(),
                      destPath.c_str(),
                      *m_context,
                      pool);

    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::move(const Path & srcPath,
               const Revision & /*srcRevision*/,
               const Path & destPath,
               bool force) throw(ClientException)
  {
    Pool pool;
    svn_client_commit_info_t *commit_info = NULL;

    svn_error_t * error =
      svn_client_move2(&commit_info,
                       srcPath.c_str(),
                       destPath.c_str(),
                       force,
                       *m_context,
                       pool);

    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::mkdir(const Path & path) throw(ClientException)
  {
    Pool pool;
    Targets targets(path.c_str());

    svn_client_commit_info_t *commit_info = NULL;
    svn_error_t * error =
      svn_client_mkdir(&commit_info,
                       const_cast<apr_array_header_t*>
                       (targets.array(pool)),
                       *m_context, pool);

    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::mkdir(const Targets & targets) throw(ClientException)
  {
    Pool pool;

    svn_client_commit_info_t *commit_info = NULL;
    svn_error_t * error =
      svn_client_mkdir(&commit_info,
                       const_cast<apr_array_header_t*>
                       (targets.array(pool)),
                       *m_context, pool);

    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::cleanup(const Path & path) throw(ClientException)
  {
    Pool subPool;
    apr_pool_t * apr_pool = subPool.pool();

    svn_error_t * error =
      svn_client_cleanup(path.c_str(), *m_context, apr_pool);

    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::resolved(const Path & path,
                   bool recurse) throw(ClientException)
  {
    Pool pool;
    svn_error_t * error =
      svn_client_resolved(path.c_str(),
                          recurse,
                          *m_context,
                          pool);

    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::doExport(const Path & from_path,
                   const Path & to_path,
                   const Revision & revision,
                   bool overwrite,
                   const Revision & peg_revision,
                   bool ignore_externals,
                   bool recurse,
                   const char * native_eol) throw(ClientException)
  {
    Pool pool;
    svn_revnum_t revnum = 0;

    svn_error_t * error =
      svn_client_export3(&revnum,
                         from_path.c_str(),
                         to_path.c_str(),
                         peg_revision.revision(),
                         revision.revision(),
                         overwrite,
                         ignore_externals,
                         recurse,
                         native_eol,
                         *m_context,
                         pool);

    if (error != NULL)
      throw ClientException(error);
  }

  svn_revnum_t
  Client::doSwitch(const Path & path,
                   const char * url,
                   const Revision & revision,
                   bool recurse) throw(ClientException)
  {
    Pool pool;
    svn_revnum_t revnum = 0;
    svn_error_t * error =
      svn_client_switch(&revnum,
                        path.c_str(),
                        url,
                        revision.revision(),
                        recurse,
                        *m_context,
                        pool);

    if (error != NULL)
      throw ClientException(error);
    return revnum;
  }

  void
  Client::import(const Path & path,
                 const char * url,
                 const char * message,
                 bool recurse) throw(ClientException)
  {
    Pool pool;
    svn_client_commit_info_t *commit_info = NULL;

    m_context->setLogMessage(message);

    svn_error_t * error =
      svn_client_import(&commit_info,
                        path.c_str(),
                        url,
                        !recurse,
                        *m_context,
                        pool);

    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::import(const Path & path,
                 const Path & url,
                 const char * message,
                 bool recurse) throw(ClientException)
  {
    import(path, url.c_str(), message, recurse);
  }

  void
  Client::merge(const Path & path1, const Revision & revision1,
                const Path & path2, const Revision & revision2,
                const Path & localPath, bool force,
                bool recurse,
                bool notice_ancestry,
                bool dry_run) throw(ClientException)
  {
    Pool pool;
    svn_error_t * error =
      svn_client_merge(path1.c_str(),
                       revision1.revision(),
                       path2.c_str(),
                       revision2.revision(),
                       localPath.c_str(),
                       recurse,
                       !notice_ancestry,
                       force,
                       dry_run,
                       *m_context,
                       pool);

    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::relocate(const Path & path,
                   const char * from_url,
                   const char * to_url,
                   bool recurse) throw(ClientException)
  {
    Pool pool;
    svn_error_t * error =
      svn_client_relocate(path.c_str(),
                          from_url,
                          to_url,
                          recurse,
                          *m_context,
                          pool);

    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::ignore(const Path & path) throw(ClientException)
  {
    static const char s_svnIgnore[] = "svn:ignore";
    Pool pool;

    std::string dirpath, basename;
    path.split(dirpath, basename);

    Revision revision;
    apr_hash_t *props;
    svn_error_t * error =
      svn_client_propget(&props,
                         s_svnIgnore,
                         dirpath.c_str(),
                         Revision::UNSPECIFIED.revision(),
                         false, // recursive
                         *m_context,
                         pool);
    if (error != NULL)
      throw ClientException(error);

    PathPropertiesMapList path_prop_map_list;

    apr_hash_index_t *hi;
    for (hi = apr_hash_first(pool, props); hi;
         hi = apr_hash_next(hi))
    {
      PropertiesMap prop_map;

      const void *key;
      void *val;

      apr_hash_this(hi, &key, NULL, &val);

      prop_map [std::string(s_svnIgnore)] = std::string(((const svn_string_t *)val)->data);

      path_prop_map_list.push_back(PathPropertiesMapEntry((const char *)key, prop_map));
    }

    std::string str = basename;
    for (PathPropertiesMapList::const_iterator i=path_prop_map_list.begin(), ei=path_prop_map_list.end();i!=ei;++i)
    {
      if (dirpath != i->first)
        continue;
      for (PropertiesMap::const_iterator j=i->second.begin(), ej=i->second.end(); j != ej; ++j)
      {
        if (s_svnIgnore != j->first)
          continue;
        str += '\n'+j->second;
      }
    }
    const svn_string_t * propval =
      svn_string_create(str.c_str(), pool);
    error =
      svn_client_propset2(s_svnIgnore,
                          propval,
                          dirpath.c_str(),
                          false,
                          false,
                          *m_context,
                          pool);
    if (error != NULL)
      throw ClientException(error);
  }

  void
  Client::ignore(const Targets & targets) throw(ClientException)
  {
    // it's slow, but simple
    for (std::vector<Path>::const_iterator i=targets.targets().begin(), e=targets.targets().end();i!=e;++i)
    {
      ignore(*i);
    }
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
