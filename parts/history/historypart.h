#ifndef __HISTORYPART_H__
#define __HISTORYPART_H__


#include <qguardedptr.h>
#include <qptrlist.h>
#include <qlistbox.h>


#include <kparts/part.h>
#include <kaction.h>
#include <kurl.h>


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

  void recentFileSelected(const QString &url);


private:

  void saveState(KParts::Part *part);
  void restoreState();

  void setupActions();

  void addHistoryEntry(HistoryEntry *entry);
  QPtrList<HistoryEntry> m_history;

  void addRecentEntry(KParts::Part *part);

  KToolBarPopupAction *m_backAction, *m_forwardAction;

  bool m_restoring;

  QPtrList<KURL> m_recentUrls;

  QGuardedPtr<QListBox> m_recentList;

};

#endif
