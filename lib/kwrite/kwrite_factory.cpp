
#include "kwrite_factory.h"

#include "kwdoc.h"
#include "highlight.h"
#include "kwview.h"

#include <klocale.h>
#include <kinstance.h>
#include <kaboutdata.h>

extern "C"
{
  void *init_libkwritepart()
  {
    return new KWriteFactory();
  }
}

KInstance *KWriteFactory::s_instance = 0;

KWriteFactory::KWriteFactory()
{
  s_instance = 0; // I don't trust anyone ;-)
}

KWriteFactory::~KWriteFactory()
{
  if ( s_instance )
  {
    delete s_instance->aboutData();
    delete s_instance;
  }
  s_instance = 0;
}

KParts::Part *KWriteFactory::createPart( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList & )
{
  bool bWantDocument = ( strcmp( classname, "KTextEditor::Document" ) == 0 );
  bool bWantBrowserView = ( strcmp( classname, "Browser/View" ) == 0 );

  KParts::ReadWritePart *part = new KWriteDoc( HlManager::self(), QString::null, !bWantDocument, bWantBrowserView, parentWidget, widgetName, parent, name );

  if ( bWantBrowserView || ( strcmp( classname, "KParts::ReadOnlyPart" ) == 0 ) )
    part->setReadWrite( false );

  emit objectCreated( part );
  return part;
}

KInstance *KWriteFactory::instance()
{
  if ( !s_instance )
    s_instance = new KInstance( aboutData() );
  return s_instance;
}

const KAboutData *KWriteFactory::aboutData()
{
    KAboutData *data = new KAboutData( "kwrite", I18N_NOOP( "KWrite" ),
                                       KWRITE_VERSION,
                                       I18N_NOOP( "Advanced Texteditor Component" ),
                                       KAboutData::License_GPL,
                                       "(c) 2000, Jochen Wilhelmy" );
    data->addAuthor( "Jochen Wilhemly", I18N_NOOP( "Author" ), "digisnap@cs.tu-berlin.de" );
    data->addAuthor( "Michael Koch",I18N_NOOP("Port to KParts"), "koch@kde.org");
    data->addAuthor( "Christian Gebauer", 0, "gebauer@bigfoot.com" );
    data->addAuthor( "Simon Hausmann", 0, "hausmann@kde.org" );
    data->addAuthor("Glen Parker",I18N_NOOP("Undo History, Kspell integration"), "glenebob@nwlink.com");
    data->addAuthor("Scott Manson",I18N_NOOP("XML Syntax highlighting support"), "sdmanson@alltel.net");
    return data;
}

#include "kwrite_factory.moc"
