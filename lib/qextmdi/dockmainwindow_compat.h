//----------------------------------------------------------------------------
//    filename             : dummykpartsdockmainwindow.h
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 10/2000       Falk Brettschneider
//
//    copyright            : (C) 1999-2000 by Falk Brettschneider
//
//    email                :  gigafalk@yahoo.com (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef DOCKMAINWINDOW_COMPAT_H
#define DOCKMAINWINDOW_COMPAT_H

#include "kmdidefines.h"
#include "kdockwidget_compat.h"
#include "kparts/part.h"

class DockMainWindowPrivate;

namespace KDockWidget_Compat {

/**
 * A KPart-aware main window with ability for docking widgets, whose user interface is described in XML.
 *
 * Inherit your main dock-window from this class
 * and don't forget to call @ref setXMLFile() in the inherited constructor.
 *
 * It implements all internal interfaces in the case of a @ref KDockMainWindow as host:
 * the builder and servant interface (for menu merging).
 */
class DockMainWindow : public KDockMainWindow, virtual public KParts::PartBase
{
  Q_OBJECT
 public:
  /**
   * Constructor, same signature as @ref KDockMainWindow.
   */
  DockMainWindow( QWidget* parent = 0L, const char *name = 0L, WFlags f = WDestructiveClose );
  /**
   * Destructor.
   */
  virtual ~DockMainWindow();

protected slots:

  /**
   * Create the GUI (by merging the host's and the active part's)
   *
   * Called on startup and whenever the active part changes
   * For this you need to connect this slot to the
   * @ref KPartManager::activePartChanged() signal
   * @param part The active part (set to 0L if no part).
   */
  void createGUI( KParts::Part * part );

  /**
   * Called when the active part wants to change the statusbar message
   * Reimplement if your dock-mainwindow has a complex statusbar
   * (with several items)
   */
  virtual void slotSetStatusBarText( const QString & );

protected:
    virtual void createShellGUI( bool create = true );

 private:
  DockMainWindowPrivate *d;
};

}; // KDockWidget_Compat

#endif   // DOCKMAINWINDOW_COMPAT_H
