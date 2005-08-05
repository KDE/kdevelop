
#ifndef _SPLASHSCREEN_H_
#define _SPLASHSCREEN_H_

#include <qsplashscreen.h>
#include <qpainter.h>
#include <qlabel.h>

class QPixmap;

/**
Splash screen.
*/
class SplashScreen : public QSplashScreen
{
Q_OBJECT

public:
	SplashScreen(const QPixmap& pixmap, WFlags f = 0);
	virtual ~SplashScreen();

protected:
	void drawContents (QPainter * painter);

public slots:
	void animate();
	void message( const QString &str, int flags = AlignLeft,
		const QColor &color = black );

private:
	int state;
	int progress_bar_size;
	QString m_string;
		
};

#endif

