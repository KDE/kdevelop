#ifndef __HISTORYPART_H__
#define __HISTORYPART_H__


#include <qptrlist.h>


#include <kparts/part.h>
#include <kaction.h>


#include "kdevplugin.h"


class HistoryEntry;

class HistoryPart : public KDevPlugin
{
  Q_OBJECT

public:
   
  HistoryPart(QObject *parent, const char *name, const QStringList &);
  ~HistoryPart();

 
private slots:

  void partAdded(KParts::Part *part);
  void partRemoved(KParts::Part *part);
  void activePartChanged(KParts::Part *part);

  void backActivated();
  void forwardActivated();

  void backAboutToShow();
  void forwardAboutToShow();

  void backPopupActivated(int id);
  void forwardPopupActivated(int id);

  void updateActions();


private:

  void saveState(KParts::Part *part);
  void restoreState();

  void setupActions();

  void addHistoryEntry(HistoryEntry *entry);
  QPtrList<HistoryEntry> m_history;

  KToolBarPopupAction *m_backAction, *m_forwardAction;

  bool m_restoring;

};

#endif
