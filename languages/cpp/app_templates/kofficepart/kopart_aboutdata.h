
/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/

#ifndef %{APPNAME}_ABOUTDATA
#define %{APPNAME}_ABOUTDATA

#include <kaboutdata.h>
#include <klocale.h>

static const char description[] = I18N_NOOP("%{APPNAME} KOffice Program");
static const char version[]     = "%{VERSION}";

KAboutData * new%{APPNAME}AboutData()
{
    KAboutData * aboutData=new KAboutData( "%{APPNAMELC}", I18N_NOOP("%{APPNAME}"),
                                           version, description, KAboutData::License_$LICENSE$,
                                           "(c) 2001, %{AUTHOR}");
    aboutData->addAuthor("%{AUTHOR}",0, "%{EMAIL}");
    return aboutData;
}

#endif
