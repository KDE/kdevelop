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

#ifndef _SVNCPP_EXCEPTION_H_
#define _SVNCPP_EXCEPTION_H_

// subversion api
#include "svn_client.h"


namespace svn
{

  /**
   * Generic exception class.
   */
  class Exception
  {
  public:
    /**
     * Constructor.  Assigns the exception reason.
     */
    Exception(const char * message) throw();

    ~Exception() throw();

    /**
     * @return the exception message.
     */
    const char * message() const;

    /**
     * @return the outermost error code.
     */
    apr_status_t apr_err() const;

  protected:
    struct Data;
    Data * m;

  private:

    Exception(const Exception &) throw();

    Exception() throw();

    Exception & operator = (const Exception &);
  };

  /**
   * Subversion client exception class.
   */
  class ClientException : public Exception
  {
  public:
    /**
     * Constructor.  Sets the error template and an optional message.
     */
    ClientException(svn_error_t * error) throw();


    /**
     * Constructor that takes only an apr errorcode
     */
    ClientException(apr_status_t status) throw();

    ClientException(const char * message) throw()
        : Exception(message)
    {
    }


    /**
     * Copy constructor
     */
    ClientException(const ClientException & src) throw();

    virtual ~ClientException() throw();

  private:
    ClientException() throw();

    ClientException & operator = (ClientException &);
  };

}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
