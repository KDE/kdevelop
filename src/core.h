#ifndef _CORE_H_
#define _CORE_H_

#include <qregexp.h>
#include <kxmlguiclient.h>

#include <kparts/partmanager.h>


#include "kdevcore.h"

namespace MainWindowUtils{

QString beautifyToolTip(const QString& text);
}

/**
Core implementation.
*/
class Core : public KDevCore
{
  Q_OBJECT

public:

  static Core *getInstance();
  
  /**
   * Setup shourtcut tips.
   *
   * @param client
   *   Pointer to KXMLGUIClient object, which contain an collection
   *   of actions (KActionCollection). If the parameter is null,
   *   function is applied to the all available KXMLGUIClient objects.
   */
  static void setupShourtcutTips(KXMLGUIClient * client = 0);

  ~Core();

  virtual void running(KDevPlugin *which, bool runs);
  virtual void fillContextMenu(QPopupMenu *popup, const Context *context);
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
