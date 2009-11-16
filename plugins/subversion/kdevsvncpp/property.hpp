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

#ifndef _SVNCPP_PROPERTY_H_
#define _SVNCPP_PROPERTY_H_


// Ignore MSVC 6 compiler warning: debug symbol truncated
#if defined (_MSC_VER) && _MSC_VER <= 1200
#pragma warning (disable: 4786)
#endif

// Ignore MSVC 7 compiler warning: C++ exception specification
#if defined (_MSC_VER) && _MSCVER > 1200 && _MSCVER <= 1310
#pragma warning (disable: 4290)
#endif


// stl
#include "svncpp/vector_wrapper.hpp"
#include "svncpp/string_wrapper.hpp"

// svncpp
#include "svncpp/context.hpp"
#include "svncpp/path.hpp"

namespace svn
{
  struct PropertyEntry
  {
    std::string name;
    std::string value;

    PropertyEntry(const char * name, const char * value);
  };

  // forward declarations
  class Path;

  /**
   * Class for manipulating Subversion properties.
   */
  class Property
  {
  public:
    Property(Context * context = 0,
             const Path & path = "");

    virtual ~Property();

    /**
     * get the list of properties for the path.
     * throws an exception if the path isnt versioned.
     */
    const std::vector<PropertyEntry> &
    entries() const
    {
      return m_entries;
    }

    /**
     * Sets an existing property with a new value or adds a new
     * property.  If a result is added it does not reload the
     * result set.  Run loadPath again.
     * @exception ClientException
     */
    void set(const char * name, const char * value);

    /**
     * Deletes a property.
     * @exception ClientException
     */
    void remove(const char * name);

  private:
    Context * m_context;
    Path m_path;
    std::vector<PropertyEntry> m_entries;

    std::string getValue(const char * name);
    void list();
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
