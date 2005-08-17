#include "%{APPNAMELC}.h"

#include <qpe/applnk.h>
#include <qpe/resource.h>

/* QT */
#include <qicon.h>
#include <q3popupmenu.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <QPixmap>


%{APPNAME}::%{APPNAME}()
             :QObject( 0, "%{APPNAME}" )
{
}

%{APPNAME}::~%{APPNAME} ( )
{}

int %{APPNAME}::position() const
{
    return 3;
}

QString %{APPNAME}::name() const
{
    return tr( "MenuApplet Example Name" );
}

QString %{APPNAME}::text() const
{
    return tr( "Click the white rabbit" );
}


QIcon %{APPNAME}::icon() const
{
    QPixmap pix;
    QImage img = Resource::loadImage( "Tux" );
    if ( !img.isNull() )
        pix.convertFromImage( img.smoothScale( AppLnk::smallIconSize(), AppLnk::smallIconSize() ) );
    return pix;
}

Q3PopupMenu* %{APPNAME}::popup(QWidget*) const
{
    /* no subdir */
    return 0;
}

void %{APPNAME}::activated()
{
    QMessageBox::information(0,tr("No white rabbit found"),
			     tr("<qt>No white rabbit was seen near Opie."
			        "Only the beautiful OpieZilla is available"
				"for your pleassure</qt>"));
}


QRESULT %{APPNAME}::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
        *iface = this;
    else if ( uuid == IID_MenuApplet )
        *iface = this;
    else
	return QS_FALSE;

    if ( *iface )
        (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( %{APPNAME} )
}
