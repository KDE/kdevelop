/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KDEVPART_FILELIST_H__
#define __KDEVPART_FILELIST_H__


#include <qpointer.h>
#include <kdevplugin.h>


class FileListWidget;

class FileListPart : public KDevPlugin
{
  Q_OBJECT

public:
   
	FileListPart(QObject *parent, const char *name, const QStringList &);
	~FileListPart();

private:
	QPointer<FileListWidget> m_widget;

};


#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
