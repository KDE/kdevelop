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
#ifndef _KMDIDEFINES_H_
#define _KMDIDEFINES_H_

#include <qglobal.h>

#ifdef NO_KDE2
#undef NO_KDE
#define NO_KDE
#endif

#define KMDI_CHILDFRM_SEPARATOR 2
#define KMDI_CHILDFRM_BORDER 3
#define KMDI_CHILDFRM_DOUBLE_BORDER 6
#define KMDI_CHILDFRM_MIN_WIDTH 130

//----------------------------------------------------------------------------
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
      ToplevelMode   = 0,
      ChildframeMode = 1,
      TabPageMode    = 2,
      IDEAlMode      = 3
   };

} //namespace


//----------------------------------------------------------------------------
#ifndef _DLL_IMP_EXP_MSG_
#   define _DLL_IMP_EXP_MSG_
#endif

#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
  /* QT linked libraries compiled with MSVC */
#  ifdef MAKEDLL_KMDI
    /* for building kmdi */
#   ifdef _DLL_IMP_EXP_MSG_
#     ifdef _DEBUG
#      pragma message ("  exporting C++ class to debug lib...")
#      else
#      pragma message ("  exporting C++ class to release lib...")
#      endif
#    endif
#    define DLL_IMP_EXP_KMDICLASS  __declspec(dllexport)
#  else
    /* for including headers of kmdi */
#    ifdef _DLL_IMP_EXP_MSG_
#      ifdef _DEBUG
#      pragma message ("  importing C++ class from kmdi debug lib...")
#      else
#      pragma message ("  importing C++ class from kmdi release lib...")
#      endif
#    endif
#    define DLL_IMP_EXP_KMDICLASS  __declspec(dllimport)
#  endif
#else
#  define DLL_IMP_EXP_KMDICLASS
#endif

#endif //_KMDIDEFINES_H_
