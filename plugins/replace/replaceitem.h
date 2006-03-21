/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __REPLACEITEM_H__
#define __REPLACEITEM_H__


#include <kdeversion.h>
#include <kdebug.h>

#include "replaceview.h"

class ReplaceItem : public Q3CheckListItem
{
public:
    // the file item
    ReplaceItem( ReplaceView * parent, ReplaceItem * after, QString file ) :
            Q3CheckListItem( parent,
#if KDE_VERSION > 305
                            after,
#endif
                            file, Q3CheckListItem::CheckBox ),
            _file( file ), _string( file ), _line( 0 ), _isfile( true ),
            _lineclicked( false ), _clicked( true )
    {
        setOpen( true );
        setOn( true );
    }

    // the line item
    ReplaceItem( ReplaceItem * parent, ReplaceItem * after, QString file, QString string, int line ) :
            Q3CheckListItem( parent,
#if KDE_VERSION > 305
                            after,
#endif
                            QString::number( line + 1 ) + ": " + string, Q3CheckListItem::CheckBox ),
            _file( file ), _string( string ), _line( line ), _isfile( false ),
            _lineclicked( false ), _clicked( true )
    {
        setOn( true );
    }

    QString const & file() const
    {
        return _file;
    }

    int line() const
    {
        return _line;
    }

    QString const & string() const
    {
        return _string;
    }

    bool isFile() const
    {
        return _isfile;
    }

    bool justClicked()
    {
        bool t = _clicked;
        _clicked = true;
        return t;
    }

    bool lineClicked()
    {
        return _lineclicked;
    }

    ReplaceItem * parent() const
    {
        return static_cast<ReplaceItem*>( Q3ListViewItem::parent() );
    }

    ReplaceItem * firstChild() const
    {
        return static_cast<ReplaceItem*>( Q3ListViewItem::firstChild() );
    }

    ReplaceItem * nextSibling() const
    {
        return static_cast<ReplaceItem*>( Q3ListViewItem::nextSibling() );
    }

    void activate( int column, QPoint const & localPos );
    bool hasCheckedChildren() const;
    virtual void stateChange( bool state );

    static bool s_listview_done;

private:
#if KDE_VERSION > 305
    void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align );
#endif    
    void setChecked( bool checked );

    QString _file;
    QString _string;
    int _line;
    bool const _isfile;
    bool _lineclicked;
    bool _clicked;
};

#endif

