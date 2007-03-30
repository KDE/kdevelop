/*
 *  Copyright (C) 2007 Dukju Ahn (dukjuahn@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 */

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
