#ifndef __DOCUMENTATIONPART_H__
#define __DOCUMENTATIONPART_H__

#include <kdevhtmlpart.h>

class HTMLDocumentationPart : public KDevHTMLPart
{
  Q_OBJECT

public:

    HTMLDocumentationPart();

protected slots:

    virtual void slotDuplicate();
    virtual void slotOpenInNewWindow(const KURL &url);
};

#endif
