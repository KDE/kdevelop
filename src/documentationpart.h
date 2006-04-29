#ifndef __DOCUMENTATIONPART_H__
#define __DOCUMENTATIONPART_H__

#include <kdevhtmlpart.h>
#include "shellexport.h"

/**
HTML documentation part.

Implements shell-dependent "duplicate" and "open in new window" actions of KDevHTMLPart.
*/
class KDEVSHELL_EXPORT HTMLDocumentationPart : public KDevHTMLPart
{
  Q_OBJECT

public:

    HTMLDocumentationPart();

protected slots:

    virtual void slotDuplicate();
    virtual void slotOpenInNewWindow(const KUrl &url);
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
