/*
    This file is part of the KDE Project

    Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSVND_H
#define KSVND_H

#include <dcopclient.h>
#include <kdedmodule.h>
#include <kurl.h>
#include <qstringlist.h>

class KDevSvnd : public KDEDModule
{
Q_OBJECT

K_DCOP

  //note: InSVN means parent is added.  InRepos  means itself is added
	enum { SomeAreFiles = 1, SomeAreFolders = 2,  SomeAreInParentsEntries = 4, SomeParentsHaveSvn = 8, SomeHaveSvn = 16, SomeAreExternalToParent = 32, AllAreInParentsEntries = 64, AllParentsHaveSvn = 128, AllHaveSvn = 256, AllAreExternalToParent = 512, AllAreFolders = 1024 };
public:
	KDevSvnd(const QCString &);
	~KDevSvnd();

k_dcop:
//  void addAuthInfo(KIO::AuthInfo, long);
	QString commitDialog(QString);
	int sslServerTrustPrompt(QString certFailMsg, QString hostname, QString fingerPrint, QString validfrom, QString validuntil, QString issuerName, QString ascii_cert );
	QString sslCertFile();
	QCString sslPasswdDlg(QString promptMsg);

};

#endif
