/*
    This file is part of the KDE Project

    Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>

#include "config.h"

#include "ksvnd.h"
#include "commitdlg.h"

extern "C" {
    KDEDModule *create_ksvnd(const QCString &name) {
       return new KSvnd(name);
    }
}

KSvnd::KSvnd(const QCString &name)
 : KDEDModule(name) {
}

KSvnd::~KSvnd() {
}

QString KSvnd::commitDialog(QString modifiedFiles) {
	CommitDlg commitDlg;
	commitDlg.setLog( modifiedFiles );
	int result = commitDlg.exec();
	if ( result == QDialog::Accepted ) {
		return commitDlg.logMessage();
	} else
		return QString::null;
}

void KSvnd::notify(const QString& path, int action, int kind, const QString& mime_type, int content_state, int prop_state, long int revision) {
	kdDebug() << "KDED/Subversion : notify " << path << " action : " << action << " mime_type : " << mime_type << " content_state : " << content_state << " prop_state : " << prop_state << " revision : " << revision << endl; 
	QByteArray params;

	QDataStream stream(params, IO_WriteOnly);
	stream << path << action << kind << mime_type << content_state << prop_state << revision;

	kapp->dcopClient()->emitDCOPSignal( "subversionNotify(QString,int,int,QString,int,int,long int)", params );
}

void KSvnd::status(const QString& path, int text_status, int prop_status, int repos_text_status, int repos_prop_status ) {
	kdDebug() << "KDED/Subversion : status " << path << " " << text_status << " " << prop_status << " "
			<< repos_text_status << " " << repos_prop_status << endl;
	QByteArray params;

	QDataStream stream(params, IO_WriteOnly);
	stream << path << text_status << prop_status << repos_text_status << repos_prop_status;

	kapp->dcopClient()->emitDCOPSignal( "subversionStatus(QString,int,int,int,int)", params );
}

#include "ksvnd.moc"
