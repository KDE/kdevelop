
/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/

#include "%{APPNAMELC}_part.h"
#include "%{APPNAMELC}_view.h"

#include <qpainter.h>

%{APPNAME}Part::%{APPNAME}Part( QWidget *parentWidget, const char *widgetName, QObject* parent, const char* name, bool singleViewMode )
    : KoDocument( parentWidget, widgetName, parent, name, singleViewMode )
{
}

bool %{APPNAME}Part::initDoc()
{
    // If nothing is loaded, do initialize here
    return TRUE;
}

KoView* %{APPNAME}Part::createViewInstance( QWidget* parent, const char* name )
{
    return new %{APPNAME}View( this, parent, name );
}

bool %{APPNAME}Part::loadXML( QIODevice *, const QDomDocument & )
{
    /// @todo load the document from the QDomDocument
    return true;
}

QDomDocument %{APPNAME}Part::saveXML()
{
    /// @todo save the document into a QDomDocument
    return QDomDocument();
}


void %{APPNAME}Part::paintContent( QPainter& painter, const QRect& rect, bool /*transparent*/,
                                double /*zoomX*/, double /*zoomY*/ )
{
    // ####### handle transparency

    // Need to draw only the document rectangle described in the parameter rect.
    int left = rect.left() / 20;
    int right = rect.right() / 20 + 1;
    int top = rect.top() / 20;
    int bottom = rect.bottom() / 20 + 1;

    for( int x = left; x < right; ++x )
        painter.drawLine( x * 20, top * 20, x * 20, bottom * 20 );
    for( int y = left; y < right; ++y )
        painter.drawLine( left * 20, y * 20, right * 20, y * 20 );
}

#include "%{APPNAMELC}_part.moc"
