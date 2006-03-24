/***************************************************************************
 *   Copyright (C) 2005 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CTAGS2_SETTINGSWIDGET_H
#define CTAGS2_SETTINGSWIDGET_H

#include <qlistview.h>

#include "ctags2_settingswidgetbase.h"

class CTags2Part;

class TagsItem : public QCheckListItem
{
public:
	TagsItem( QListView * parent, QString name, QString tagsfilePath, bool active )
		: QCheckListItem( parent, name, QCheckListItem::CheckBox ), m_name( name ), m_tagsfilePath( tagsfilePath )
	{
		setOn( active );
	}
	QString name() { return m_name; }
	QString tagsfilePath() { return m_tagsfilePath; }

private:
	QString m_name;
	QString m_tagsfilePath;
};



class CTags2SettingsWidget : public CTags2SettingsWidgetBase
{
	Q_OBJECT

public:
	CTags2SettingsWidget( CTags2Part * part, QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
	~CTags2SettingsWidget();

signals:
	void newTagsfileName( const QString & name );

public slots:
	void slotAccept();
	void createNewTagSlot();
	void addNewTagFile();
	void removeTagFile();

protected:

protected slots:

private:
	void loadSettings();
	void storeSettings();

	CTags2Part * m_part;
};

#endif

