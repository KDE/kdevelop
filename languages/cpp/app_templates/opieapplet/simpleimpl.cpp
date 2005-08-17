#include <qlabel.h>
#include <qpainter.h>
#include <qmessagebox.h>

#include <qpe/applnk.h> // for AppLnk
#include <qpe/resource.h> // for Resource loading
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>
#include <QPaintEvent>

#include "%{APPNAMELC}.h"


%{APPNAME}::%{APPNAME}(QWidget *parent)
    : QWidget( parent,  "%{APPNAME} Applet" ) {
/*
 * we will load an Image, scale it for the right usage
 * remember your applet might be used by different
 * resolutions.
 * Then we will convert the image back to an Pixmap
 * and draw this Pimxap. We need to use Image because its
 * the only class that allows scaling.
 */

    QImage image = Resource::loadImage("%{APPNAMELC}/%{APPNAMELC}");
    /*
     * smooth scale to AppLnk smallIconSize for applest
     * smallIconSize gets adjusted to the resolution
     * so on some displays like SIMpad and a C-750 the smallIconSize
     * is greater than on a iPAQ h3870
     */
    image = image.smoothScale(AppLnk::smallIconSize(), AppLnk::smallIconSize() );

    /*
     * now we need to convert the Image to a Pixmap cause these
     * can be drawn more easily
     */
    m_pix = new QPixmap();
    m_pix->convertFromImage( image );

    /*
     * Now we will say that we don't want to be bigger than our
     * Pixmap
     */
    setFixedHeight(AppLnk::smallIconSize() );
    setFixedWidth( AppLnk::smallIconSize() );

}

%{APPNAME}::~%{APPNAME}() {
    delete m_pix;
}


/*
 * here you would normal show or do something
 * useful. If you want to show a widget at the top left
 * of your icon you need to map your rect().topLeft() to
 * global with mapToGlobal(). Then you might also need to
 * move the widgets so it is visible
 */
void %{APPNAME}::mousePressEvent(QMouseEvent* ) {
    QMessageBox::information(this, tr("No action taken"),
                             tr("<qt>This Plugin does not yet support anything usefule aye.</qt>"),
                             QMessageBox::Ok );

}

void %{APPNAME}::paintEvent( QPaintEvent* ) {
    QPainter p(this);

    /* simpy draw the pixmap from the start of this widget */
    p.drawPixmap(0, 0,  *m_pix );
}

/*
 * Here comes the implementation of the interface
 */
%{APPNAME}Impl::%{APPNAME}Impl() {
}
/* needed cause until it is only pure virtual */
%{APPNAME}Impl::~%{APPNAME}Impl() {
    /*
     * we will delete our applets as well
     * setAUtoDelete makes the QList free
     * the objects behind the pointers
     */
    m_applets.setAutoDelete( true );
    m_applets.clear();
}

/*
 * For the taskbar interface return a Widget
 */
QWidget* %{APPNAME}Impl::applet( QWidget* parent ) {
    /*
     * There are problems with ownership. So we add
     * our ownlist and clear this;
     */
    %{APPNAME}* ap = new %{APPNAME}( parent );
    m_applets.append( ap );

    return ap;
}

/*
 * A small hint where the Applet Should be displayed
 */
int %{APPNAME}Impl::position()const {
    return 1;
}


/*
 * Now the important QUnkownInterface method without
 * this one your applet won't load
 * @param uuid The uuid of the interface
 * @param iface The pointer to the interface ptr
 */
QRESULT %{APPNAME}Impl::queryInterface( const QUuid& uuid, QUnknownInterface** iface) {
    /* set the pointer to the interface to 0 */
    *iface = 0;

    /*
     * we check if we support the requested interface
     * and then assign to the pointer.
     * You may alos create another interface here so
     * *iface = this is only in this simple case true you
     * could also support more interfaces.
     * But this example below is the most common use.
     * Now the caller knows that the Interface Pointer
     * is valid and the interface supported
     */
    if ( uuid == IID_QUnknown )
        *iface = this;
    else if ( uuid == IID_TaskbarApplet )
        *iface = this;
    else
        return QS_FALSE;

    if ( *iface )
        (*iface)->addRef();

    return QS_OK;
}


/*
 * Finally we need to export the Interface.
 * CREATE_INSTANCE creates a interface and calls
 * queryInterface for the QUnknownInterface once
 * With out this function the applet can't be loaded.
 *
 * NOTE: If your applet does not load it's likely you've an
 * unresolved symbol. Change the .pro TEMPLATE = lib to TEMPLATE= app
 * and recompile. If the linker only complains about a missing
 * main method the problem is more complex. In most cases it'll say
 * you which symbols are missing and you can implement them.
 * The main(int argc, char* argv[] ) does not need to be
 * included in a library so it's ok that the linker complains
 */
Q_EXPORT_INTERFACE() {
    Q_CREATE_INSTANCE( %{APPNAME}Impl )
}

