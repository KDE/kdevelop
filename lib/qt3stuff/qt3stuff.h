#ifndef QT3STUFF_H
#define QT3STUFF_H

// This file is where the dirty hacks go :)
#include <qnamespace.h>

namespace Qt3 { // can't be Qt, a class already exists with that name :(

    enum NewFlags {
        AlignAuto = 0x0000,
        AlignJustify = 0x0080, // This is NOT the Qt 3 value, but we have to use another one !
        AlignHorizontal_Mask = Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter | AlignJustify,
        AlignVertical_Mask = Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter,
        WStaticContents = Qt::WNorthWestGravity,
        WType_Dialog  = 0x00000002,
        WStyle_Dialog = 0x00000002
    };

    enum TextFlags {
	SingleLine	= 0x0080,		// misc. flags
	DontClip	= 0x0100,
	ExpandTabs	= 0x0200,
	ShowPrefix	= 0x0400,
	WordBreak	= 0x0800,
	DontPrint	= 0x1000		// internal
    };

    enum WordWrap {
	NoWrap 		= 0x0000,
	WidgetWidth 	= 0x2000,
	FixedPixelWidth = 0x3000,
	FixedColumnWidth= 0x4000
    };

    enum WrapPolicy {
	AtWordBoundary 	= 0x0000,
	AtWhiteSpace 	= AtWordBoundary, // deprecated, don't use
	Anywhere 	= 0x8000
    };

    /*enum CursorShape {
	ArrowCursor,
	UpArrowCursor,
	CrossCursor,
	WaitCursor,
	IbeamCursor,
	SizeVerCursor,
	SizeHorCursor,
	SizeBDiagCursor,
	SizeFDiagCursor,
	SizeAllCursor,
	BlankCursor,
	SplitVCursor,
	SplitHCursor,
	PointingHandCursor,
	ForbiddenCursor,
	LastCursor	= ForbiddenCursor,
	BitmapCursor	= 24
    };*/

    enum Dock { Unmanaged, TornOff, Top, Bottom, Right, Left, Minimized };
    // compatibility
    //typedef Dock ToolBarDock;

    enum DateFormat {
	TextDate,      // default Qt
	ISODate,       // ISO 8601
	LocalDate      // locale dependant
    };

};

// Called _blah in Qt 2 and Q_blah in Qt 3
#if defined(_WS_X11)
#define Q_WS_X11
#endif

#define Q_ASSERT ASSERT

// Classes renamed in Qt 3
#define QPtrList QList
#define QPtrListIterator QListIterator
#define QPtrVector QVector
#define QMemArray QArray

#endif // QNAMESPACE_H

