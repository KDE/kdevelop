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


#include <dlfcn.h>


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


static const char *KDLGLDR_LIBRARY_NAME          = "libkdlgloader.so";

class KDlgLdr;

static const char *FNC_KDLGLDR_INIT              = "kdlgldr_init";
static const char *FNC_KDLGLDR_DELETE            = "kdlgldr_delete";
static const char *FNC_KDLGLDR_OPENDLG           = "kdlgldr_opendlg";
static const char *FNC_KDLGLDR_FREEDLG           = "kdlgldr_freedlg";
static const char *FNC_KDLGLDR_GETITEMPTRNAME    = "kdlgldr_getitemptrname";
static const char *FNC_KDLGLDR_GETITEMPTRVARNAME = "kdlgldr_getitemptrvarname";
static const char *FNC_KDLGLDR_SETBEHAVIOUR      = "kdlgldr_setbehaviour";


KDlgLdr* (*KDlgLdr_Init)(QWidget*) = 0;                    // KDlgLoader::KDlgLoader()
long (*KDlgLdr_Delete)(KDlgLdr*) = 0;                      // delete KDlgLoader
long (*KDlgLdr_OpenDialog)(KDlgLdr*, QString) = 0;         // KDlgLoader::openDialog()
long (*KDlgLdr_FreeDialog)(KDlgLdr*) = 0;                  // KDlgLoader::freeDialog()
QWidget* (*KDlgLdr_GetItemPtrName)(KDlgLdr*, QString) = 0;     // KDlgLoader::getItemPtrName()
QWidget* (*KDlgLdr_GetItemPtrVarName)(KDlgLdr*, QString) = 0;  // KDlgLoader::getItemPtrVarName()
long (*KDlgLdr_SetBehaviour)(KDlgLdr*, long) = 0;          // KDlgLoader::setBehaviour()

void *KDlgLdr_Library = 0;                                 // Library handle

/** This method loades the library and gets the function pointers if not done before. */
bool loadKDlgLdrLibrary()
{
  if (!KDlgLdr_Library)
    {
      KDlgLdr_Library = dlopen(KDLGLDR_LIBRARY_NAME,RTLD_NOW);
      if (!KDlgLdr_Library)
        {
          warning("Error loading dialog loader library. (%s)\n",dlerror());
          return false;
        }
    }

  #define LOAD_LIB_FNC(fncname, fncptr) \
       if (!fncptr) { (void*)fncptr = dlsym(KDlgLdr_Library, fncname); \
        if (!fncptr) { warning("Getting dialog loader library method failed. (%s)",dlerror()); return false; } }

  LOAD_LIB_FNC(FNC_KDLGLDR_INIT,              KDlgLdr_Init)
  LOAD_LIB_FNC(FNC_KDLGLDR_DELETE,            KDlgLdr_Delete)
  LOAD_LIB_FNC(FNC_KDLGLDR_OPENDLG,           KDlgLdr_OpenDialog)
  LOAD_LIB_FNC(FNC_KDLGLDR_FREEDLG,           KDlgLdr_FreeDialog)
  LOAD_LIB_FNC(FNC_KDLGLDR_GETITEMPTRNAME,    KDlgLdr_GetItemPtrName)
  LOAD_LIB_FNC(FNC_KDLGLDR_GETITEMPTRVARNAME, KDlgLdr_GetItemPtrVarName)
  LOAD_LIB_FNC(FNC_KDLGLDR_SETBEHAVIOUR,      KDlgLdr_SetBehaviour)

  #undef LOAD_LIB_FNC

  return true;
}

/** This is a wrapper class for dealing with the library */
class KDlgLoader {
  public:
    /** construtor */
    KDlgLoader(QWidget *parent)
    {
      libLoaded = false;
      if (!loadKDlgLdrLibrary())
        return;

      if (ldr)
        ldr = KDlgLdr_Init(parent);

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

