#ifndef __WINDOWMENUPART_H__
#define __WINDOWMENUPART_H__


#include "kdevplugin.h"


class WindowMenuPart : public KDevPlugin
{
  Q_OBJECT

public:
   
  WindowMenuPart(QObject *parent, const char *name, const QStringList &);
  ~WindowMenuPart();

 
private slots:

  void updateBufferMenu();
  void bufferSelected();

};

#endif
