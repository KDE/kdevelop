/***************************************************************************
*   Copyright (C) 2003 by Jens Dagerbo                                    *
*   jens@krypton.supernet                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef __REPLACEITEM_H__
#define __REPLACEITEM_H__

#include <kdebug.h>
#include <klistview.h>

#include <qstringlist.h>
#include <qptrlist.h>


class OneShot
{
public:

    OneShot( bool d ) : _default( d ), _b( _default )
    {}
    void set()
    {
        _b = true;
    }

    void unset()
    {
        _b = false;
    }

    operator bool()
    {
        return test();
    }

private:

    bool test()
    {
        bool b = _b;
        _b = _default;
        return b;
    }
    bool const _default;
    bool _b;
};

class ReplaceItem : public QCheckListItem
{
public:
    // the file item
    ReplaceItem( QListView * parent, ReplaceItem * after, QString file ) :
            QCheckListItem( parent, after, file, QCheckListItem::CheckBox ),
            _file( file ), _string( file ), _line( 0 ), _isfile( true ),
            _block( false ), _clicked( true )
    {
        setOpen( true );
        setOn( true );
    }

    // the line item
    ReplaceItem( ReplaceItem * parent, ReplaceItem * after, QString file, QString string, int line ) :
            QCheckListItem( parent, after, QString::number( line + 1 ) + ": " + string, QCheckListItem::CheckBox ),
            _file( file ), _string( string ), _line( line ), _isfile( false ),
            _block( false ), _clicked( true )
    {
        setOn( true );
    }

    QString file()
    {
        return _file;
    }

    int line()
    {
        return _line;
    }

    QString string()
    {
        return _string;
    }

    bool isFile()
    {
        return _isfile;
    }

    bool blockClick()
    {
        return _block;
    }

    ReplaceItem * parent()
    {
        return static_cast<ReplaceItem*>( QListViewItem::parent() );
    }

    ReplaceItem * firstChild()
    {
        return static_cast<ReplaceItem*>( QListViewItem::firstChild() );
    }

    ReplaceItem * nextSibling()
    {
        return static_cast<ReplaceItem*>( QListViewItem::nextSibling() );
    }

    // QPtrList<ReplaceItem> getChildren( bool only_checked );
    bool hasCheckedChildren();
    virtual void stateChange( bool state );

    static bool s_listview_done;

private:

    void setChecked( bool checked );
    // virtual void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align );

    QString _file;
    QString _string;
    int _line;
    bool const _isfile;
    OneShot _block;
    OneShot _clicked;
};

#endif

