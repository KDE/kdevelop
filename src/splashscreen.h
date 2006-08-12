
#ifndef _SPLASHSCREEN_H_
#define _SPLASHSCREEN_H_

#include <QSplashScreen>

class QPixmap;
class QPainter;

class SplashScreen : public QSplashScreen
{
    Q_OBJECT

public:
    SplashScreen( const QPixmap& pixmap = QPixmap(), Qt::WindowFlags f = 0 );
    virtual ~SplashScreen();

protected:
    void drawContents( QPainter * painter );

public slots:
    void animate();
    void showMessage( const QString &str, int flags = Qt::AlignLeft,
                      const QColor &color = Qt::black );

private:
    int m_state;
    int m_progress_bar_size;
    QString m_string;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
