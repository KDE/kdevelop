#ifndef __KONSOLEVIEWPART_H__
#define __KONSOLEVIEWPART_H__


#include <qguardedptr.h>
#include "kdevpart.h"


class KonsoleViewWidget;


class KonsoleViewPart : public KDevPart
{
  Q_OBJECT

public:
   
  KonsoleViewPart(KDevApi *api, QObject *parent=0, const char *name=0);
  ~KonsoleViewPart();

  
private:
    
  QGuardedPtr<KonsoleViewWidget> m_widget;

};

#endif
