/***************************************************************************
                          kdlgloader.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Jun  2 19:19:32 MEST 1999
                                           
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

#include "kdlgldr.h"
#include <qmessagebox.h>

extern "C" {

  KDlgLoader *kdlgldr_init(QWidget *parent)
    {
       KDlgLoader *ldr = new KDlgLoader(parent);
       return ldr;
    }

  long kdlgldr_opendlg(KDlgLoader *ldr, QString fileName)
    {
      if (!ldr)
        return KDLGERR_BAD_PARAMETER;

      return ldr->openDialog(fileName);
    }

  long kdlgldr_freedlg(KDlgLoader *ldr)
    {
      if (!ldr)
        return KDLGERR_BAD_PARAMETER;

      return ldr->freeDialog();
    }

  long kdlgldr_delete(KDlgLoader *ldr)
    {
      if (!ldr)
        return KDLGERR_BAD_PARAMETER;

      delete ldr;

      return KDLGERR_SUCCESS;
    }

  QWidget *kdlgldr_getitemptrname(KDlgLoader *ldr, QString itemName)
    {
      if (!ldr)
        return 0;

      return ldr->getItemPtrName(itemName);
    }

  QWidget *kdlgldr_getitemptrvarname(KDlgLoader *ldr, QString itemName)
    {
      if (!ldr)
        return 0;

      return ldr->getItemPtrVarName(itemName);
    }

  long kdlgldr_setbehaviour(KDlgLoader *ldr, long flags)
    {
      if (!ldr)
        return KDLGERR_BAD_PARAMETER;

      return ldr->setBehaviour(flags);
    }

}


void KDlgLoader::WatchMsg(const char* msg)
{
    fprintf(stderr,"KDlgLdr wth: %s\n", msg);
}

void WatchMsg(const char* msg)
{
    fprintf(stderr,"KDlgLdr wth: %s\n", msg);
}

void KDlgLoader::DebugMsg(const char* msg)
{
    fprintf(stderr,"KDlgLdr dbg ==> %s >%X<\n", msg, (unsigned int)lastErrorCode);
}






KDlgLoader::KDlgLoader(QWidget* Parent)
{
#ifdef dlgldr_wth
  WatchMsg("[KDlgLoader()]");
#endif

  parent = Parent;

  isDlgOpened = false;  // No dialog opened at startup time
  behaviour = BEHAVE_REPORTERRORS;   // We want the class to report errors in messageboxes by default
  lastErrorCode = KDLGERR_SUCCESS;    // We don´t had any error before startup ;)

  dbase = new KDlgPtrDb(0, parent);
  actdb = 0;
}

KDlgLoader::~KDlgLoader()
{
#ifdef dlgldr_wth
  WatchMsg("[~KDlgLoader()]");
#endif

  delete dbase;

  if (isDialogOpened())
    {
      freeDialog();
    }
}

long KDlgLoader::openDialog(QString fileName, long flags)
{
#ifdef dlgldr_wth
  WatchMsg("[openDialog()]");
#endif

  if (fileName.isEmpty())
    {
      Return(KDLGERR_NO_NAME_SPECIFIED)
    }

  if (isDialogOpened())
    {
      freeDialog();
    }

  filename = fileName;

  readFile(flags);
  Validate

  isDlgOpened = true;

  Return(KDLGERR_SUCCESS)
}


long KDlgLoader::freeDialog(long flags)
{
#ifdef dlgldr_wth
  WatchMsg("[freeDialog()]");
#endif
  ValidateDlgOpened

  Return(KDLGERR_SUCCESS)
}


/** Protected method to read the file from hard drive. */
long KDlgLoader::readFile(long flags)
{
  dbase->deleteMyself();
#ifdef dlgldr_wth
  WatchMsg("[readFile()]");
#endif

  dlgfilelinecnt = 0;
  QFile f(filename);
  if ( f.open(IO_ReadOnly) )
    {
      QTextStream t( &f );

      while (!t.eof())
        {
          readGroup( &t );
          if LastFailed
            {
#ifdef dlgldr_dbg
                DebugMsg("readGroup failed!");
#endif
                if (DoesBehave(BEHAVE_REPORTERRORS) && !DoesBehave(BEHAVE_IGNOREERRORS))
                  {
                    switch ( QMessageBox::warning( parent, i18n("Dialog loader"),
                               i18n("An error occured while loading an "
                               "dialog needed for the application.\n\n"
                               "You may ignore this or close the "
                               "application now. Notice if you choose\n"
                               "close, all data will be lost!"),
                               i18n("&Ignore"), i18n("Cancel &loading"), i18n("&Close application"),
                               0, 1 ) )
                      {
                        case 0: // Ignore
                          printf(i18n("An error occured while opening a dialog. User chose to ignore and continue.\n"));
                        break;
                        case 1:
                          f.close();
                          Return(KDLGERR_ERROR_READING_FILE)
                        break;
                        case 2: // Close application
                          printf(i18n("\n\nThis application was closed because an error occured while loading a needed dialog.\n"));
                          exit(1);
                        break;
                      }
                  }
                else
                  {
                    if (!DoesBehave(BEHAVE_IGNOREERRORS))
                      Return(KDLGERR_ERROR_READING_FILE)
                  }
            }
        }
    }

  Return(KDLGERR_SUCCESS)
}


