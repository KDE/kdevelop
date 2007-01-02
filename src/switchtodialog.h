/***************************************************************************
 *   Copyright (C) 2007 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SWITCHTODIALOG_H
#define SWITCHTODIALOG_H

#include <qmap.h>

#include <kurl.h>

#include "switchtodialogbase.h"

class SwitchToDialog : public SwitchToDialogBase
{
Q_OBJECT
	
public:
	SwitchToDialog( const KURL::List & urls, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~SwitchToDialog();
	
	KURL selectedUrl();

protected:
	void resizeEvent(QResizeEvent *);

protected slots:
	void slotHighlightChanged( const QString & );
	void slotReturnPressed( const QString & );

private:
	void setSqueezedLabel( const QString & );
	QMap<QString, KURL> m_urlMap;
 	KURL m_selectedUrl;
	QString m_unsqueezedString;
};

#endif

