#ifndef SVN_SSLTRUSTDIALOG_H
#define SVN_SSLTRUSTDIALOG_H

#include "ui_svnssltrustdialog.h"
#include "ui_svnlogindialog.h"

struct svn_auth_ssl_server_cert_info_t;

class SvnSSLTrustDialog: public QDialog, public Ui::SvnSSLTrustDialog
{
    Q_OBJECT
public:
    SvnSSLTrustDialog( QWidget *parent = 0 );
    virtual ~SvnSSLTrustDialog();
    
    void setFailedReasons( unsigned int failures );
    void setCertInfos( const svn_auth_ssl_server_cert_info_t *ci );
    void setCertInfos( QString hostname,QString fingerPrint,
                       QString validfrom,QString validuntil,
                       QString issuerName, QString ascii_cert);
    int userDecision();
    
protected Q_SLOTS:
    virtual void accept();
    virtual void reject();
    
private:
    int m_decision;
};

class SvnLoginDialog : public QDialog, public Ui::SvnLoginDialog
{
    Q_OBJECT
public:
    SvnLoginDialog( QWidget *parent = 0 );
    virtual ~SvnLoginDialog();

    void setRealm( QString &realm );
    
};

#endif
