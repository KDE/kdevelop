
#include "projectworkspace.h"

#include <qdom.h>
#include <qptrlist.h>

#include <kparts/part.h>
#include <kurl.h>

#include "api.h"
#include "partcontroller.h"
#include "documentationpart.h"


void ProjectWorkspace::save()
{  
  // Save to a separate file for portability amoung team projects?
  // No, these things are exactly what the project file is for

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

    DocumentationPart* docpart = dynamic_cast<DocumentationPart*>(ro_part);

    // TODO: Save relative path for project sharing?
    QString url = ro_part->url().url();

    QDomElement file = wsElement.ownerDocument().createElement( "file" );
    file.setAttribute( "url", url );
    if (docpart)
        file.setAttribute( "context", docpart->context() );
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
    if (file.hasAttribute( "context" )) {
      QString context( file.attribute( "context" ) );
      PartController::getInstance()->showDocument( url, context );
    } else {
      PartController::getInstance()->editDocument( url );
    }
  }
}

ProjectWorkspace::ProjectWorkspace()
{
}


ProjectWorkspace::~ProjectWorkspace()
{
}
