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

// stl
#include <cstdio>

// Subversion api
#include "svn_client.h"

// svncpp
#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/exception.hpp"
#include "kdevsvncpp/pool.hpp"
#include "kdevsvncpp/status.hpp"
#include "m_check.hpp"

namespace svn
{
  std::string
  Client::cat(const Path & path,
              const Revision & revision,
              const Revision & peg_revision) throw(ClientException)
  {
    Pool pool;

    svn_stringbuf_t * stringbuf = svn_stringbuf_create("", pool);
    svn_stream_t * stream = svn_stream_from_stringbuf(stringbuf, pool);

    svn_error_t * error;
    error = svn_client_cat2(stream,
                            path.c_str(),
                            peg_revision.revision(),
                            revision.revision(),
                            *m_context,
                            pool);

    if (error != 0)
      throw ClientException(error);

    return std::string(stringbuf->data, stringbuf->len);
  }

  /**
   * Create a new temporary file in @a dstPath. If @a dstPath
   * is empty (""), then construct the temporary filename
   * from the temporary directory and the filename component
   * of @a path. The file-extension of @a path will be transformed
   * to @a dstPath and @a dstPath will be a unique filename
   *
   * @param dstPath path to temporary file. Will be constructed
   *                from @a path and temporary dir (and unique elements)
   *                if empty string
   * @param path existing filename. Necessary only for construction
   *             of @a dstPath
   * @param pool pool to use
   * @return open file
   */
  static apr_file_t *
  openTempFile(Path & dstPath, const Path & path,
               const Revision & revision, Pool & pool)
  throw(ClientException)
  {
    apr_file_t * file = 0;

    if (dstPath.length() > 0)
    {
      apr_status_t status =
        apr_file_open(&file, dstPath.c_str(),
                      APR_WRITE | APR_CREATE |
                      APR_TRUNCATE | APR_BINARY,
                      APR_OS_DEFAULT,
                      pool);
      if (status != 0)
        throw ClientException(status);
    }
    else
    {
      // split the path into its components
      std::string dir, filename, ext;
      path.split(dir, filename, ext);

      // add the revision number to the filename
      char revstring[20];
      if (revision.kind() == revision.HEAD)
        strcpy(revstring, "HEAD");
      else
        sprintf(revstring, "%" SVN_REVNUM_T_FMT, revision.revnum());
      filename += '-';
      filename += revstring;

      // replace the dir component with tempdir
      Path tempPath = Path::getTempDir();
      tempPath.addComponent(filename);

      const char * unique_name;
      svn_error_t * error =
        svn_io_open_unique_file(
          &file, &unique_name,
          tempPath.c_str(),  // path
          ext.c_str(),  // suffix
          0, // dont delete on close
          pool);

      if (error != 0)
        throw ClientException(error);

      dstPath = unique_name;
    }

    return file;
  }

  void
  Client::get(Path & dstPath,
              const Path & path,
              const Revision & revision,
              const Revision & peg_revision) throw(ClientException)
  {
    Pool pool;

    // create a new file and suppose we only want
    // this users to be able to read and write the file

    apr_file_t * file = openTempFile(dstPath, path, revision, pool);

    // now create a stream and let svn_client_cat write to the
    // stream
    svn_stream_t * stream = svn_stream_from_aprfile(file, pool);
    if (stream != 0)
    {
      svn_error_t * error = svn_client_cat2(
                              stream,
                              path.c_str(),
                              peg_revision.revision() ,
                              revision.revision(),
                              *m_context,
                              pool);

      if (error != 0)
        throw ClientException(error);

      svn_stream_close(stream);
    }

    // finalize stuff
    apr_file_close(file);
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
