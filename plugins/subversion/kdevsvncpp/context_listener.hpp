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

#ifndef _SVNCPP_CONTEXT_LISTENER_HPP_
#define _SVNCPP_CONTEXT_LISTENER_HPP_

// stl
#include "svncpp/string_wrapper.hpp"

// Subversion api
#include "svn_client.h"

// svncpp
#include "svncpp/pool.hpp"

namespace svn
{
  /**
   * This is the interface that is used by @a Context
   * for callbacks.
   * To use this you will have to inherit from this
   * interface and overwrite the virtual methods.
   */
  class ContextListener
  {
  public:
    /**
     * this method will be called to retrieve
     * authentication information
     *
     * WORKAROUND FOR apr_xlate PROBLEM:
     * STRINGS ALREADY HAVE TO BE UTF8!!!
     *
     * @param username
     * @param realm in which username/password will be used
     * @param password
     * @param maySave in/out set false to not save
     * @return continue action?
     * @retval true continue
     */
    virtual bool
    contextGetLogin(const std::string & realm,
                    std::string & username,
                    std::string & password,
                    bool & maySave) = 0;

    /**
     * this method will be called to notify about
     * the progress of an ongoing action
     *
     * @param path
     * @param action
     * @param kind
     * @param mime_type
     * @param content_state
     * @param prop_state
     * @param revision
     */
    virtual void
    contextNotify(const char *path,
                  svn_wc_notify_action_t action,
                  svn_node_kind_t kind,
                  const char *mime_type,
                  svn_wc_notify_state_t content_state,
                  svn_wc_notify_state_t prop_state,
                  svn_revnum_t revision) = 0;

    /*
     * this method will be called periodically to allow
     * the app to cancel long running operations
     *
     * @return cancel action?
     * @retval true cancel
     */
    virtual bool
    contextCancel() = 0;

    /**
     * this method will be called to retrieve
     * a log message
     *
     * WORKAROUND FOR apr_xlate PROBLEM:
     * STRINGS ALREADY HAVE TO BE UTF8!!!
     *
     * @param msg log message
     * @return continue action?
     * @retval true continue
     */
    virtual bool
    contextGetLogMessage(std::string & msg) = 0;

    typedef enum
    {
      DONT_ACCEPT = 0,
      ACCEPT_TEMPORARILY,
      ACCEPT_PERMANENTLY
    } SslServerTrustAnswer;


    /**
     * @see contextSslServerTrust
     * @see svn_auth_cred_ssl_server_trust_t
     */
    struct SslServerTrustData
    {
public:
      /** bit coded failures */
      apr_uint32_t failures;

      /** certificate information */
      std::string hostname;
      std::string fingerprint;
      std::string validFrom;
      std::string validUntil;
      std::string issuerDName;
      std::string realm;
      bool maySave;

      SslServerTrustData(const apr_uint32_t failures_ = 0)
          : failures(failures_), hostname(""), fingerprint(""),
          validFrom(""), validUntil(""), issuerDName(""),
          realm(""), maySave(true)
      {
      }

      SslServerTrustData(const SslServerTrustData & src)
          : failures(src.failures)
      {
        hostname = src.hostname;
        fingerprint = src.fingerprint;
        validFrom = src.validFrom;
        validUntil = src.validUntil;
        issuerDName = src.issuerDName;
        realm = src.realm;
        maySave = src.maySave;
      }

      SslServerTrustData &
      operator =(const SslServerTrustData & src)
      {
        if (this == &src)
          return *this;

        hostname = src.hostname;
        fingerprint = src.fingerprint;
        validFrom = src.validFrom;
        validUntil = src.validUntil;
        issuerDName = src.issuerDName;
        realm = src.realm;
        maySave = src.maySave;
        failures = src.failures;

        return *this;
      }
    };


    /**
     * this method is called if there is ssl server
     * information, that has to be confirmed by the user
     *
     * @param data
     * @param acceptedFailures
     * @return @a SslServerTrustAnswer
     */
    virtual SslServerTrustAnswer
    contextSslServerTrustPrompt(const SslServerTrustData & data,
                                apr_uint32_t & acceptedFailures) = 0;

    /**
     * this method is called to retrieve client side
     * information
     */
    virtual bool
    contextSslClientCertPrompt(std::string & certFile) = 0;

    /**
     * this method is called to retrieve the password
     * for the client certificate
     *
     * @param password
     * @param realm
     * @param maySave
     */
    virtual bool
    contextSslClientCertPwPrompt(std::string & password,
                                 const std::string & realm,
                                 bool & maySave) = 0;

    virtual ~ContextListener() { }
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
