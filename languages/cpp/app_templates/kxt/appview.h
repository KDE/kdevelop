/***************************************************************************
 *   Copyright (C) 2004 by ita                                             *
 *   ita@localhost                                                         *
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

#ifndef _$APPNAMEUC$VIEW_H_
#define _$APPNAMEUC$VIEW_H_

#include <qwidget.h>

#include "$APPNAME$view_base.h"

class QPainter;
class KURL;

/**
 * This is the main view class for $APPNAME$.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * @short Main view
 * @author $AUTHOR$ <$EMAIL$>
 * @version $VERSION$
 */
class $APPNAME$View : public $APPNAME$view_base
{
    Q_OBJECT
public:
	/**
	 * Default constructor
	 */
    $APPNAME$View(QWidget *parent);

	/**
	 * Destructor
	 */
    virtual ~$APPNAME$View();

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString& text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString& text);

private slots:
    void switchColors();
    void settingsChanged();
};

#endif // _$APPNAMEUC$VIEW_H_
