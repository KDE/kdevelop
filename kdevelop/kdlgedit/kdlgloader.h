/***************************************************************************
                   kdlgloader.h  -  loading dialogs at runtime
                             -------------------
    begin                : Wed Jun  8 19:19:32 MEST 1999

    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDLGLOADER_H
#define KDLGLOADER_H

#include <qwidget.h>
#include <qstring.h>

#if hpux
#include <dl.h>
inline const char *dlerror()
{
	return "No dlerror() for HPUX";
}

#else
#include <dlfcn.h>
#endif

#define BEHAVE_REPORTERRORS 1               // Report errors to user by an message box
#define BEHAVE_IGNOREERRORS 2               // Ingore errors (i.e. while reading the dialog file)

#define KDLGERR_SUCCESS                 0   // Success
#define KDLGERR_UNKNOWN                 1   // Unknown error.
#define KDLGERR_NOT_SUPPORTED           2   // Requested function is not supported.
#define KDLGERR_BAD_PARAMETER           3   // Bad parameter given.
#define KDLGERR_NAME_DOES_NOT_EXIST     4   // Specified (var)name does not exist.
#define KDLGERR_NO_NAME_SPECIFIED       5   // No (var)name specified.
#define KDLGERR_WRONG_FLAGS             6   // Wrong or unsupported flags given.
#define KDLGERR_DLG_NOT_OPENED          7   // Dialog is not open.
#define KDLGERR_ERROR_READING_FILE      8   // Error while reading the dialog file.

class KDlgLdr;

typedef KDlgLdr* (*PFN_DLGLDR_INIT)(QWidget*);
typedef long (*PFN_DLGLDR_DELETE)(KDlgLdr*);
typedef long (*PFN_DLGLDR_OPENDIALOG)(KDlgLdr*, QString);
typedef long (*PFN_DLGLDR_FREEDIALOG)(KDlgLdr*);
typedef QWidget* (*PFN_DLGLDR_GIPNAME)(KDlgLdr*, QString);
typedef QWidget* (*PFN_DLGLDR_GIPVNAME)(KDlgLdr*, QString);
typedef long (*PFN_DLGLDR_SETBEHAVIOUR)(KDlgLdr*, long);

static const char *KDLGLDR_LIBRARY_NAME          = "libkdlgloader.so";

static const char *FNC_KDLGLDR_INIT              = "kdlgldr_init";
static const char *FNC_KDLGLDR_DELETE            = "kdlgldr_delete";
static const char *FNC_KDLGLDR_OPENDLG           = "kdlgldr_opendlg";
static const char *FNC_KDLGLDR_FREEDLG           = "kdlgldr_freedlg";
static const char *FNC_KDLGLDR_GETITEMPTRNAME    = "kdlgldr_getitemptrname";
static const char *FNC_KDLGLDR_GETITEMPTRVARNAME = "kdlgldr_getitemptrvarname";
static const char *FNC_KDLGLDR_SETBEHAVIOUR      = "kdlgldr_setbehaviour";


PFN_DLGLDR_INIT KDlgLdr_Init = 0;                    // KDlgLoader::KDlgLoader()
PFN_DLGLDR_DELETE KDlgLdr_Delete = 0;                      // delete KDlgLoader
PFN_DLGLDR_OPENDIALOG KDlgLdr_OpenDialog = 0;         // KDlgLoader::openDialog()
PFN_DLGLDR_FREEDIALOG KDlgLdr_FreeDialog = 0;                     // KDlgLoader::freeDialog()
PFN_DLGLDR_GIPNAME KDlgLdr_GetItemPtrName = 0;     // KDlgLoader::getItemPtrName()
PFN_DLGLDR_GIPVNAME KDlgLdr_GetItemPtrVarName = 0;  // KDlgLoader::getItemPtrVarName()
PFN_DLGLDR_SETBEHAVIOUR KDlgLdr_SetBehaviour = 0;          // KDlgLoader::setBehaviour()

void *KDlgLdr_Library = 0;                                 // Library handle

/** This method loades the library and gets the function pointers if not done before. */
bool loadKDlgLdrLibrary()
{
  if (!KDlgLdr_Library)
    {
#if hpux
      KDlgLdr_Library = shl_load(KDLGLDR_LIBRARY_NAME,BIND_DEFERRED, 0L);
#else
      KDlgLdr_Library = dlopen(KDLGLDR_LIBRARY_NAME,RTLD_NOW);
#endif
      if (!KDlgLdr_Library)
        {
          warning("Error loading dialog loader library. (%s)\n",dlerror());
          return false;
        }
    }

#if hpux
  #define LOAD_LIB_FNC(fncname, fncptr, fncast) \
       if (!fncptr) { \
         if (shl_findsym((mapped_shl_entry **)&KDlgLdr_Library, fncname, \
                        TYPE_PROCEDURE, &fncptr) != 0) { \
	        fncptr = 0; \
	     } \
         if (!fncptr) { \
	       warning("Getting dialog loader library method failed. (%s)",dlerror()); \
	       return false; \
	     } \
	   }
#else
  #define LOAD_LIB_FNC(fncname, fncptr, fncast) \
       if (!fncptr) { fncptr = (fncast) dlsym(KDlgLdr_Library, fncname); \
        if (!fncptr) { warning("Getting dialog loader library method failed. (%s)",dlerror()); return false; } }
#endif

  LOAD_LIB_FNC(FNC_KDLGLDR_INIT,              KDlgLdr_Init, PFN_DLGLDR_INIT)
  LOAD_LIB_FNC(FNC_KDLGLDR_DELETE,            KDlgLdr_Delete, PFN_DLGLDR_DELETE)
  LOAD_LIB_FNC(FNC_KDLGLDR_OPENDLG,           KDlgLdr_OpenDialog, PFN_DLGLDR_OPENDIALOG)
  LOAD_LIB_FNC(FNC_KDLGLDR_FREEDLG,           KDlgLdr_FreeDialog, PFN_DLGLDR_FREEDIALOG)
  LOAD_LIB_FNC(FNC_KDLGLDR_GETITEMPTRNAME,    KDlgLdr_GetItemPtrName, PFN_DLGLDR_GIPNAME)
  LOAD_LIB_FNC(FNC_KDLGLDR_GETITEMPTRVARNAME, KDlgLdr_GetItemPtrVarName, PFN_DLGLDR_GIPVNAME)
  LOAD_LIB_FNC(FNC_KDLGLDR_SETBEHAVIOUR,      KDlgLdr_SetBehaviour, PFN_DLGLDR_SETBEHAVIOUR)

  #undef LOAD_LIB_FNC

  return true;
}

