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

// apr
#include "apr_date.h"

// svncpp
#include "kdevsvncpp/datetime.hpp"


namespace svn
{
  DateTime::DateTime()
      : m_time(APR_DATE_BAD)
  {
  }

  DateTime::DateTime(const apr_time_t time)
      : m_time(time)
  {
  }

  DateTime::DateTime(const DateTime & dateTime)
      : m_time(dateTime.m_time)
  {
  }

  const DateTime &
  DateTime::operator =(const DateTime & dateTime)
  {
    m_time = dateTime.m_time;
    return *this;
  }

  bool
  DateTime::operator ==(const DateTime & dateTime)
  {
    return m_time == dateTime.m_time;
  }

  bool
  DateTime::operator !=(const DateTime & dateTime)
  {
    return m_time != dateTime.m_time;
  }

  bool
  DateTime::IsValid() const
  {
    return m_time != APR_DATE_BAD;
  }

  apr_time_t
  DateTime::GetAPRTimeT() const
  {
    return m_time;
  }

  bool
  DateTime::SetRFC822Date(const char* date)
  {
    m_time = apr_date_parse_rfc(date);
    return IsValid();
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
