
#include "splashscreen.h"

#include <config.h>

#include <QFont>
#include <QTimer>
#include <QPixmap>
#include <QPainter>

#include <klocale.h>
#include <kglobalsettings.h>

SplashScreen::SplashScreen( const QPixmap& pixmap, Qt::WindowFlags f )
        : QSplashScreen( pixmap, f )
{
    QTimer * timer = new QTimer( this );
    QObject::connect( timer, SIGNAL( timeout() ), this, SLOT( animate() ) );
    timer->start( 150 );

    m_state = 0;
    m_progress_bar_size = 3;
}

SplashScreen::~SplashScreen()
{}

void SplashScreen::animate()
{
    m_state = ( ( m_state + 1 ) % ( 2 * m_progress_bar_size - 1 ) );
    repaint();
}

void SplashScreen::showMessage( const QString &str, int flags, const QColor &color )
{
    QSplashScreen::showMessage( str, flags, color );
    animate();
    m_string = str;
}

void SplashScreen::drawContents( QPainter* painter )
{
    int position;
    QColor base_color ( 201, 229, 165 ); // Base green color

    // Draw background circles
    painter->setPen( Qt::NoPen );
    painter->setBrush( QColor( 215, 234, 181 ) );
    painter->drawEllipse( 51, 7, 9, 9 );
    painter->drawEllipse( 62, 7, 9, 9 );
    painter->drawEllipse( 73, 7, 9, 9 );

    // Draw animated circles, increments are chosen
    // to get close to background's color
    // (didn't work well with QColor::light function)
    for ( int i = 0; i < m_progress_bar_size; i++ )
    {
        position = ( m_state + i ) % ( 2 * m_progress_bar_size - 1 );
        painter->setBrush( QColor( base_color.red() - 18 * i,
                                   base_color.green() - 10 * i,
                                   base_color.blue() - 28 * i ) );

        if ( position < 3 ) painter->drawEllipse( 51 + position * 11, 7, 9, 9 );
    }

    painter->setPen( QColor( 74, 112, 18 ) );
    QFont fnt( KGlobalSettings::generalFont() );
    fnt.setPointSize( 8 );
    painter->setFont( fnt );

    // Draw version number
    QRect r = rect();
    r.setRect( r.x() + 5, r.y() + 5, r.width() - 10, r.height() - 10 );
    painter->drawText( r, Qt::AlignRight, i18n( "Version %1", QString( VERSION ) ) );

    // Draw message at given position, limited to 43 chars
    // If message is too long, string is truncated
    if ( m_string.length() > 40 )
    {
        m_string.truncate( 39 );
        m_string += "...";
    }
    painter->drawText ( 90, 16, m_string );
}

#include "splashscreen.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