/** This is a wrapper class for dealing with the library */
class KDlgLoader {
  public:
    /** construtor */
    KDlgLoader(QWidget *parent)
    {
      ldr=0l;
      libLoaded = false;
      if (!loadKDlgLdrLibrary())
        return;

      ldr = KDlgLdr_Init(parent);
      if (ldr)
        libLoaded = true;
    }

    /** destructor */
    ~KDlgLoader()
      {
         if (ldr)
         {
           freeDialog();
           deleteLoader();
         }
      }

    bool isLibLoaded() { return libLoaded; }

    /** Opens a dialog from a file specified by fileName. */
    long openDialog(QString fileName)
      {
        return KDlgLdr_OpenDialog(ldr, fileName);
      }

    /** Frees all the dialogs memory. */
    long freeDialog()
      {
        return KDlgLdr_FreeDialog(ldr);
      }

    /** Deletes the pointer (ldr) */
    long deleteLoader()
      {
        return KDlgLdr_Delete(ldr);
      }

    /** Returns a pointer to an item specified by itemName. */
    QWidget* getItemPtrName(QString itemName)
      {
        return KDlgLdr_GetItemPtrName(ldr, itemName);
      }

    /** Returns a pointer to an item specified by itemName. */
    QWidget* getItemPtrVarName(QString itemName)
      {
        return KDlgLdr_GetItemPtrVarName(ldr, itemName);
      }

    /** Changes the classes' behaviour. You can use a combination of the BEHAVE_* constants. */
    long setBehaviour(long flags = 0)
      {
        return KDlgLdr_SetBehaviour(ldr, flags);
      }

  private:
    /** this is a pointer of an abstract type */
    KDlgLdr *ldr;
    bool libLoaded;
};

#endif