/** reads one group from the dialog file. */
long KDlgLoader::readGroup( QTextStream *t )
{
#ifdef dlgldr_wth
  WatchMsg("[readGroup()]");
#endif

  QString s;
  s = dlgReadLine(t);
  if (s.isEmpty())
    Return(KDLGERR_UNKNOWN)

  QString type = s.left(s.find(' ')).upper();
  QString name = s.right(s.length()-s.find(' ')-1);

  if ((s == "{") || (s == "}"))
    Return(KDLGERR_SUCCESS)

#ifdef dlgldr_dbg
  char msg[512];
  sprintf(msg, "reading root %s-section \"%s\"",(const char*)type.lower(),(const char*)name);
  DebugMsg(msg);
#endif

  if (type == "DATA")
    {
      if (name.upper() == "INFORMATION")
        {
          readGrp_Information( t );
          Validate
        }
      else if (name.upper() == "SESSIONMANAGEMENT")
        {
          readGrp_SessionManagement( t );
          Validate
        }
      else
        {

#ifdef dlgldr_dbg
          sprintf(msg,"line %d : unknown data type \"%s\". Section ignored.", dlgfilelinecnt, (const char*)name);
          DebugMsg(msg);
#endif

          readGrp_Ignore( t );
          Validate
        }
    }
  else if (type == "ITEM")
    {
      readGrp_Item( 0, t, name, dbase );
      Validate
    }


  Return(KDLGERR_SUCCESS)
}







QString KDlgLoader::dlgReadLine( QTextStream *t )
{
  QString s;
  do {
    s = t->readLine().stripWhiteSpace();
    dlgfilelinecnt++;
  } while ( (!t->eof()) && ((s.left(2) == "//") || (s.isEmpty())) );

  if (s.left(2) == "//")
    return QString();

  if (s.isEmpty())
    return QString();

  return s;
}

long KDlgLoader::readGrp_Ignore( QTextStream *t )
{
#ifdef dlgldr_wth
  WatchMsg("[readGrp_Ignore()]");
#endif
  QString s;
  int cnt = 0;
  do {
    s = dlgReadLine(t);
    if (s=="{") cnt++;
    if (s=="}")
      {
        if (cnt <= 1)
          {
            Return(KDLGERR_SUCCESS)
          }
        else
          cnt --;
      }
  } while (!t->eof());

  Return(KDLGERR_SUCCESS)
}

