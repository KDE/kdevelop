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


#include "keditor/editor.h"


class KToggleAction;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Matthias Hölzer-Klüpfel <mhk@caldera.de>
 * @version 0.1
 */
class EditorTest : public KParts::MainWindow
{
  Q_OBJECT
public:
  /**
   * Default Constructor
   */
  EditorTest();

  /**
   * Default Destructor
   */
  virtual ~EditorTest();

  /**
   * Use this method to load whatever file/URL you have
   */
  void load(const KURL& url);

protected:

  /**
      * This method is called when it is time for the app to save its
      * properties for session management purposes.
      */
  void saveProperties(KConfig *);

  /**
   * This method is called when this app is restored.  The KConfig
   * object points to the session management config file that was saved
   * with @ref saveProperties
   */
  void readProperties(KConfig *);

private slots:
 
  void insertText();
  void appendText();

		
private:
  void setupAccel();
  void setupActions();

private:
  KEditor::Editor *m_editor;
};

#endif // _EDITORTEST_H_
