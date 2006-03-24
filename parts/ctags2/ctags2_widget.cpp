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

#include <qtimer.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <qfocusdata.h>

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>
#include <kapplication.h>

#include <kdevproject.h>
#include <kdevpartcontroller.h>

#include "ctags2_widget.h"
#include "tags.h"

class TagItem : public QListViewItem
{
public:
    TagItem(QListView * lv, QString const & tag, QString const & type, QString const & file, QString const & pattern );

	QString tag;
	QString type;
	QString file;
	QString pattern;
};

TagItem::TagItem( QListView * lv, QString const & tag, QString const & type, QString const & file, QString const & pattern )
	: QListViewItem( lv, tag, type, file ), tag(tag), type(type), file(file), pattern(pattern)
{}

CTags2Widget::CTags2Widget( CTags2Part * part, const char* name, WFlags fl)
: CTags2WidgetBase(0,name,fl), _part(part)
{
	_typeTimeout = new QTimer( this );
	connect( _typeTimeout, SIGNAL(timeout()), this, SLOT(line_edit_changed()) );

	connect( output_view, SIGNAL(executed(QListViewItem*)), this, SLOT(itemExecuted(QListViewItem*)) );
	connect( output_view, SIGNAL(returnPressed(QListViewItem*)), this, SLOT(itemExecuted(QListViewItem*)) );

	updateDBDateLabel();
}

CTags2Widget::~CTags2Widget()
{
}

void CTags2Widget::displayHits( Tags::TagList const & list )
{
	output_view->clear();
	showHitCount( list.count() );

	Tags::TagList::ConstIterator it = list.begin();
	while( it != list.end() )
	{
		new TagItem( output_view, (*it).tag, (*it).type, (*it).file, (*it).pattern );
		++it;
	}
}

void CTags2Widget::displayHitsAndClear( Tags::TagList const & list )
{
	input_edit->blockSignals( true );
	input_edit->clear();
	input_edit->blockSignals( false );

	displayHits( list );
}

void CTags2Widget::line_edit_changed( )
{
	displayHits( Tags::getPartialMatches( input_edit->text() ) );
}

void CTags2Widget::line_edit_changed_delayed( )
{
	showHitCount( calculateHitCount() );
	_typeTimeout->start( 500, true );
}

void CTags2Widget::showHitCount( int n )
{
	hitcount_label->setText( i18n("Hits: %1").arg( n ) );
}

int CTags2Widget::calculateHitCount( )
{
	return Tags::numberOfPartialMatches( input_edit->text() ) ;
}

void CTags2Widget::itemExecuted( QListViewItem * item )
{
	TagItem * tagItem = static_cast<TagItem*>( item );

	KURL url;
	QString fileWithTagInside;
	// assume relative path to project directory if path does not start with slash
	if (tagItem->file[0] != '/') {
		fileWithTagInside = _part->project()->projectDirectory() + "/" + tagItem->file;
	}
	else {
		fileWithTagInside = tagItem->file;
	}

	url.setPath(fileWithTagInside);

	_part->partController()->editDocument( url, _part->getFileLineFromPattern( url, tagItem->pattern ) );
}

void CTags2Widget::regeneratebutton_clicked()
{
	QApplication::setOverrideCursor(Qt::waitCursor);

	_part->createTagsFile();

	QApplication::restoreOverrideCursor();

	updateDBDateLabel();
}

void CTags2Widget::updateDBDateLabel( )
{
	QStringList tagFiles = Tags::getTagFiles();
	QFileInfo tagsdb(tagFiles[0]);
	if ( tagsdb.exists() )
	{
		datetime_label->setText( tagsdb.created().date().toString( Qt::ISODate ) );
	}
	else
	{
		datetime_label->setText( i18n("No CTags database found") );
	}
}

void CTags2Widget::focusInEvent( QFocusEvent* )
{
	updateDBDateLabel();
	input_edit->setFocus();
}

void CTags2Widget::goToNext( )
{
	QListViewItem * item = output_view->firstChild();
	while( item )
	{
		if ( item->isSelected() )
		{
			// found the current, take the next
			item->setSelected( false );
			if ( (item = item->nextSibling()) != NULL )
			{
				item->setSelected( true );
				output_view->repaint( true );
				itemExecuted( item );
				return;
			}
			else
			{
				break;
			}
		}
		item = item->nextSibling();
	}
	
	// use the first
	if ( (item = output_view->firstChild()) != NULL )
	{
		item->setSelected( true );
		itemExecuted( item );
	}
}

#include "ctags2_widget.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;

