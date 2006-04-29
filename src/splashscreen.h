
#ifndef _SPLASHSCREEN_H_
#define _SPLASHSCREEN_H_

#include <qsplashscreen.h>
#include "shellexport.h"

class QPainter;
class QPixmap;

/**
Splash screen.
*/
class KDEVSHELL_EXPORT SplashScreen : public QSplashScreen
{
Q_OBJECT

public:
    SplashScreen(const QPixmap& pixmap, Qt::WFlags f = 0);
    virtual ~SplashScreen();

protected:
    void drawContents (QPainter * painter);

public slots:
    void animate();
    void showMessage( const QString &str, int flags = Qt::AlignLeft,
                      const QColor &color = Qt::black );

private:
    int state;
    int progress_bar_size;
    QString m_string;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
