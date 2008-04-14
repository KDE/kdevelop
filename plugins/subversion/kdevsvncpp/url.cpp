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
#include "svn_path.h"

// svncpp
#include "kdevsvncpp/pool.hpp"
#include "kdevsvncpp/url.hpp"

namespace svn
{
  Url::Url () {}

  Url::~Url () {}

  /**
   * determines if a path is a url; escape unsupported characters
   * before checking
   *
   * @param urlToValidate url to be validated
   */
  bool
  Url::isValid (const char * urlToValidate)
  {
    std::string escapedUrlToValidate = escape (urlToValidate);

    return svn_path_is_url (escapedUrlToValidate.c_str ()) != 0;
  }

  /**
   * returns a url with forbidden characters like spaces escaped
   *
   * @param url url to be escaped
   *
   * @return string escaped url
   */
  std::string
  Url::escape (const char * url)
  {
    Pool pool;

    return svn_path_uri_autoescape (url, pool);
  }

  /**
   * returns a url with unescaped special characters, undo changes of the
   * previous, escape function
   *
   * @param url url to be unescaped
   *
   * @return string unescaped url
   */
  std::string
  Url::unescape (const char * url)
  {
    Pool pool;

    return svn_path_uri_decode (url, pool);
  }

  /**
   * the implementation of the function that pull the supported
   * url schemas out of the ra layer it rather dirty now since
   * we are lacking a higher level of abstraction
   */
  std::vector<std::string>
  Url::supportedSchemas ()
  {
    std::vector<std::string> schemas;

    return schemas;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
