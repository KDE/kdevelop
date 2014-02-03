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

/**
 * @todo implement 1.3 SVN api:
 *
 *  SVN_CLIENT_COMMIT_ITEM_LOCK_TOKEN
 *  svn_client_add3
 *  svn_client_copy2
 *  svn_client_commit3
 *  svn_client_delete2
 *  svn_client_move3
 *  svn_client_mkdir2
 *  svn_client_import2
 *  svn_client_info
 */


// Apache Portable Runtime
#include "apr_xlate.h"

// Subversion api
#include "svn_auth.h"
#include "svn_config.h"
#include "svn_subst.h"
//#include "svn_utf.h"

// svncpp
#include "kdevsvncpp/apr.hpp"
#include "kdevsvncpp/context.hpp"
#include "kdevsvncpp/context_listener.hpp"

namespace svn
{
  struct Context::Data
  {
public:
    /** The usage of Apr makes sure Apr is initialized
     * before any use of apr functions.
     */
    Apr apr;

    ContextListener * listener;
    bool logIsSet;
    int promptCounter;
    Pool pool;
    svn_client_ctx_t * ctx;
    std::string username;
    std::string password;
    std::string logMessage;
    std::string configDir;

    /**
     * translate native c-string to utf8
     */
    static svn_error_t *
    translateString(const char * str, const char ** newStr,
                    apr_pool_t * /*pool*/)
    {
      // due to problems with apr_xlate we dont perform
      // any conversion at this place. YOU will have to make
      // sure any strings passed are UTF 8 strings
      // svn_string_t *string = svn_string_create ("", pool);
      //
      // string->data = str;
      // string->len = strlen (str);
      //
      // const char * encoding = APR_LOCALE_CHARSET;
      //
      // SVN_ERR (svn_subst_translate_string (&string, string,
      //                                      encoding, pool));
      //
      // *newStr = string->data;
      *newStr = str;
      return SVN_NO_ERROR;
    }

    /**
     * the @a baton is interpreted as Data *
     * Several checks are performed on the baton:
     * - baton == 0?
     * - baton->Data
     * - listener set?
     *
     * @param baton
     * @param data returned data if everything is OK
     * @retval SVN_NO_ERROR if everything is fine
     * @retval SVN_ERR_CANCELLED on invalid values
     */
    static svn_error_t *
    getData(void * baton, Data ** data)
    {
      if (baton == NULL)
        return svn_error_create(SVN_ERR_CANCELLED, NULL,
                                "invalid baton");

      Data * data_ = static_cast <Data *>(baton);

      if (data_->listener == 0)
        return svn_error_create(SVN_ERR_CANCELLED, NULL,
                                "invalid listener");

      *data = data_;
      return SVN_NO_ERROR;
    }

