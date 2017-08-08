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

#ifndef _SVNCPP_M_CHECK_HPP_
#define _SVNCPP_M_CHECK_HPP_

// subversion api
#include "svn_version.h"

#ifndef SVN_VER_MAJOR
#error "SVN_VER_MAJOR not defined"
#endif

#ifndef SVN_VER_MINOR
#error "SVN_VER_MINOR not defined"
#endif

/**
 * Check if the current version of the subversion
 * API is at least major.minor
 */
#define CHECK_SVN_VERSION(major,minor) \
  ((SVN_VER_MAJOR > (major)) || \
  ((SVN_VER_MAJOR == (major)) && (SVN_VER_MINOR >= (minor))))

#endif

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
