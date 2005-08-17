#ifndef __OPENWITHPART_H__
#define __OPENWITHPART_H__

#include <kurl.h>

#include "kdevplugin.h"

class QMenu;
class Context;

class OpenWithPart : public KDevPlugin
{
  Q_OBJECT
public:   
  OpenWithPart(QObject *parent, const char *name, const QStringList &);
  virtual ~OpenWithPart();

private slots:
  void fillContextMenu(QMenu *popup, const Context *context);

  void openWithService();
  void openWithDialog();
  void openAsEncoding(int id);

private:
  void populateMenu(QMenu *popup);

private:
  KURL m_url;
};

#endif
