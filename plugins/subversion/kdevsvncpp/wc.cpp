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
#include "svn_wc.h"

// svncpp
#include "kdevsvncpp/exception.hpp"
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/pool.hpp"
#include "kdevsvncpp/wc.hpp"


namespace svn
{
  bool 
  Wc::checkWc (const char * dir)
  {
    Path path (dir);
    return Wc::checkWc (path);
  }

  bool
  Wc::checkWc (const Path & dir)
  {
    Pool pool;
    int wc;

    svn_error_t * error =
      svn_wc_check_wc (dir.c_str (), &wc, pool);

    if ((error != NULL) || (wc == 0))
    {
      return false;
    }

    return true;
  }

  void
  Wc::ensureAdm (const char * dir, const char *uuid, 
                 const char * url, const Revision & revision)
  {
    Pool pool;
    Path dirPath (dir);
    Path urlPath (url);

    svn_error_t * error =
      svn_wc_ensure_adm (dirPath.c_str (),    // path 
                         uuid,                // UUID 
                         urlPath.c_str (),    // url
                         revision.revnum (),  // revision
                         pool);

    if(error != NULL)
      throw ClientException (error);
  }

  void
  Wc::setAdmDir (const char * dir)
  {
    Pool pool;

    svn_error_t * error = 
      svn_wc_set_adm_dir (dir, pool);

    if(error != NULL)
      throw ClientException (error);
  }

  bool
  Wc::isAdmDir (const char * name)
  {
    Pool pool;

    return 0 != svn_wc_is_adm_dir (name, pool);
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
