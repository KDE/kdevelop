
#ifndef _SPLASHSCREEN_H_
#define _SPLASHSCREEN_H_

#include <qsplashscreen.h>
#include <qpainter.h>
#include <qlabel.h>

class QPixmap;

/**
Splash screen.
*/
class KDevSplashScreen : public QSplashScreen
{
Q_OBJECT

public:
	KDevSplashScreen(const QPixmap& pixmap, WFlags f = 0);
	virtual ~KDevSplashScreen();

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

