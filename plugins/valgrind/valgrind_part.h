/*
 *  Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>   
 */

#ifndef __KDEVPART_VALGRIND_H__
#define __KDEVPART_VALGRIND_H__

#include <qpointer.h>
#include <QString>
#include <qstringlist.h>
#include <kdevplugin.h>

class ValgrindWidget;
class KProcess;

class ValgrindPart : public KDevPlugin
{
  Q_OBJECT

public:   
  ValgrindPart( QObject *parent, const char *name, const QStringList & );
  ~ValgrindPart();
  
  void runValgrind( const QString& exec, const QString& parameters, const QString& valExec, const QString& valParameters );

  void savePartialProjectSession( QDomElement* el );
  void restorePartialProjectSession( const QDomElement* el );

private slots:
  void slotExecValgrind();
  void slotExecCalltree();
  void slotKillValgrind();
  void slotStopButtonClicked( KDevPlugin* which );
  void receivedStdout( KProcess*, char*, int );
  void receivedStderr( KProcess*, char*, int );
  void processExited( KProcess* );
  void loadOutput();
  void projectOpened();
  
private:
  void getActiveFiles();
  void appendMessage( const QString& message );
  void appendMessages( const QStringList& lines );
  void receivedString( const QString& str );
  void clear();
  QString _lastExec, _lastParams, _lastValExec, _lastValParams,
      _lastCtExec, _lastCtParams, _lastKcExec;
  KProcess* proc;
  QString currentMessage;
  QString lastPiece;
  QStringList activeFiles;
  int currentPid;
  QPointer<ValgrindWidget> m_widget;
  struct {
      bool runKc;
      QString kcPath;
//      QString kcWorkDir;
  } kcInfo;
};


#endif
