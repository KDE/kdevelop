/*
 * Copyright (C) 2001  <kurt@granroth.org>
 */

#ifndef _EDITORTEST_H_
#define _EDITORTEST_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapp.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <qtabwidget.h>


#include "keditor/editor.h"


class KToggleAction;


class EditorTest : public KParts::MainWindow
{
  Q_OBJECT
public:
		  
  EditorTest();
  virtual ~EditorTest();
	

private slots:

  void slotPartCreated(KParts::Part *part);
  void slotViewCreated(QWidget *view);
  void slotPartActivated(KParts::Part *part);
  void slotViewActivated(QWidget *view);


private:
  void setupAccel();
  void setupActions();

  KEditor::Editor *m_editor;
  KParts::PartManager *partManager;
  QTabWidget *tabWidget;

};

#endif // _EDITORTEST_H_
