/***************************************************************************
                            -------------------
   begin                : 19.01.2003
   copyright            : (C) 2002 by Victor Röder
   email                : victor_roeder@gmx.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KIMPORTICONVIEW_H
#define KIMPORTICONVIEW_H

#include <qwidget.h>

#include "kfiledndiconview.h"

class KImportIconView : public KFileDnDIconView
{
	Q_OBJECT

public:
	KImportIconView( const QString& strIntro, QWidget *parent, const char *name );
	virtual ~KImportIconView();

	void somethingDropped ( bool dropped );

protected:
	void drawContents ( QPainter *p, int cx, int cy, int cw, int ch );

private:
	QString m_strIntro;
	bool m_bDropped;
};

#endif 
// kate: indent-mode csands; tab-width 4;

