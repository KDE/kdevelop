
#include "plugin_%{APPNAMELC}.h"

#include <khtml_part.h>
#include <kaction.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>

Plugin%{APPNAME}::Plugin%{APPNAME}( QObject* parent, const char* name )
    : Plugin( parent, name )
{
    // Instantiate all of your actions here.  These will appear in
    // Konqueror's menu and toolbars.
    (void) new KAction( i18n("&Plugin Action"), "%{APPNAMELC}", 0,
                        this, SLOT(slotAction()),
                        actionCollection(), "plugin_action" );
}

Plugin%{APPNAME}::~Plugin%{APPNAME}()
{
}

void Plugin%{APPNAME}::slotAction()
{
    // This plugin assumes KHTMLPart.  If your plugin can handle more
    // than this or a different Part than this, simply delete or
    // change the following block.
    if ( !parent()->inherits("KHTMLPart") )
    {
        QString title( i18n( "Cannot Translate Source" ) );
        QString text( i18n( "You cannot translate anything except web pages "
                            "with this plugin." ) );

        KMessageBox::sorry( 0, text, title );
        return;
    }

    // Get a handle on our parent so we may get the necessary data for
    // processing
    KHTMLPart *part = dynamic_cast<KHTMLPart *>(parent());

    // This plugin only uses the URL.  You may use whatever data you
    // need.
    KURL url( part->url() );

    // This is a standard check to make sure we are dealing with a
    // valid URL
    if ( !url.isValid() )
    {
        QString title( i18n( "Malformed URL" ) );
        QString text( i18n( "The URL you entered is not valid, please "
                            "correct it and try again" ) );

        KMessageBox::sorry( 0, text, title );
        return;
    }

// The following block is very plugin specific.  In this example, we
// translate the current page with AltaVista's BabelFish.  You will
// definitely want to change this.
// BEGIN
    KURL work( "http://babel.altavista.com/translate.dyn" );

    QString query( "urltext=" );
    query += KURL::encode_string( url.url() );
    work.setQuery( query );
// END

    // Finally, execute the request
    part->openURL( work );
}

%{APPNAME}Factory::%{APPNAME}Factory()
  : KParts::Factory()
{
}

%{APPNAME}Factory::~%{APPNAME}Factory()
{
  delete s_instance;
  s_instance = 0;
}

QObject* %{APPNAME}Factory::createObject( QObject* parent, const char* name, const char*, const QStringList & )
{
  return new Plugin%{APPNAME}( parent, name );
}

KInstance *%{APPNAME}Factory::instance()
{
  if ( !s_instance )
    s_instance = new KInstance( "%{APPNAMELC}" );
  return s_instance;
}

extern "C"
{
  void* init_lib%{APPNAMELC}plugin()
  {
    KGlobal::locale()->insertCatalogue("%{APPNAMELC}");
    return new %{APPNAME}Factory;
  }
}

KInstance* %{APPNAME}Factory::s_instance = 0L;

#include "plugin_%{APPNAMELC}.moc"
