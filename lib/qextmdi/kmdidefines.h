//----------------------------------------------------------------------------
//    filename             : kmdidefines.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                                         stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//
//    copyright            : (C) 1999-2003 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
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
#ifndef _KMDI_DEFINES_H_
#define _KMDI_DEFINES_H_

#include <kdelibs_export.h>

#define KMDI_CHILDFRM_SEPARATOR 2
#define KMDI_CHILDFRM_BORDER 4
#define KMDI_CHILDFRM_DOUBLE_BORDER 8
#define KMDI_CHILDFRM_MIN_WIDTH 130

//----------------------------------------------------------------------------
/** 
* @short A namespace for the KMDI library
*/
namespace KMdi
{
   /** extent Qt events
      @see QCustomEvent, QEvent::User
      \code
      bool B_MyWidget::event( QEvent* e) {
         if( e->type() == QEvent::Type(QEvent::User + int(KMdi::EV_Move))) {
            ...
         }
         ...
      }
      \endcode
   */
   enum EventType {
      EV_Move=1,
      EV_DragBegin,
      EV_DragEnd,
      EV_ResizeBegin,
      EV_ResizeEnd
   };

   /**
   * During KMdiMainFrm::addWindow the enum AddWindowFlags is used to determine how the view is initialy being added to the MDI system
   */
   enum AddWindowFlags {
      /**
      * standard is: show normal, attached, visible, document view (not toolview). Maximize, Minimize, Hide adds
      * appropriately. Detach adds a view that appears toplevel, ToolWindow adds the view as tool view.
      * That means it is stay-on-top and toplevel. UseKMdiSizeHint should use the restore geometry of the
      * latest current top childframe but is not supported yet.
      */
      StandardAdd = 0,
      Maximize    = 1,
      Minimize    = 2,
      Hide        = 4,
      Detach      = 8,
      ToolWindow  = 16,
      UseKMdiSizeHint = 32,
      AddWindowFlags = 0xff
   };

   enum FrameDecor {
      Win95Look = 0,
      KDE1Look  = 1,
      KDELook  = 2,
      KDELaptopLook = 3
   };

   enum MdiMode {
      UndefinedMode  = 0,
      ToplevelMode   = 1,
      ChildframeMode = 2,
      TabPageMode    = 3,
      IDEAlMode      = 4
   };

   enum TabWidgetVisibility {
      AlwaysShowTabs         = 0,
      ShowWhenMoreThanOneTab = 1,
      NeverShowTabs          = 2
   };

  /**
   * The style of the toolview tabs
   * \since 3.3
   */
   enum ToolviewStyle {
   /** Show only icons on the toolview tabs. The visible toolviews contain both the icon and text. */
      IconOnly = 0,
   /** Show only the text description on the toolview tabs. */
      TextOnly = 1,
   /** Show both the icon and description on the toolview tabs. */
      TextAndIcon = 3
   };
} //namespace

#endif //_KMDIDEFINES_H_
