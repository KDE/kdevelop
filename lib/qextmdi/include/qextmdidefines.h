//----------------------------------------------------------------------------
//    filename             : qextmdidefines.h
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                                         stand-alone Qt extension set of
//                                         classes and a Qt-based library
//
//    copyright            : (C) 1999-2000 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
//    email                :  gigafalk@geocities.com (Falk Brettschneider)
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
#ifndef _MDIDEFINES_H_
#define _MDIDEFINES_H_

#include <qglobal.h>

#ifdef NO_KDE2
#undef NO_KDE
#define NO_KDE
#endif

#define QEXTMDI_MDI_CHILDFRM_SEPARATOR 2
#define QEXTMDI_MDI_CHILDFRM_BORDER 3
#define QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER 6
#define QEXTMDI_MDI_CHILDFRM_MIN_WIDTH 130

//----------------------------------------------------------------------------
namespace QextMdi
{
   /** extent Qt events
      @see QCustomEvent, QEvent::User 
      <PRE>
      bool
      B_MyWidget::event( QEvent* e) {
         if( e->type() == QEvent::Type(QEvent::User + int(QextMdi::EV_Move))) {
            ...
         }
         ...
      }
      </PRE>
   */
   enum EventType {
      EV_Move=1,
      EV_DragBegin,
      EV_DragEnd,
      EV_ResizeBegin,
      EV_ResizeEnd
   };

   /**
   * During @ref QextMdiMainFrm::addWindow the enum AddWindowFlags is used to determine how the view is initialy being added to the MDI system
   */
   enum AddWindowFlags {
      /**
      * standard is: show normal, attached, visible, document view (not toolview). Maximize, Minimize, Hide adds
      * appropriately. Detach adds a view that appears toplevel, ToolWindow adds the view as tool view.
      * That means it is stay-on-top and toplevel. UseQextMDISizeHint should use the restore geometry of the
      * latest current top childframe but is not supported yet.
      */
      StandardAdd = 0,
      Maximize    = 1,
      Minimize    = 2,
      Hide        = 4,
      Detach      = 8,
      ToolWindow  = 16,
      UseQextMDISizeHint = 32
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
      TabPageMode    = 2
   };

}; //namespace


//----------------------------------------------------------------------------
#ifndef _DLL_IMP_EXP_MSG_
#   define _DLL_IMP_EXP_MSG_
#endif

#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
  /* QT linked libraries compiled with MSVC */
#  ifdef MAKEDLL_QEXTMDI
    /* for building qextmdi */
#   ifdef _DLL_IMP_EXP_MSG_
#     ifdef _DEBUG
#      pragma message ("  exporting C++ class to debug lib...")
#      else
#      pragma message ("  exporting C++ class to release lib...")
#      endif
#    endif
#    define DLL_IMP_EXP_QEXTMDICLASS  __declspec(dllexport)
#  else
    /* for including headers of qextmdi */
#    ifdef _DLL_IMP_EXP_MSG_
#      ifdef _DEBUG
#      pragma message ("  importing C++ class from qextmdi debug lib...")
#      else
#      pragma message ("  importing C++ class from qextmdi release lib...")
#      endif
#    endif
#    define DLL_IMP_EXP_QEXTMDICLASS  __declspec(dllimport)
#  endif
#else
#  define DLL_IMP_EXP_QEXTMDICLASS
#endif

#endif //_MDIDEFINES_H_
