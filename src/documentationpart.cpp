#include "partcontroller.h"

#include "documentationpart.h"

DocumentationPart::DocumentationPart()
  : KDevHTMLPart()
{
    setOptions(CanDuplicate | CanOpenInNewWindow);
}

void DocumentationPart::slotDuplicate( )
{
    PartController::getInstance()->showDocument(url(), true);
}

void DocumentationPart::slotOpenInNewWindow( const KURL & url )
{
    PartController::getInstance()->showDocument(url, true);
}

#include "documentationpart.moc"
