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

// subversion api
#include "svn_path.h"
#include "svn_dirent_uri.h"

// apr api
#include "apr_file_io.h"

// svncpp
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/pool.hpp"
#include "kdevsvncpp/url.hpp"

namespace svn
{
  const PathVector EmptyPathVector;

  Path::Path(const char * path)
  {
    init(path);
  }

  Path::Path(const std::string & path)
  {
    init(path.c_str());
  }

  Path::Path(const Path & path)
  {
    init(path.c_str());
  }

  void
  Path::init(const char * path)
  {
    Pool pool;

    m_pathIsUrl = false;

    if (path == 0)
      m_path = "";
    else
    {
      const char * int_path = svn_dirent_canonicalize(path, pool);

      m_path = int_path;

      if (svn::Url::isValid(int_path))
        m_pathIsUrl = true;
    }
  }

  const std::string &
  Path::path() const
  {
    return m_path;
  }

  const char *
  Path::c_str() const
  {
    return m_path.c_str();
  }

  Path&
  Path::operator= (const Path & path)
  {
    if (this == &path)
      return *this;

    init(path.c_str());

    return *this;
  }

  bool
  Path::operator== (const Path& path) const
  {
    if (path.path() == this->path())
      return true;

    return false;
  }

  bool
  Path::isSet() const
  {
    return m_path.length() > 0;
  }

  bool
  Path::isUrl() const
  {
    return m_pathIsUrl;
  }

  static bool
  isAbsolute(const char * path)
  {
    if (0 == path)
      return false;

    std::string p(path);

    if (0 == p.length())
      return false;

    // a path that begins with "/" is absolute
    if ('/' == p [0])
      return true;

    // a path with a ":" like "http://xxx" or
    // "c:/foo" is absolute too
    if (p.find(":", 0) != std::string::npos)
      return true;

    // Well it's relative
    return false;
  }

  void
  Path::addComponent(const char * component)
  {
    Pool pool;

    if (0 == component)
      return;

    // in case of an empty string, return
    if (*component == 0)
      return;

    // if the @a component is absolute, simply
    // use it
    if (isAbsolute(component))
    {
      m_path = component;
      return;
    }

    if (Url::isValid(m_path.c_str()))
    {
      const char * newPath =
        svn_path_url_add_component(m_path.c_str(),
                                   component,
                                   pool);
      m_path = newPath;
    }
    else
    {
      svn_stringbuf_t * pathStringbuf =
        svn_stringbuf_create(m_path.c_str(), pool);

      svn_path_add_component(pathStringbuf,
                             component);

      m_path = pathStringbuf->data;
    }
  }

  void
  Path::addComponent(const std::string & component)
  {
    addComponent(component.c_str());
  }

  void
  Path::split(std::string & dirpath, std::string & basename) const
  {
    Pool pool;

    const char * cdirpath;
    const char * cbasename;

    svn_path_split(m_path.c_str(), &cdirpath, &cbasename, pool);

    dirpath = cdirpath;
    basename = cbasename;
  }

  void
  Path::split(std::string & dir, std::string & filename, std::string & ext) const
  {
    std::string basename;

    // first split path into dir and filename+ext
    split(dir, basename);

    // next search for last .
    size_t pos = basename.find_last_of(".");
    if (pos == std::string::npos)
    {
      filename = basename;
      ext = "";
    }
    else
    {
      filename = basename.substr(0, pos);
      ext = basename.substr(pos);
    }
  }

  std::string
  Path::basename() const
  {
    std::string dir;
    std::string filename;

    split(dir, filename);

    return filename;
  }

  std::string
  Path::dirpath() const
  {
    std::string dir;
    std::string filename;

    split(dir, filename);

    return dir;
  }

  std::string
  Path::substr(const size_t count) const
  {
    if (m_path.length() > count)
      return m_path.substr(count);
    else
      return "";
  }

  std::string
  Path::unescape() const
  {
    return svn::Url::unescape(m_path.c_str());
  }


