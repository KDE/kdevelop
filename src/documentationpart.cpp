#include "documentcontroller.h"

#include "documentationpart.h"

HTMLDocumentationPart::HTMLDocumentationPart()
  : KDevHTMLPart()
{
    setOptions(CanDuplicate | CanOpenInNewWindow);
}

void HTMLDocumentationPart::slotDuplicate( )
{
    DocumentController::getInstance()->showDocument(url(), true);
}

void HTMLDocumentationPart::slotOpenInNewWindow( const KUrl & url )
{
    DocumentController::getInstance()->showDocument(url, true);
}

#include "documentationpart.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
