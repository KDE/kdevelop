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

#define QEXTMDI_MDI_CHILDFRM_SEPARATOR 2
#define QEXTMDI_MDI_CHILDFRM_BORDER 3
#define QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER 6
#define QEXTMDI_MDI_CHILDFRM_MIN_WIDTH 306
#define QEXTMDI_MDI_CHILDFRM_MIN_HEIGHT 155

#ifndef _DLL_IMP_EXP_MSG_
   #define _DLL_IMP_EXP_MSG_
#endif

#ifdef _OS_WIN32_
  /* QT linked libraries compiled with MSVC */
  #ifdef MAKEDLL_QEXTMDI
    /* for building qextmdi */
    #ifdef _DLL_IMP_EXP_MSG_
      #ifdef _DEBUG
      #pragma message ("  exporting C++ class to debug lib...")
      #else
      #pragma message ("  exporting C++ class to release lib...")
      #endif
    #endif
    #define DLL_IMP_EXP_QEXTMDICLASS  __declspec(dllexport)
  #else
    /* for including headers of qextmdi */
    #ifdef _DLL_IMP_EXP_MSG_
      #ifdef _DEBUG
      #pragma message ("  importing C++ class from qextmdi debug lib...")
      #else
      #pragma message ("  importing C++ class from qextmdi release lib...")
      #endif
    #endif
    #define DLL_IMP_EXP_QEXTMDICLASS  __declspec(dllimport)
  #endif
#else
  #define DLL_IMP_EXP_QEXTMDICLASS
#endif

#endif //_MDIDEFINES_H_
