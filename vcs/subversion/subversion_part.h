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
#include <qpopupmenu.h>
#include <kdialogbase.h>
#include "kdevversioncontrol.h"

class subversionCore;
class subversionOptionsWidget;
class subversionProjectWidget;
class Context;

class subversionPart : public KDevVersionControl
{
	Q_OBJECT

public:
		subversionPart(QObject *parent, const char *name, const QStringList &);
		virtual ~subversionPart();

		void setupActions();
		QWidget* newProjectWidget( QWidget* parent );
		void createNewProject( const QString& dirname );
		bool fetchFromRepository();
		KDevVCSFileInfoProvider * fileInfoProvider() const;
		bool urlFocusedDocument( KURL &url );
		void restorePartialProjectSession(const QDomElement* );
		void savePartialProjectSession(QDomElement* );
		void setBaseURL(const KURL& url ) { base = url; }
		KURL baseURL() { return base; }
		virtual bool isValidDirectory( const QString &dirPath ) const;

signals:
//		void finishedFetching( QString destinationDir );

private slots:
		void contextMenu( QPopupMenu *popup, const Context *context );
		void slotActionUpdate();
		void slotActionRevert();
		void slotActionCommit();
		void slotActionAdd();
		void slotActionDel();
		void slotActionDiffHead();
		void slotActionDiffLocal();
		void slotActionResolve();
		void slotUpdate();
		void slotRevert();
		void slotCommit();
		void slotAdd();
		void slotDel();
		void slotDiffLocal();
		void slotDiffHead();
		void slotResolve();
		void slotProjectClosed();
		void slotProjectOpened();

private:
		QGuardedPtr<subversionCore> m_impl;
		KURL::List m_urls;

		KAction *actionCommit,
		*actionDiffHead,
		*actionDiffLocal,
		*actionAdd,
		*actionRemove,
		*actionUpdate,
		//*actionAddToIgnoreList,
		//*actionRemoveFromIgnoreList,
		*actionRevert,
		*actionResolve;

		QGuardedPtr<subversionProjectWidget> m_projWidget;
		KURL base;

public:

};


#endif
