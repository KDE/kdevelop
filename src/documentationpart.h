#ifndef __DOCUMENTATIONPART_H__
#define __DOCUMENTATIONPART_H__

#include <kdevhtmlpart.h>

class DocumentationPart : public KDevHTMLPart
{
  Q_OBJECT

public:

    DocumentationPart();

protected slots:

    virtual void slotDuplicate();
    virtual void slotOpenInNewWindow(const KURL &url);
};

#endif
