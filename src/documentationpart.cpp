#include "partcontroller.h"


#include "documentationpart.h"


DocumentationPart::DocumentationPart()
  : KHTMLPart(0L, 0L, 0L, "DocumentationPart")
{
  connect(browserExtension(), SIGNAL(openURLRequestDelayed(const KURL &,const KParts::URLArgs &)),
          this, SLOT(openURLRequest(const KURL &)) );
}


void DocumentationPart::setContext(const QString &context)
{
  m_context = context;
}


QString DocumentationPart::context() const
{
  return m_context;
}


void DocumentationPart::openURLRequest(const KURL &url)
{
  PartController::getInstance()->showDocument(url, context());
}


#include "documentationpart.moc"