  /* ===================================================================
   * The next two Fixed_* functions are copies of the APR
   * apr_temp_dir_get functionality with a fix applied.
   * This should turn up in APR release 0.9.5 or 1.0, but
   * for now is reproduced here.
   *
   * TODO: Remove this section!
   */
#include "apr_env.h"

#define test_tempdir    Fixed_test_tempdir
#define apr_temp_dir_get    Fixed_apr_temp_dir_get

  static char global_temp_dir[APR_PATH_MAX+1] = { 0 };

  /* Try to open a temporary file in the temporary dir, write to it,
    and then close it. */
  static int Fixed_test_tempdir(const char *temp_dir, apr_pool_t *p)
  {
    apr_file_t *dummy_file;
    // This is the only actual fix - adding the ".XXXXXX"!
    const char *path = apr_pstrcat(p, temp_dir, "/apr-tmp.XXXXXX", NULL);

    if (apr_file_mktemp(&dummy_file, (char *)path, 0, p) == APR_SUCCESS) {
      if (apr_file_putc('!', dummy_file) == APR_SUCCESS) {
        if (apr_file_close(dummy_file) == APR_SUCCESS) {
          apr_file_remove(path, p);
          return 1;
        }
      }
    }
    return 0;
  }

  static apr_status_t Fixed_apr_temp_dir_get(const char **temp_dir, apr_pool_t *p)
  {
    apr_status_t apr_err;
    const char *try_dirs[] = { "/tmp", "/usr/tmp", "/var/tmp" };
    const char *try_envs[] = { "TMP", "TEMP", "TMPDIR" };
    char *cwd;
    size_t i;

    /* Our goal is to find a temporary directory suitable for writing
       into.  We'll only pay the price once if we're successful -- we
       cache our successful find.  Here's the order in which we'll try
       various paths:

          $TMP
          $TEMP
          $TMPDIR
          "/tmp"
          "/var/tmp"
          "/usr/tmp"
          `pwd`

       NOTE: This algorithm is basically the same one used by Python
       2.2's tempfile.py module.  */

    /* Try the environment first. */
    for (i = 0; i < (sizeof(try_envs) / sizeof(const char *)); i++) {
      char *value;
      apr_err = apr_env_get(&value, try_envs[i], p);
      if ((apr_err == APR_SUCCESS) && value) {
        apr_size_t len = strlen(value);
        if (len && (len < APR_PATH_MAX) && test_tempdir(value, p)) {
          memcpy(global_temp_dir, value, len + 1);
          goto end;
        }
      }
    }

    /* Next, try a set of hard-coded paths. */
    for (i = 0; i < (sizeof(try_dirs) / sizeof(const char *)); i++) {
      if (test_tempdir(try_dirs[i], p)) {
        memcpy(global_temp_dir, try_dirs[i], strlen(try_dirs[i]) + 1);
        goto end;
      }
    }

    /* Finally, try the current working directory. */
    if (APR_SUCCESS == apr_filepath_get(&cwd, APR_FILEPATH_NATIVE, p)) {
      if (test_tempdir(cwd, p)) {
        memcpy(global_temp_dir, cwd, strlen(cwd) + 1);
        goto end;
      }
    }

end:
    if (global_temp_dir[0]) {
      *temp_dir = apr_pstrdup(p, global_temp_dir);
      return APR_SUCCESS;
    }
    return APR_EGENERAL;
  }
  /* ===================================================================
   * End of inserted fixed APR code
   */

  Path
  Path::getTempDir()
  {
    const char * tempdir = NULL;
    Pool pool;

    if (apr_temp_dir_get(&tempdir, pool) != APR_SUCCESS)
    {
      tempdir = NULL;
    }

    return tempdir;
  }

  size_t
  Path::length() const
  {
    return m_path.length();
  }

  std::string
  Path::native() const
  {
    if (m_pathIsUrl)
    {
      // this converts something like
      // http://foo/my%20location
      // to
      // http://foo/my location
	    return Url::unescape(m_path.c_str());
    }
    else
    {
      // On Windows, p://foo/bar will be converted to p:\foo\bar
	    Pool pool;
      return svn_path_local_style(m_path.c_str(), pool);
    }
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