long KDlgLoader::readGrp_Information( QTextStream *t )
{
#ifdef dlgldr_wth
  WatchMsg("[readGrp_Information()]");
#endif
  return readGrp_Ignore( t );
}

long KDlgLoader::readGrp_SessionManagement( QTextStream *t )
{
#ifdef dlgldr_wth
  WatchMsg("[readGrp_SessionManagement()]");
#endif
  return readGrp_Ignore( t );
}

long KDlgLoader::readGrp_Item( QWidget* par, QTextStream *t, QString ctype, KDlgPtrDb *pardb )
{
#ifdef dlgldr_wth
  WatchMsg("[readGrp_Item()]");
#endif

#ifdef dlgldr_dbg
  char msg[512];
  sprintf(msg, "reading subitem-section \"%s\"",(const char*)ctype.lower());
  DebugMsg(msg);
#endif

  QWidget *thatsme;

  if (par)
    {
      thatsme = createItem(par, ctype);
    }
  else
    {
      thatsme = parent;
    }

  if (!thatsme)
    {
#ifdef dlgldr_dbg
      DebugMsg("Error creating item (maybe I don´t know it ?)");
#endif
      readGrp_Ignore( t );
      Return(KDLGERR_SUCCESS)
    }

  if (dlgReadLine(t) != "{")
    {
#ifdef dlgldr_dbg
      DebugMsg("Parse error (no \"{\" found at the beginning of the block)");
#endif
      Return(KDLGERR_UNKNOWN)
    }

  KDlgPtrDb *chdb = new KDlgPtrDb( pardb, thatsme );
  pardb->groups.append( chdb );
  actdb = chdb;

  QString s;
  do {
    s = dlgReadLine(t);
    if ((s!="}") && (thatsme))
      {
        if (s.left(s.find(' ')).upper() == "ITEM")
          {
            if ((ctype.upper() == "QWIDGET") || (ctype.upper() == "QFRAME")
		|| (ctype.upper() == "QBUTTONGROUP") || (ctype.upper() == "QGROUPBOX")) // da
              {
#ifdef dlgldr_wth
                WatchMsg("Child widget");
#endif
                readGrp_Item( thatsme, t, s.right(s.length()-s.find(' ')-1), chdb );
                Validate
              }
            else
              {
#ifdef dlgldr_dbg
                char msg[512];
                sprintf(msg,"line %d : \"%s\" cannot have child items.\n", dlgfilelinecnt, (const char*)ctype);
                DebugMsg(msg);
#endif
                readGrp_Ignore( t );
                actdb = 0;
                Return(KDLGERR_UNKNOWN)
              }
          }
        else
          {
            QString name  = s.left(s.find('=')).lower();
            QString value = s.right(s.length()-s.find('=')-2);
            value = value.left(value.length()-1);
            setProperty(thatsme, name, value, ctype);
          }
      }
  } while ((!t->eof()) && (s!="}"));

  actdb = 0;

  Return(KDLGERR_SUCCESS)
}




QWidget* KDlgLoader::getItemPtrVarName(QString itemName)
{
#ifdef dlgldr_wth
  WatchMsg("[getItemPtrVarName()]");
#endif
  if (!dbase)
    return 0;

  itemName = itemName.lower();

  return dbase->getItemPtrVarName(itemName);
}



QWidget* KDlgLoader::getItemPtrName(QString itemName)
{
#ifdef dlgldr_wth
  WatchMsg("[getItemPtrVarName()]");
#endif
  if (!dbase)
    return 0;

  itemName = itemName.lower();

  return dbase->getItemPtrName(itemName);
}


QWidget* KDlgLoader::getItemPtrName(int itemId)
{
  QString s;
  s.setNum(itemId);
  return getItemPtrName(s);
}

QWidget* KDlgLoader::getItemPtrVarName(int itemId)
{
  QString s;
  s.setNum(itemId);
  return getItemPtrVarName(s);
}




