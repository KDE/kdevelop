/*
 *  Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>
 */

#ifndef __KDEVPART_VALGRIND_H__
#define __KDEVPART_VALGRIND_H__

#include <qpointer.h>
#include <QString>
#include <qstringlist.h>
#include <kdevplugin.h>

class QTreeView;
class ValgrindModel;
class ValgrindControl;

class ValgrindPart : public KDevPlugin
{
  Q_OBJECT

public:
  ValgrindPart( QObject *parent, const QStringList & );
  ~ValgrindPart();

  void runValgrind( const QString& exec, const QString& parameters, const QString& valExec, const QString& valParameters );

  void savePartialProjectSession( QDomElement* el );
  void restorePartialProjectSession( const QDomElement* el );

  QWidget* pluginView() const;
  Qt::DockWidgetArea dockWidgetAreaHint() const;

private slots:
  void slotExecValgrind();
  void slotExecCalltree();
  void slotKillValgrind();
  void slotStopButtonClicked( KDevPlugin* which );
  void loadOutput();
  void projectOpened();

private:
  void clear();

  QString m_lastExec, m_lastParams, m_lastValExec, m_lastValParams,
      m_lastCtExec, m_lastCtParams, m_lastKcExec;

  QPointer<QTreeView> m_treeView;

  ValgrindModel* m_model;
  ValgrindControl* m_control;
};


#endif
