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
#ifndef _SVNCPP_ANNOTATE_LINE_HPP_
#define _SVNCPP_ANNOTATE_LINE_HPP_

// subversion api
#include "svn_types.h"


namespace svn
{
  /**
   * This class holds the data for one line in an annotation
   */
  class AnnotateLine
  {
  public:
    AnnotateLine(apr_int64_t line_no,
                 svn_revnum_t revision,
                 const char *author,
                 const char *date,
                 const char *line)
        : m_line_no(line_no), m_revision(revision),
        m_author(author), m_date(date), m_line(line)
    {
    }

    AnnotateLine(const AnnotateLine &other)
        : m_line_no(other.m_line_no), m_revision(other.m_revision),
        m_author(other.m_author), m_date(other.m_date),
        m_line(other.m_line)
    {
    }

    /**
     * destructor
     */
    virtual ~AnnotateLine()
    {
    }

    apr_int64_t
    lineNumber() const
    {
      return m_line_no;
    }
    svn_revnum_t
    revision() const
    {
      return m_revision;
    }


    const std::string &
    author() const
    {
      return m_author;
    }


    const std::string &
    date() const
    {
      return m_date;
    }


    const std::string &
    line() const
    {
      return m_line;
    }

  private:
    apr_int64_t m_line_no;
    svn_revnum_t m_revision;
    std::string m_author;
    std::string m_date;
    std::string m_line;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