    Data(const std::string & configDir_)
        : listener(0), logIsSet(false),
        promptCounter(0), configDir(configDir_)
    {
      const char * c_configDir = 0;
      if (configDir.length() > 0)
        c_configDir = configDir.c_str();

      // make sure the configuration directory exists
      svn_config_ensure(c_configDir, pool);


      // initialize authentication providers
      // * simple
      // * username
      // * simple prompt
      // * ssl server trust file
      // * ssl server trust prompt
      // * ssl client cert pw file
      // * ssl client cert pw prompt
      // * ssl client cert file
      // ===================
      // 8 providers

      apr_array_header_t *providers =
        apr_array_make(pool, 8,
                       sizeof(svn_auth_provider_object_t *));
      svn_auth_provider_object_t *provider;

      svn_client_get_simple_provider(
        &provider,
        pool);
      *(svn_auth_provider_object_t **)apr_array_push(providers) =
        provider;

      svn_client_get_username_provider(
        &provider,
        pool);
      *(svn_auth_provider_object_t **)apr_array_push(providers) =
        provider;

      svn_client_get_simple_prompt_provider(
        &provider,
        onSimplePrompt,
        this,
        100000000, // not very nice. should be infinite...
        pool);
      *(svn_auth_provider_object_t **)apr_array_push(providers) =
        provider;

      // add ssl providers

      // file first then prompt providers
      svn_client_get_ssl_server_trust_file_provider(&provider, pool);
      *(svn_auth_provider_object_t **)apr_array_push(providers) =
        provider;

      svn_client_get_ssl_client_cert_file_provider(&provider, pool);
      *(svn_auth_provider_object_t **)apr_array_push(providers) =
        provider;

      svn_client_get_ssl_client_cert_pw_file_provider(&provider, pool);
      *(svn_auth_provider_object_t **)apr_array_push(providers) =
        provider;

      svn_client_get_ssl_server_trust_prompt_provider(
        &provider, onSslServerTrustPrompt, this, pool);
      *(svn_auth_provider_object_t **)apr_array_push(providers) =
        provider;

      // plugged in 3 as the retry limit - what is a good limit?
      svn_client_get_ssl_client_cert_pw_prompt_provider(
        &provider, onSslClientCertPwPrompt, this, 3, pool);
      *(svn_auth_provider_object_t **)apr_array_push(providers) =
        provider;

      svn_auth_baton_t *ab;
      svn_auth_open(&ab, providers, pool);

      // initialize ctx structure
      svn_client_create_context(&ctx, pool);

      // get the config based on the configDir passed in
      svn_config_get_config(&ctx->config, c_configDir, pool);

      // disable external diff and diff3 commands
      svn_config_t *config = (svn_config_t *)apr_hash_get(
        ctx->config, SVN_CONFIG_CATEGORY_CONFIG, APR_HASH_KEY_STRING);
      svn_config_set(config, SVN_CONFIG_SECTION_HELPERS,
                     SVN_CONFIG_OPTION_DIFF_CMD, NULL);
      svn_config_set(config, SVN_CONFIG_SECTION_HELPERS,
                     SVN_CONFIG_OPTION_DIFF3_CMD, NULL);

      // tell the auth functions where the config is
      svn_auth_set_parameter(ab, SVN_AUTH_PARAM_CONFIG_DIR,
                             c_configDir);

      ctx->auth_baton = ab;
      ctx->log_msg_func = onLogMsg;
      ctx->log_msg_baton = this;
      ctx->notify_func = onNotify;
      ctx->notify_baton = this;
      ctx->cancel_func = onCancel;
      ctx->cancel_baton = this;

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
      ctx->notify_func2 = onNotify2;
      ctx->notify_baton2 = this;
#endif
    }

    void setAuthCache(bool value)
    {
      void *param = 0;
      if (!value)
        param = (void *)"1";

      svn_auth_set_parameter(ctx->auth_baton,
                             SVN_AUTH_PARAM_NO_AUTH_CACHE,
                             param);
    }

    /** @see Context::setLogin */
    void setLogin(const char * usr, const char * pwd)
    {
      username = usr;
      password = pwd;

      svn_auth_baton_t * ab = ctx->auth_baton;
      svn_auth_set_parameter(ab, SVN_AUTH_PARAM_DEFAULT_USERNAME,
                             username.c_str());
      svn_auth_set_parameter(ab, SVN_AUTH_PARAM_DEFAULT_PASSWORD,
                             password.c_str());

    }

    /** @see Context::setLogMessage */
    void setLogMessage(const char * msg)
    {
      logMessage = msg;
      logIsSet = true;
    }

    /**
     * this function gets called by the subversion api function
     * when a log message is needed. This is the case on a commit
     * for example
     */
    static svn_error_t *
    onLogMsg(const char **log_msg,
             const char **tmp_file,
             apr_array_header_t *, //UNUSED commit_items
             void *baton,
             apr_pool_t * pool)
    {
      Data * data = NULL;
      SVN_ERR(getData(baton, &data));

      std::string msg;
      if (data->logIsSet)
        msg = data->getLogMessage();
      else
      {
        if (!data->retrieveLogMessage(msg))
          return svn_error_create(SVN_ERR_CANCELLED, NULL, "");
      }

      *log_msg = apr_pstrdup(pool, msg.c_str());

      *tmp_file = NULL;

      return SVN_NO_ERROR;
    }

    /**
     * this is the callback function for the subversion
     * api functions to signal the progress of an action
     */
    static void
    onNotify(void * baton,
             const char *path,
             svn_wc_notify_action_t action,
             svn_node_kind_t kind,
             const char *mime_type,
             svn_wc_notify_state_t content_state,
             svn_wc_notify_state_t prop_state,
             svn_revnum_t revision)
    {
      if (baton == 0)
        return;

      Data * data = static_cast <Data *>(baton);

      data->notify(path, action, kind, mime_type, content_state,
                   prop_state, revision);
    }


#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    /**
     * this is the callback function for the subversion 1.2
     * api functions to signal the progress of an action
     *
     * @todo right now we forward only to @a onNotify,
     *       but maybe we should a notify2 to the listener
     * @since subversion 1.2
     */
    static void
    onNotify2(void*baton,const svn_wc_notify_t *action,apr_pool_t *)
    {
      if (!baton)
        return;

      // for now forward the call to @a onNotify
      onNotify(baton, action->path, action->action,
               action->kind, action->mime_type,
               action->content_state, action->prop_state,
               action->revision);
    }
#endif


