#ifndef __OPENWITHPART_H__
#define __OPENWITHPART_H__

#include <kurl.h>

#include "kdevplugin.h"
//Added by qt3to4:
#include <Q3PopupMenu>

class Q3PopupMenu;
class Context;


class OpenWithPart : public KDevPlugin
{
  Q_OBJECT

public:
   
  OpenWithPart(QObject *parent, const char *name, const QStringList &);
  ~OpenWithPart();

 
private slots:

  void fillContextMenu(Q3PopupMenu *popup, const Context *context);

  void openWithService();
  void openWithDialog();
  void openAsEncoding(int id);

private:

  KURL m_url;
};

#endif
