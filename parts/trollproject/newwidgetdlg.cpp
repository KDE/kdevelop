/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "newwidgetdlg.h"
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <qpushbutton.h>
#include <domutil.h>

NewWidgetDlg::NewWidgetDlg(QStringList &newFileNames,QWidget* parent, const char* name, bool modal, WFlags fl)
: NewWidgetDlgBase(parent,name,modal,fl),
m_newFileNames(newFileNames)
//=================================================
{
}


NewWidgetDlg::~NewWidgetDlg()
//==============================================
{
}


void NewWidgetDlg::subclassingPressed()
//=====================================
{
  QMessageBox::information(0,"subclassing","");
}


void NewWidgetDlg::templateSelChanged()
//=====================================
{
  QMessageBox::information(0,"template","");
}


bool NewWidgetDlg::openXMLFile(QDomDocument &doc, QString filename)
//=================================================================
{
  QFile file( filename );
  if ( !file.open( IO_ReadOnly ) )
    return false;
  if ( !doc.setContent( &file ) ) {
    file.close();
    return false;
  }
  file.close();
  return true;
}

bool NewWidgetDlg::saveXMLFile(QDomDocument &doc, QString filename)
//=================================================================
{
  QFile file( filename );
  if ( !file.open( IO_ReadWrite | IO_Truncate ) )
    return false;
  QTextStream t( &file );
  t << doc.toString();
  file.close();
  return true;
}

void NewWidgetDlg::removeTextNodes(QDomDocument doc,QString pathExt)
//==================================================================
{
  QDomElement elem = DomUtil::elementByPathExt(doc,pathExt);
  QDomNodeList children = elem.childNodes();
  for (unsigned int i=0;i<children.count();i++)
    if (children.item(i).isText())
      elem.removeChild(children.item(i));
}


void NewWidgetDlg::appendTextNode(QDomDocument doc, QString pathExt, QString text)
//================================================================================
{
  QDomElement elem = DomUtil::elementByPathExt(doc,pathExt);
  elem.appendChild(doc.createTextNode(text));
}



void NewWidgetDlg::accept()
//=========================
{
  QDomDocument doc;
  openXMLFile(doc,"/home/jsgaarde/programming/kdevelop/domapp/clean_dialog.ui");
  removeTextNodes(doc,"class");
  appendTextNode(doc,"class","TestClass");
  removeTextNodes(doc,"widget/property|name=caption/string");
  appendTextNode(doc,"widget/property|name=caption/string","Test Dialog");
  saveXMLFile(doc,"/home/jsgaarde/programming/kdevelop/domapp/clean_dialog2.ui");
  NewWidgetDlgBase::accept();
}

