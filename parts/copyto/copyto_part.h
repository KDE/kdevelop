/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __KDEVPART_COPYTO_H__
#define __KDEVPART_COPYTO_H__

#include <kdevplugin.h>
#include <kurl.h>

class CopyToPart : public KDevPlugin
{
	Q_OBJECT

public:

	CopyToPart(QObject *parent, const char *name, const QStringList &);
	~CopyToPart();


private slots:
	void contextMenu( QPopupMenu* popup, const Context* context );
	void doCopy();
	
private:
	QString relativeProjectPath( QString path );
	KURL::List _fileList;
	
};


#endif
