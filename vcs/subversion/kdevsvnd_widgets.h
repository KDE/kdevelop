#ifndef KDEVSVND_WIDGETS_H
#define KDEVSVND_WIDGETS_H

#include "svnssltrustpromptbase.h"

class SvnSSLTrustPrompt : public SvnSSLTrustPromptBase{
Q_OBJECT
public:
	SvnSSLTrustPrompt( QWidget* parent=0, const char* name=0, bool modal=true, WFlags f=0 );
	~SvnSSLTrustPrompt();
	void setupCertInfo( QString hostname, QString fingerPrint, QString validfrom, QString validuntil, QString issuerName, QString ascii_cert );
	void setupFailedReasonMsg( QString msg );
	int code();

public slots:
	void setPermanent();
	void setTemporary();
	void setRejected();

protected:
	// -1 for reject
	// 0 for accept temporarily
	// 1 for accept permanently
	int m_code;	
};

#endif
