
/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/
#include <%{APPNAMELC}_factory.h>
#include <%{APPNAMELC}_part.h>
#include <%{APPNAMELC}_aboutdata.h>
#include <kinstance.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

extern "C"
{
    void* init_lib%{APPNAMELC}part()
    {
        KGlobal::locale()->insertCatalog("%{APPNAMELC}");
        return new %{APPNAME}Factory;
    }
};

KInstance* %{APPNAME}Factory::s_global = 0L;
KAboutData* %{APPNAME}Factory::s_aboutData = 0L;

%{APPNAME}Factory::%{APPNAME}Factory( QObject* parent, const char* name )
    : KoFactory( parent, name )
{
    global();
}

%{APPNAME}Factory::~%{APPNAME}Factory()
{
    delete s_aboutData;
    s_aboutData = 0L;
    delete s_global;
    s_global = 0L;
}

KParts::Part* %{APPNAME}Factory::createPartObject( QWidget *parentWidget, const char *widgetName, QObject* parent, const char* name, const char* classname, const QStringList & )
{
    // If classname is "KoDocument", our host is a koffice application
    // otherwise, the host wants us as a simple part, so switch to readonly and single view.
    bool bWantKoDocument = ( strcmp( classname, "KoDocument" ) == 0 );

    // parentWidget and widgetName are used by KoDocument for the "readonly+singleView" case.
    %{APPNAME}Part *part = new %{APPNAME}Part( parentWidget, widgetName, parent, name, !bWantKoDocument );

    if ( !bWantKoDocument )
      part->setReadWrite( false );

    return part;
}

KAboutData* %{APPNAME}Factory::aboutData()
{
    if ( !s_aboutData )
        // Change this, of course
        s_aboutData = new%{APPNAME}AboutData();
    return s_aboutData;
}

KInstance* %{APPNAME}Factory::global()
{
    if ( !s_global )
    {
        s_global = new KInstance( aboutData() );
        // Add any application-specific resource directories here

        // Tell the iconloader about share/apps/koffice/icons
        s_global->iconLoader()->addAppDir("koffice");
    }
    return s_global;
}

#include <%{APPNAMELC}_factory.moc>
