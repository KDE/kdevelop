
#include "projectworkspace.h"

#include <qdom.h>
#include <qptrlist.h>

#include <kparts/part.h>
#include <kurl.h>

#include "api.h"
#include "partcontroller.h"

void ProjectWorkspace::save()
{  
  // TODO: Save to a separate file for portability amoung team projects?
  QDomDocument* dom = API::getInstance()->projectDom();
  QDomElement element = dom->documentElement();
  QDomElement wsElement = element.namedItem( "workspace" ).toElement();
  
  if( wsElement.isNull() ) {
    wsElement = dom->createElement( "workspace" );
    element.appendChild( wsElement );
  }
  
  saveFileList( wsElement );
}

void ProjectWorkspace::restore()
{
  QDomDocument* dom = API::getInstance()->projectDom();
  QDomElement wsElement = dom->documentElement()
       .namedItem( "workspace" ).toElement();
  
  restoreFileList( wsElement );
}

void ProjectWorkspace::saveFileList( QDomElement& wsElement)
{
  QDomElement openfiles = wsElement
       .namedItem( "openfiles" ).toElement();
  if( openfiles.isNull() ) {
      openfiles = wsElement.ownerDocument().createElement( "openfiles" );
      wsElement.appendChild( openfiles );
  }
  
  // clear old entries
  while( !openfiles.firstChild().isNull() )
    openfiles.removeChild( openfiles.firstChild() );
    
  QPtrListIterator<KParts::Part> it( *PartController::getInstance()->parts() );
  for ( ; it.current(); ++it ) {
    KParts::ReadOnlyPart* ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
    if (!ro_part)
      continue;

    // TODO: Save relative path for project sharing?
    QString url = ro_part->url().url();

    QDomElement file = wsElement.ownerDocument().createElement( "file" );
    file.setAttribute( "url", url );
    openfiles.appendChild( file );
  }
}

void ProjectWorkspace::restoreFileList( const QDomElement& wsElement )
{
  QDomElement file = wsElement
       .namedItem( "openfiles" ).toElement()
       .firstChild().toElement();
  for( ; !file.isNull(); file = file.nextSibling().toElement() ) {
    if( file.tagName() != "file" )
      continue;
    KURL url( file.attribute( "url" ) );
    PartController::getInstance()->editDocument( url );
  }
}

ProjectWorkspace::ProjectWorkspace()
{
}


ProjectWorkspace::~ProjectWorkspace()
{
}
