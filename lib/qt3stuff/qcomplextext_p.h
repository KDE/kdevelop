#ifndef QCOMPLEXTEXT_H
#define QCOMPLEXTEXT_H

#ifndef QT_H
#include <qstring.h>
#include <qpointarray.h>
#include <qfont.h>
#include <qpainter.h>
#include <qlist.h>
#include <qshared.h>
#endif // QT_H

class QFontPrivate;

namespace Qt3 {

// bidi helper classes. Internal to Qt
struct Q_EXPORT QBidiStatus {
    QBidiStatus() {
	eor = QChar::DirON;
	lastStrong = QChar::DirON;
	last = QChar:: DirON;
    }
    QChar::Direction eor 		: 5;
    QChar::Direction lastStrong 	: 5;
    QChar::Direction last		: 5;
};

struct Q_EXPORT QBidiContext : public QShared {
    // ### ref and deref parent?
    QBidiContext( uchar level, QChar::Direction embedding, QBidiContext *parent = 0, bool override = FALSE );
    ~QBidiContext();

    unsigned char level;
    bool override : 1;
    QChar::Direction dir : 5;

    QBidiContext *parent;
};

struct Q_EXPORT QBidiControl {
    QBidiControl() { context = 0; }
    QBidiControl( QBidiContext *c, QBidiStatus s)
    { context = c; if( context ) context->ref(); status = s; }
    ~QBidiControl() { if ( context && context->deref() ) delete context; }
    void setContext( QBidiContext *c ) { if ( context == c ) return; if ( context && context->deref() ) delete context; context = c; context->ref(); }
    QBidiContext *context;
    QBidiStatus status;
};

struct Q_EXPORT QTextRun {
    QTextRun(int _start, int _stop, QBidiContext *context, QChar::Direction dir) {
	start = _start;
	stop = _stop;
	if(dir == QChar::DirON) dir = context->dir;

	level = context->level;

	// add level of run (cases I1 & I2)
	if( level % 2 ) {
	    if(dir == QChar::DirL || dir == QChar::DirAN)
		level++;
	} else {
	    if( dir == QChar::DirR )
		level++;
	    else if( dir == QChar::DirAN )
		level += 2;
	}
	//printf("new run: level = %d\n", level);
    }

    int start;
    int stop;
    // explicit + implicit levels here
    uchar level;
};

class Q_EXPORT QComplexText {
public:
    enum Shape {
        XIsolated,
        XFinal,
        XInitial,
        XMedial
    };
    static Shape glyphVariant( const QString &str, int pos);
    static Shape glyphVariantLogical( const QString &str, int pos);

    //QT2HACK
    //static QString shapedString( const QString &str, int from = 0, int len = -1, QPainter::TextDirection dir = QPainter::Auto);
    static QChar shapedCharacter(const QString &str, int pos);

    // positions non spacing marks relative to the base character at position pos.
    static QPointArray positionMarks( QFontPrivate *f, const QString &str, int pos, QRect *boundingRect = 0 );

    static QList<QTextRun> *bidiReorderLine( QBidiControl *control, const QString &str, int start, int len );
    static QString bidiReorderString( const QString &str, QChar::Direction basicDir = QChar::DirON );
};

}; // namespace

#endif
