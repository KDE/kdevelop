/***************************************************************************
                          mainpage.dox.h  -  description
                             -------------------
    begin                : Sam Apr 12 2003
    copyright            : (C) 2003 by KDevelop Authors
    email                : kdevelop-devel@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** \file  mainpage.dox.h
  * \brief KDevelop 3 achitecture (Doxygen main page)
  */

/** \mainpage KDevelop 3 Architecture

\section intro Introduction

KDevelop is an easy to use IDE for developing C/C++ applications under X11.
Currently it supports:

  - KDE applications
  - QT applications
  - Fortran
  - Java
  - Perl
  - Phyton
  - PHP
  - Ruby
  - Pascal
  - Ada
  .

For more info goto: http://www.kdevelop.org

\section overview Source Overview

\subsection source-division Main Source Divisions

The KDevelop 3 source is divided into several parts which correspond to
subdirectories in the KDevelop project directory. There are several main
parts to distinguish, mainly:

  - src = The core part of KDevelop
  - lib/interfaces = Plugin handler interface classes
  - parts = The various parts using the KParts framework
  .
      
\subsection core-part The Core Part of KDevelop

\subsubsection mainwindows Main Window Objects

There are two types of possible main window objects:

 - MainWindow implements standard MDI user interfaces:
   - Top level mode (see MainWindow::switchToToplevelMode() )
   - Childframe mode (see MainWindow::switchToChildframeMode() )
   - Tab page mode (see MainWindow::switchToTabPageMode() )
   .
 - MainWindowIDEAl implements an enhanced MDI main window providing a set of
   pre-arranged access tabs around user areas.
 .

Both main window classes inherit from the KDevMainWindow class which provides
access to common window features.

\subsubsection toplevel The TopLevel Object

There is only one toplevel object of class KDevMainWindow in Gideon. It can
be accessed through the static function TopLevel::getInstance() (see the 
TopLevel class).

\subsection parts-overview KDevelop Parts Overview

All parts reside in dedicated subdirectories of the /parts directory. They can
be viewed according to their functionalities as follows.
 
\subsubsection global-parts Global Parts in KDevelop

Some of the parts are considered global - that is, they effect the entire
operation of KDevelop.

  - parts/appwizard = New Project Wizard
    - (see AppWizardPart)
    .
  - parts/history = Project history
    - (see HistoryPart)
    .
  - parts/konsole = Embedded Konsole
    - (see KonsoleViewPart)
    .
  - parts/editor-chooser = Chooses an internal text editor
    - (see EditorChooserPart)
    .
  - parts/doctreeview = Documentation Viewer
    - (see DocTreeViewPart)
    .
  - parts/openwith = "Open with" menu addon
    - (see OpenWithPart)
    .
  - parts/fileselector = A file selection widget
    - (see FileSelectorPart)
    .
  - parts/uimode = Customize the UI mode
    - (see UIChooserPart)
    .
  - parts/texttools = Additional text tools
    - (see TextToolsPart)
    .
  - parts/tipofday = A tip of the day
    - (see TipOfDayPart)
    .
  - parts/grepview = A graphical grep utility
    - (see GrepViewPart)
    .
  - parts/cvs = CVS Support
    - (see CvsPart)
    .
  - parts/tools = Toolbar management
    - (see ToolsPart)
    .
  - parts/regexptest = A regular expression tester
    - (see RegexpTestPart)
    .
  - parts/abbrev = Abbreviation Expansion (see AbbrevPart class)
    - (see AbbrevPart)
    .
  - parts/filter = Shell Filtering and Insertion
    - (see FilterPart)
    .
  - parts/valgrind = A graphical valgrind frontend
    - (see ValgrindPart)
    .
  .

\subsubsection project-management-parts Project Management Parts in KDevelop

  - parts/antproject = ANT Project Manager
    - (see AntProjectPart)
    .
  - parts/autoproject = Automake Project Manager
    - (see AutoProjectPart)
    .
  - parts/customproject = Custom Project Manager
    - (see CustomProjectPart)
    .
  - parts/trollproject = QMake based Project Manager
    - (see TrollProjectPart)
    .
  .

\subsubsection language-parts Language Support Parts in KDevelop

  - parts/cppsupport = Support for C/C++
    - (see CppSupportPart)
    .
  - parts/fortransupport = Support for Fortran
    - (see FortranSupportPart)
    .
  - parts/javasupport = Support for Java
    - (see JavaSupportPart)
    .
  - parts/perlsupport = Support for Perl
    - (see PerlSupportPart)
    .
  - parts/phpsupport = Support for PHP
    - (see PHPSupportPart)
    .
  - parts/pythonsupport = Support for Python
    - (see PythonSupportPart)
    .
  - parts/rubysupport = Support for Ruby
    - (see RubySupportPart)
    .
  - parts/pascalsupport = Support for Pascal
    - (see PascalSupportPart)
    .
  - parts/adasupport = Support for Ada
    - (see AdaSupportPart)
    .
  .

\subsubsection project-specific-parts Project Specific Parts in KDevelop

  - parts/astyle = Source code formatter
    - (see AStylePart)
    .
  - parts/buglist = Bug tracking application
    - (see BugList)
    .
  - parts/classview = Classview Manager
    - (see ClassViewPart)
    .
  - parts/ctags = CTags frontend
    - (see CTagsPart)
    .
  - parts/debugger = GNU Debugger Manager
    - (see DebuggerPart)
    .
  - parts/diff = Difference viewer
    - (see DiffPart)
    .
  - parts/distpart = Aids in building and publishing the final project
    - (see DistpartPart)
    .
  - parts/doxygen = Doxygen integration
    - (see DoxygenPart)
    .
  - parts/fileview = File groups
    - (see FileViewPart, FileGroupsPart)
    .
  - parts/javadebugger = Java debugger frontend
    - (see JavaDebuggerPart)
    .
  - parts/manager = Document manager
    - (see DocManager, ViewManager)
    .
  - parts/nedit = KTextEditor interface for nedit
    - (see KNEditFactory, Document, View)
    .
  - parts/outputviews = Application output views
    - (see AppOutputViewPart, MakeViewPart)
    .
  - parts/perforce = Version managment system perforce integration
    - (see PerforcePart)
    .
  - parts/qeditor = QEditor integration
    - (see QEditorPart)
    .
  - parts/scripting = Python scripting interface
    - (see ScriptingPart)
    .
  - parts/scriptproject = Scripting language project
    - (see ScriptProjectPart)
    .
  - parts/sourcenav = Source code navigation
    - (see SourceNavPart)
    .
  - parts/svn = Subversion integration
    - (see SvnPart)
    .
  - parts/visualboyadvance = VisualBoy Advance integration
    - (see VisualBoyAdvancePart)
    .
  .

*/

