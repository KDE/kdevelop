#ifndef _CORE_H_
#define _CORE_H_


#include <kparts/partmanager.h>


#include "kdevcore.h"

class KDevCoreIface;


class Core : public KDevCore
{
  Q_OBJECT

public:

  static void createInstance();
  static Core *getInstance();

  ~Core();

  virtual void embedWidget(QWidget *w, Role role, const QString &shortCaption);
  virtual void raiseWidget(QWidget *w);
  virtual void removeWidget( QWidget* w, Role role );

  virtual void gotoFile(const KURL &url);
  virtual void gotoDocumentationFile(const KURL& url, Embedding embed=Replace);
  virtual void gotoSourceFile(const KURL& url, int lineNum=0, Embedding embed=Replace);
  virtual void gotoExecutionPoint(const QString &fileName, int lineNum=0);
  virtual void saveAllFiles();
  virtual void revertAllFiles();
  virtual void setBreakpoint(const QString &fileName, int lineNum,
                             int id, bool enabled, bool pending);
  virtual void running(KDevPart *which, bool runs);
  virtual void fillContextMenu(QPopupMenu *popup, const Context *context);
  virtual void message(const QString &str);
  virtual KParts::PartManager *partManager() const;
  virtual void openProject(const QString& projectFileName);
  virtual KEditor::Editor *editor();
  virtual QStatusBar *statusBar() const;

  
  void doEmitProjectOpened() { emit projectOpened(); }
  void doEmitProjectClosed() { emit projectClosed(); }
  void doEmitCoreInitialized() { emit coreInitialized(); }
  void doEmitProjectConfigWidget(KDialogBase *base) { emit projectConfigWidget(base); }
  void doEmitConfigWidget(KDialogBase *base) { emit configWidget(base); };
  

protected:

  Core();

  
private:

  static Core *s_instance;
  
  KDevCoreIface *m_dcopIface;

};


#endif
