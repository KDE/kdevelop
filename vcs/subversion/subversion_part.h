/* Copyright (C) 2003
	 Mickael Marchand <marchand@kde.org>

	 This program is free software; you can redistribute it and/or
	 modify it under the terms of the GNU General Public
	 License as published by the Free Software Foundation; either
	 version 2 of the License, or (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program; see the file COPYING.  If not, write to
	 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	 Boston, MA 02111-1307, USA.
	 */
#ifndef __KDEVPART_SUBVERSION_H__
#define __KDEVPART_SUBVERSION_H__


#include <qguardedptr.h>
#include <kdevplugin.h>
#include <kurl.h>
#include "kdevversioncontrol.h"
#include <kio/job.h>

class subversionCore;
class subversionOptionsWidget;
class subversionProjectWidget;

class subversionPart : public KDevVersionControl
{
  Q_OBJECT

public:
   
  subversionPart(QObject *parent, const char *name, const QStringList &);
  ~subversionPart();

	void setupActions();
	QWidget* newProjectWidget( QWidget* parent );
	void createNewProject( const QString& dirname );
	void fetchFromRepository();
	KDevVCSFileInfoProvider * fileInfoProvider() const;
	bool isValidDirectory( const QString& dirPath);
	void projectConfigWidget( KDialogBase *dlg );

private slots:
	void contextMenu( QPopupMenu *popup, const Context *context );
	void slotResult( KIO::Job * job );
  
private:
    
  QGuardedPtr<subversionCore> m_impl;
	KURL::List m_urls;

	KAction *actionCommit,
	*actionDiff,
	*actionAdd,
	*actionRemove,
	*actionUpdate,
	*actionAddToIgnoreList,
	*actionRemoveFromIgnoreList;

	subversionProjectWidget *m_projWidget;

};


#endif