    /**
     * this is the callback function for the subversion
     * api functions to signal the progress of an action
     */
    static svn_error_t *
    onCancel(void * baton)
    {
      if (baton == 0)
        return SVN_NO_ERROR;

      Data * data = static_cast <Data *>(baton);

      if (data->cancel())
        return svn_error_create(SVN_ERR_CANCELLED, NULL, "cancelled by user");
      else
        return SVN_NO_ERROR;
    }

    /**
     * @see svn_auth_simple_prompt_func_t
     */
    static svn_error_t *
    onSimplePrompt(svn_auth_cred_simple_t **cred,
                   void *baton,
                   const char *realm,
                   const char *username,
                   svn_boolean_t _may_save,
                   apr_pool_t *pool)
    {
      Data * data = NULL;
      SVN_ERR(getData(baton, &data));

      bool may_save = _may_save != 0;
      if (!data->retrieveLogin(username, realm, may_save))
        return svn_error_create(SVN_ERR_CANCELLED, NULL, "");

      svn_auth_cred_simple_t* lcred = (svn_auth_cred_simple_t*)
                                      apr_palloc(pool, sizeof(svn_auth_cred_simple_t));
      /*      SVN_ERR (svn_utf_cstring_to_utf8 (
                       &lcred->password,
                       data->getPassword (), pool));
            SVN_ERR (svn_utf_cstring_to_utf8 (
                       &lcred->username,
                       data->getUsername (), pool)); */
      lcred->password = data->getPassword();
      lcred->username = data->getUsername();

      // tell svn if the credentials need to be saved
      lcred->may_save = may_save;
      *cred = lcred;

      return SVN_NO_ERROR;
    }

    /**
     * @see svn_auth_ssl_server_trust_prompt_func_t
     */
    static svn_error_t *
    onSslServerTrustPrompt(svn_auth_cred_ssl_server_trust_t **cred,
                           void *baton,
                           const char *realm,
                           apr_uint32_t failures,
                           const svn_auth_ssl_server_cert_info_t *info,
                           svn_boolean_t may_save,
                           apr_pool_t *pool)
    {
      Data * data = NULL;
      SVN_ERR(getData(baton, &data));

      ContextListener::SslServerTrustData trustData(failures);
      if (realm != NULL)
        trustData.realm = realm;
      trustData.hostname = info->hostname;
      trustData.fingerprint = info->fingerprint;
      trustData.validFrom = info->valid_from;
      trustData.validUntil = info->valid_until;
      trustData.issuerDName = info->issuer_dname;
      trustData.maySave = may_save != 0;

      apr_uint32_t acceptedFailures;
      ContextListener::SslServerTrustAnswer answer =
        data->listener->contextSslServerTrustPrompt(
          trustData, acceptedFailures);

      if (answer == ContextListener::DONT_ACCEPT)
        *cred = NULL;
      else
      {
        svn_auth_cred_ssl_server_trust_t *cred_ =
          (svn_auth_cred_ssl_server_trust_t*)
          apr_palloc(pool, sizeof(svn_auth_cred_ssl_server_trust_t));

        if (answer == ContextListener::ACCEPT_PERMANENTLY)
        {
          cred_->may_save = 1;
          cred_->accepted_failures = acceptedFailures;
        }
        *cred = cred_;
      }

      return SVN_NO_ERROR;
    }

    /**
     * @see svn_auth_ssl_client_cert_prompt_func_t
     */
    static svn_error_t *
    onSslClientCertPrompt(svn_auth_cred_ssl_client_cert_t **cred,
                          void *baton,
                          apr_pool_t *pool)
    {
      Data * data = NULL;
      SVN_ERR(getData(baton, &data));

      std::string certFile;
      if (!data->listener->contextSslClientCertPrompt(certFile))
        return svn_error_create(SVN_ERR_CANCELLED, NULL, "");

      svn_auth_cred_ssl_client_cert_t *cred_ =
        (svn_auth_cred_ssl_client_cert_t*)
        apr_palloc(pool, sizeof(svn_auth_cred_ssl_client_cert_t));

      /*      SVN_ERR (svn_utf_cstring_to_utf8 (
                       &cred_->cert_file,
                       certFile.c_str (),
                       pool)); */
      cred_->cert_file = certFile.c_str();

      *cred = cred_;

      return SVN_NO_ERROR;
    }

