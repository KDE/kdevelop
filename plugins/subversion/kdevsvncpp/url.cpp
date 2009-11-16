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
#include "svn_path.h"

// svncpp
#include "kdevsvncpp/pool.hpp"
#include "kdevsvncpp/url.hpp"

static void findAndReplace(std::string & source, const std::string & find, const std::string & replace)
{
  // start seaching from the beginning
  size_t pos = 0;
  size_t findLength = find.length();
  size_t replaceLength = replace.length();

  do
  {
    // search for the next occurrenc
    pos = source.find(find, pos);
    
    // found?
    if (pos != std::string::npos)
    {
      // yes, place
      source.replace(pos, findLength, replace);

      // Make sure we dont search from the beginning
      // othwise replacing % with %25 would result 
      // in an endless loop
      pos = pos + replaceLength;
    }
  }
  while (pos != std::string::npos);
}

namespace svn
{
  Url::Url() {}

  Url::~Url() {}

  bool
  Url::isValid(const char * urlToValidate)
  {
    return svn_path_is_url(urlToValidate) != 0;
  }


  std::string
  Url::escape(const char * url)
  {
    Pool pool;

    // First make sure % gets escaped
    std::string partlyEscaped(url);
    findAndReplace(partlyEscaped, "%", "%25");

    // Let svn do the first part of the work
    partlyEscaped=svn_path_uri_autoescape(partlyEscaped.c_str(), pool);

    // Then worry about the rest
    findAndReplace(partlyEscaped, "#", "%23");
    findAndReplace(partlyEscaped, ";", "%3B");
    findAndReplace(partlyEscaped, "?", "%3F");
    findAndReplace(partlyEscaped, "[", "%5B");
    findAndReplace(partlyEscaped, "]", "%5D");

    return partlyEscaped;
  }


  std::string
  Url::unescape(const char * url)
  {
    Pool pool;

    return svn_path_uri_decode(url, pool);
  }

  /**
   * the implementation of the function that pull the supported
   * url schemas out of the ra layer it rather dirty now since
   * we are lacking a higher level of abstraction
   */
  std::vector<std::string>
  Url::supportedSchemas()
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
