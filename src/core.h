#ifndef _CORE_H_
#define _CORE_H_

#include <qregexp.h>
#include <kparts/partmanager.h>
#include "kdevcore.h"
#include "shellexport.h"

namespace MainWindowUtils{

QString beautifyToolTip(const QString& text);
}

/**
Core implementation.
*/
class KDEVSHELL_EXPORT Core : public KDevCore
{
  Q_OBJECT

public:

  static Core *getInstance();

  ~Core();

  virtual void running(KDevPlugin *which, bool runs);
  virtual void fillContextMenu(QMenu *popup, const Context *context);
  virtual void openProject(const QString& projectFileName);

  void doEmitProjectOpened() { emit projectOpened(); }
  void doEmitProjectClosed() { emit projectClosed(); }
  void doEmitLanguageChanged() { emit languageChanged(); }
  void doEmitCoreInitialized() { emit coreInitialized(); }
  void doEmitProjectConfigWidget(KDialogBase *base) { emit projectConfigWidget(base); }
  void doEmitConfigWidget(KDialogBase *base) { emit configWidget(base); }
  void doEmitStopButtonPressed(KDevPlugin* which = 0) { emit stopButtonClicked( which ); }

  bool queryClose();

signals:

  void activeProcessChanged(KDevPlugin* which, bool runs);

protected:

  Core();

private:
  static Core *s_instance;

};


#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
