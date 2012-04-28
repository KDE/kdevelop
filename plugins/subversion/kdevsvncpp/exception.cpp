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

// stl
#include "kdevsvncpp/string_wrapper.hpp"
#include <sstream>

// svncpp
#include "kdevsvncpp/exception.hpp"


namespace svn
{

  struct Exception::Data
  {
public:
    std::string message;
    apr_status_t apr_err;

    Data(const char * msg)
        : message(msg)
    {
    }


    Data(const Data& other)
        : message(other.message), apr_err(other.apr_err)
    {
    }
  };

  Exception::Exception(const char * message) throw()
  {
    m = new Data(message);
  }

  Exception::Exception(const Exception & other) throw()
  {
    m = new Data(*other.m);
  }

  Exception::~Exception() throw()
  {
    delete m;
  }

  apr_status_t
  Exception::apr_err() const
  {
    return m->apr_err;
  }

  const char *
  Exception::message() const
  {
    return m->message.c_str();
  }


  ClientException::ClientException(svn_error_t * error) throw()
      : Exception("")
  {
    if (error == 0)
      return;

    m->apr_err = error->apr_err;
    svn_error_t * next = error->child;
    /// @todo send rapidsvn an hint that error->message may sometimes NULL!
    std::string & message = m->message;
    if (error->message)
      message = error->message;
    else
    {
      message = "Unknown error!\n";
      if (error->file)
      {
        message += "In file ";
        message += error->file;
        std::stringstream num;
        num << " Line " << error->line;
        message += num.str();
      }
    }
    while (next != NULL && next->message != NULL)
    {
      message = message + '\n' + next->message;

      next = next->child;
    }
    svn_error_clear(error);
  }


  ClientException::ClientException(apr_status_t status) throw()
      : Exception("")
  {
    m->apr_err = status;
  }


  ClientException::~ClientException() throw()
  {
  }

  ClientException::ClientException(const ClientException & src) throw()
      : Exception(src.message())
  {
  }
}
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
