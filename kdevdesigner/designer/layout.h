/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef LAYOUT_H
#define LAYOUT_H

#include <qwidget.h>
#include <qmap.h>
#include <qguardedptr.h>
#include <qobject.h>
#include <qlayout.h>
#include <qmap.h>
#include <qwidgetlist.h>

class FormWindow;
class QPaintEvent;

class Layout : public QObject
{
    Q_OBJECT

public:
    Layout( const QWidgetList &wl, QWidget *p, FormWindow *fw, QWidget *lb, bool doSetup = TRUE, bool splitter = FALSE );
    virtual ~Layout() {}

    virtual void doLayout() = 0;
    virtual void undoLayout();
    virtual void breakLayout();
    virtual bool prepareLayout( bool &needMove, bool &needReparent );
    virtual void finishLayout( bool needMove, QLayout *layout );

protected:
    QWidgetList widgets;
    QWidget *parent;
    QPoint startPoint;
    QMap<QGuardedPtr<QWidget>, QRect> geometries;
    QWidget *layoutBase;
    FormWindow *formWindow;
    QRect oldGeometry;
    bool isBreak;
    bool useSplitter;

protected:
    virtual void setup();

protected slots:
    void widgetDestroyed();

};

class HorizontalLayout : public Layout
{
public:
    HorizontalLayout( const QWidgetList &wl, QWidget *p, FormWindow *fw, QWidget *lb, bool doSetup = TRUE, bool splitter = FALSE );

    void doLayout();

protected:
    void setup();

};

class VerticalLayout : public Layout
{
public:
    VerticalLayout( const QWidgetList &wl, QWidget *p, FormWindow *fw, QWidget *lb, bool doSetup = TRUE, bool splitter = FALSE );

    void doLayout();

protected:
    void setup();

};

class Grid;

class GridLayout : public Layout
{
public:
    GridLayout( const QWidgetList &wl, QWidget *p, FormWindow *fw, QWidget *lb, const QSize &res, bool doSetup = TRUE );
    ~GridLayout();

    void doLayout();

protected:
    void setup();

protected:
    void buildGrid();
    QSize resolution;
    Grid* grid;

};

class Spacer : public QWidget
{
    Q_OBJECT

    Q_OVERRIDE( QCString name )
    Q_PROPERTY( Orientation orientation READ orientation WRITE setOrientation )
    Q_ENUMS( SizeType )
    Q_PROPERTY( SizeType sizeType READ sizeType WRITE setSizeType )
    Q_PROPERTY( QSize sizeHint READ sizeHint WRITE setSizeHint DESIGNABLE true STORED true )
    Q_OVERRIDE( QRect geometry DESIGNABLE false )

private:
    enum { HSize = 6, HMask = 0x3f, VMask = HMask << HSize,
	   MayGrow = 1, ExpMask = 2, MayShrink = 4 };

public:
    enum SizeType { Fixed = 0,
		    Minimum = MayGrow,
		    Maximum = MayShrink,
		    Preferred = MayGrow|MayShrink ,
		    MinimumExpanding = Minimum|ExpMask,
		    Expanding = MinimumExpanding|MayShrink };

    Spacer( QWidget *parent, const char *name );

    QSize minimumSize() const;
    QSize sizeHint() const;
    void setSizeType( SizeType t );
    SizeType sizeType() const;
    int alignment() const;
    Orientation orientation() const;
    void setOrientation( Orientation o );
    void setInteraciveMode( bool b ) { interactive = b; };
    void setSizeHint( const QSize &s );

protected:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent* e );
    void updateMask();
    Qt::Orientation orient;
    bool interactive;
    QSize sh;
};

class QDesignerGridLayout : public QGridLayout
{
   Q_OBJECT
public:
    QDesignerGridLayout( QWidget *parent ) : QGridLayout( parent ){};
    QDesignerGridLayout( QLayout *parentLayout ) : QGridLayout( parentLayout ){};

    void addWidget( QWidget *, int row, int col, int align = 0 );
    void addMultiCellWidget( QWidget *, int fromRow, int toRow,
			       int fromCol, int toCol, int align = 0 );

    struct Item
    {
	Item(): row(0), column(0),rowspan(1),colspan(1){}
	Item( int r, int c, int rs, int cs): row(r), column(c), rowspan(rs), colspan(cs){}
	int row;
	int column;
	int rowspan;
	int colspan;
	Q_DUMMY_COMPARISON_OPERATOR( Item )
    };

    QMap<QWidget*, Item> items;
};


#endif
