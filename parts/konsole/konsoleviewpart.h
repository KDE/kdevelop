#ifndef __KONSOLEVIEWPART_H__
#define __KONSOLEVIEWPART_H__


#include <qguardedptr.h>
#include "kdevplugin.h"


class KonsoleViewWidget;


class KonsoleViewPart : public KDevPlugin
{
  Q_OBJECT

public:
   
  KonsoleViewPart(QObject *parent, const char *name, const QStringList &);
  ~KonsoleViewPart();

  
private:
    
  QGuardedPtr<KonsoleViewWidget> m_widget;

};

#endif
