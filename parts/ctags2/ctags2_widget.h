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

#ifndef CTAGS2_WIDGET_H
#define CTAGS2_WIDGET_H

#include "ctags2_widgetbase.h"
#include "ctags2_part.h"
#include "tags.h"

class QTimer;
class QListViewItem;

class CTags2Widget : public CTags2WidgetBase
{
	Q_OBJECT

public:
	CTags2Widget( CTags2Part * part, const char* name = 0, WFlags fl = 0 );
	~CTags2Widget();

	void displayHits( Tags::TagList const & );
	void displayHitsAndClear( Tags::TagList const & );
  
	//@todo - just push the search tag into the widget lookup widget, the rest is automatic!
	
	void updateDBDateLabel();

protected:
	void focusInEvent( QFocusEvent *e );
		
private slots:
    virtual void line_edit_changed();
    virtual void line_edit_changed_delayed();
	virtual void regeneratebutton_clicked();	  
	void itemExecuted( QListViewItem * );

private: 
	void showHitCount( int );
	int calculateHitCount();
	
	CTags2Part * _part;
	QTimer * _typeTimeout;

};

#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