    /**
     * @see svn_auth_ssl_client_cert_pw_prompt_func_t
     */
    static svn_error_t *
    onSslClientCertPwPrompt(
      svn_auth_cred_ssl_client_cert_pw_t **cred,
      void *baton,
      const char *realm,
      svn_boolean_t maySave,
      apr_pool_t *pool)
    {
      Data * data = NULL;
      SVN_ERR(getData(baton, &data));

      std::string password;
      bool may_save = maySave != 0;
      if (!data->listener->contextSslClientCertPwPrompt(password, realm, may_save))
        return svn_error_create(SVN_ERR_CANCELLED, NULL, "");

      svn_auth_cred_ssl_client_cert_pw_t *cred_ =
        (svn_auth_cred_ssl_client_cert_pw_t *)
        apr_palloc(pool, sizeof(svn_auth_cred_ssl_client_cert_pw_t));

      /*      SVN_ERR (svn_utf_cstring_to_utf8 (
                       &cred_->password,
                       password.c_str (),
                       pool)); */
      cred_->password = password.c_str();

      cred_->may_save = may_save;
      *cred = cred_;

      return SVN_NO_ERROR;
    }

    const char *
    getUsername() const
    {
      return username.c_str();
    }

    const char *
    getPassword() const
    {
      return password.c_str();
    }

    const char *
    getLogMessage() const
    {
      return logMessage.c_str();
    }

    /**
     * if the @a listener is set, use it to retrieve the log
     * message using ContextListener::contextGetLogMessage.
     * This return values is given back, then.
     *
     * if the @a listener is not set the its checked whether
     * the log message has been set using @a setLogMessage
     * yet. If not, return false otherwise true
     *
     * @param msg log message
     * @retval false cancel
     */
    bool
    retrieveLogMessage(std::string & msg)
    {
      bool ok;

      if (listener == 0)
        return false;

      ok = listener->contextGetLogMessage(logMessage);
      if (ok)
        msg = logMessage;
      else
        logIsSet = false;

      return ok;
    }

    /**
     * if the @a listener is set and no password has been
     * set yet, use it to retrieve login and password using
     * ContextListener::contextGetLogin.
     *
     * if the @a listener is not set, check if setLogin
     * has been called yet.
     *
     * @return continue?
     * @retval false cancel
     */
    bool
    retrieveLogin(const char * username_,
                  const char * realm,
                  bool &may_save)
    {
      bool ok;

      if (listener == 0)
        return false;

      if (username_ == NULL)
        username = "";
      else
        username = username_;

      ok = listener->contextGetLogin(realm, username, password, may_save);

      return ok;
    }

    /**
     * if the @a listener is set call the method
     * @a contextNotify
     */
    void
    notify(const char *path,
           svn_wc_notify_action_t action,
           svn_node_kind_t kind,
           const char *mime_type,
           svn_wc_notify_state_t content_state,
           svn_wc_notify_state_t prop_state,
           svn_revnum_t revision)
    {
      if (listener != 0)
      {
        listener->contextNotify(path, action, kind, mime_type,
                                content_state, prop_state, revision);
      }
    }

    /**
     * if the @a listener is set call the method
     * @a contextCancel
     */
    bool
    cancel()
    {
      if (listener != 0)
      {
        return listener->contextCancel();
      }
      else
      {
        // don't cancel if no listener
        return false;
      }
    }
  };

  Context::Context(const std::string &configDir)
  {
    m = new Data(configDir);
  }

  Context::Context(const Context & src)
  {
    m = new Data(src.m->configDir);
    setLogin(src.getUsername(), src.getPassword());
  }

  Context::~Context()
  {
    delete m;
  }

  void
  Context::setAuthCache(bool value)
  {
    m->setAuthCache(value);
  }

  void
  Context::setLogin(const char * username, const char * password)
  {
    m->setLogin(username, password);
  }

  Context::operator svn_client_ctx_t * ()
  {
    return m->ctx;
  }

  svn_client_ctx_t *
  Context::ctx()
  {
    return m->ctx;
  }

  void
  Context::setLogMessage(const char * msg)
  {
    m->setLogMessage(msg);
  }

  const char *
  Context::getUsername() const
  {
    return m->getUsername();
  }

  const char *
  Context::getPassword() const
  {
    return m->getPassword();
  }

  const char *
  Context::getLogMessage() const
  {
    return m->getLogMessage();
  }

  void
  Context::setListener(ContextListener * listener)
  {
    m->listener = listener;
  }

  ContextListener *
  Context::getListener() const
  {
    return m->listener;
  }

  void
  Context::reset()
  {
    m->promptCounter = 0;
    m->logIsSet = false;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
