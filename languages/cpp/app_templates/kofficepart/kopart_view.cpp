
/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/
#include "%{APPNAMELC}_view.h"
#include "%{APPNAMELC}_factory.h"
#include "%{APPNAMELC}_part.h"

#include <qpainter.h>
#include <qicon.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>
#include <kdebug.h>

%{APPNAME}View::%{APPNAME}View( %{APPNAME}Part* part, QWidget* parent, const char* name )
    : KoView( part, parent, name )
{
    setInstance( %{APPNAME}Factory::global() );
    setXMLFile( "%{APPNAMELC}.rc" );
    KStdAction::cut(this, SLOT( cut() ), actionCollection(), "cut" );
    // Note: Prefer KStdAction::* to any custom action if possible.
    //m_cut = new KAction( i18n("&Cut"), "editcut", 0, this, SLOT( cut() ),
    //                   actionCollection(), "cut");
}

void %{APPNAME}View::paintEvent( QPaintEvent* ev )
{
    QPainter painter;
    painter.begin( this );

    /// @todo Scaling

    // Let the document do the drawing
    koDocument()->paintEverything( painter, ev->rect(), FALSE, this );

    painter.end();
}

void %{APPNAME}View::updateReadWrite( bool /*readwrite*/ )
{
#ifdef __GNUC__
#warning TODO
#endif
}

void %{APPNAME}View::cut()
{
    kdDebug(31000) << "%{APPNAME}View::cut(): CUT called" << endl;
}

#include "%{APPNAMELC}_view.moc"
