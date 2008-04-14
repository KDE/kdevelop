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

#ifndef _SVNCPP_WC_HPP_
#define _SVNCPP_WC_HPP_ 

// svncpp
#include "svncpp/revision.hpp"


namespace svn
{
  // forward declarations
  class Path;

  /**
   * Class that deals with a working copy
   */
  class Wc
  {
  public:
    /**
     * check if Path is a valid working directory
     *
     * @param dir path to a directory
     * @return true=valid working copy
     */
    static bool 
    checkWc (const char * dir);

    static bool
    checkWc (const Path & dir);

    /**
     * ensure that an administrative area exists for @a dir, so that @a dir
     * is a working copy subdir based on @a url at @a revision.
     *
     * @param dir path to a directory
     * @param uuid
     * @param url corresponding url
     * @param revision expected working copy revision
     */
    static void
    ensureAdm (const char * dir, const char * uuid, 
               const char * url, const Revision & revision);

    /**
     * use \a dir as name for the subversion administrative directory
     * instead of the standard ".svn"
     *
     * @param dir
     */
    static void 
    setAdmDir (const char * dir);

    /**
     * checks whether @a name is an administrative directory
     * 
     * @remarks @a name may only be a filename, not an absolute path
     */
    static bool
    isAdmDir (const char * name);

  private:
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
