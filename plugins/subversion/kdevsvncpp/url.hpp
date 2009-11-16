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
#ifndef _SVNCPP_URL_HPP_
#define _SVNCPP_URL_HPP_

// stl
#include "svncpp/string_wrapper.hpp"
#include "svncpp/vector_wrapper.hpp"

namespace svn
{
  class Url
  {
  public:
    /** Constructor */
    Url();

    /** Destructor */
    virtual ~Url();

    /**
     * Checks if @a url is valid
     *
     * Example of a valid URL:
     *   http://svn.collab.net/repos/svn
     * Example of an invalid URL:
     *   /home/foo/bar
     */
    static bool
    isValid(const char * urlToValidate);

    /**
     * returns a url with forbidden charachters like spaces escaped
     *
     * Example of input:
     *   http://rapidsvn.tigris.org/x y z.html
     * Output:
     *   http://rapidsvn.tigris.org/x%20y%20z.html
     */
    static std::string
    escape(const char * url);

    /**
     * returns a url with forbidden charachters like spaces unescaped
     *
     * Undo the changes done by the previous function, escape ()
     */
    static std::string
    unescape(const char * url);

    /**
     * returns a vector with url schemas that are
     * supported by svn
     *
     * @return vector with entries like "file:", "http:"
     * @deprecated since 0.9.1 (returns an empty vector now)
     */
    static std::vector<std::string>
    supportedSchemas();
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */

