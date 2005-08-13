/***************************************************************************
 *   Copyright (C) 2004 by ian reinhart geiser                             *
 *   geiseri@kde.org                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef __PACKAGEBASE_H__
#define __PACKAGEBASE_H__

#include <kurl.h>
#include <qstringlist.h>

class packageBase
{

public:
   packageBase();
   virtual ~packageBase();
   
// Generic accessors and mutators for child classes
   virtual QString generatePackage( ) = 0 ;
   //virtual bool loadFile(KURL theFile) = 0;
   //virtual bool saveFile(KURL theFile) = 0;

   virtual QString getAppName();
   virtual QString getAppVersion();
   virtual QString getAppRevision();
   virtual QString getAppGroup();
   virtual QString getAppPackager();
   virtual QString getAppURL();
   virtual QString getAppSummary();
   virtual QString getAppVendor();
   virtual QString getAppLicense();
   virtual QString getAppArch();
   virtual QString getAppDescription();
   virtual QString getAppChangelog();
   virtual QString getAppSource();
   virtual QStringList getAppFileList();
   
   virtual void setAppName(const QString&);
   virtual void setAppVersion(const QString&);
   virtual void setAppRevision(const QString&);
   virtual void setAppGroup(const QString&);
   virtual void setAppPackager(const QString&);
   virtual void setAppURL(const QString&);
   virtual void setAppSummary(const QString&);
   virtual void setAppVendor(const QString&);
   virtual void setAppLicense(const QString&);
   virtual void setAppArch(const QString&);
   virtual void setAppDescription(const QString&);
   virtual void setAppChangelog(const QString&);
   virtual void setAppFileList(const QStringList &list);
   
   private:
   	QString AppName;
	QString AppVersion;
	QString AppRevision;
	QString AppGroup;
	QString AppPackager;
	QString AppURL;
	QString AppSummary;
	QString AppVendor;
	QString AppLicense;
	QString AppArch;
	QString AppDescription;
	QString AppChangelog;
	QString AppSource;
	QStringList AppFileList;
};

#endif
