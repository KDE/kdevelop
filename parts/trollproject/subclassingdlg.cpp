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

#include "subclassingdlg.h"
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <qpushbutton.h>
#include <domutil.h>
#include <qdom.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <qfile.h>
#include <qregexp.h>


#define WIDGET_CAPTION_NAME "widget/property|name=caption/string"
#define WIDGET_CLASS_NAME   "class"
#define WIDGET_SLOTS        "slots"
#define WIDGET_FUNCTIONS    "functions"

SlotItem::SlotItem(QListView *parent,const QString &methodName,
                   const QString &specifier,
                   const QString &access, const QString &returnType,
                   bool isFunc)
: QCheckListItem(parent,methodName,QCheckListItem::CheckBox)
{
  setOn(true);
  m_methodName = methodName;
  m_access = access == "" ? "public" : access;
  m_specifier = specifier == "" ? "virtual" : specifier;
  m_returnType = returnType == "" ? "void" : returnType;
  setText(0,m_methodName);
  setText(1,m_access);
  setText(2,m_specifier);
  setText(3,m_returnType);
  setText(4,isFunc ? "Function" : "Slot");
}


SubclassingDlg::SubclassingDlg(const QString &formFile,QStringList &newFileNames,
                               QWidget* parent, const char* name,bool modal, WFlags fl)
: SubclassingDlgBase(parent,name,modal,fl),
m_newFileNames(newFileNames)
//=================================================
{
  m_formFile = formFile;
  QStringList splitPath = QStringList::split('/',formFile);
  m_formName = QStringList::split('.',splitPath[splitPath.count()-1])[0]; // "somedlg.ui" = "somedlg"
  splitPath.pop_back();
  m_formPath = "/" + splitPath.join("/"); // join path to ui-file
  kdDebug(9010) << "Formpath: " << m_formPath << endl;
  kdDebug(9010) << "Formname: " << m_formName << endl;

  QDomDocument doc;
  DomUtil::openDOMFile(doc,formFile);
  m_baseClassName = DomUtil::elementByPathExt(doc,WIDGET_CLASS_NAME).text();
  kdDebug(9010) << "Baseclass: " << m_baseClassName << endl;
  m_baseCaption = DomUtil::elementByPathExt(doc,WIDGET_CAPTION_NAME).text();
  setCaption(i18n("Create subclass of ")+m_baseClassName);

  QDomElement slotsElem = DomUtil::elementByPathExt(doc,WIDGET_SLOTS);
  QDomNodeList slotnodes = slotsElem.childNodes();
  for (unsigned int i=0; i<slotnodes.count();i++)
  {
    QDomElement slotelem = slotnodes.item(i).toElement();
    SlotItem *newSlot = new SlotItem(m_slotView,slotelem.text(),
                                     slotelem.attributeNode("specifier").value(),
                                     slotelem.attributeNode("access").value(),
                                     slotelem.attributeNode("returnType").value(),false);
    m_slotView->insertItem(newSlot);
    m_slots << newSlot;
  }

  QDomElement funcsElem = DomUtil::elementByPathExt(doc,WIDGET_FUNCTIONS);
  QDomNodeList funcnodes = funcsElem.childNodes();
  for (unsigned int i=0; i<funcnodes.count();i++)
  {
    QDomElement funcelem = funcnodes.item(i).toElement();
    SlotItem *newFunc = new SlotItem(m_slotView,funcelem.text(),
                                     funcelem.attributeNode("specifier").value(),
                                     funcelem.attributeNode("access").value(),
                                     funcelem.attributeNode("returnType").value(),true);
    m_slotView->insertItem(newFunc);
    m_functions << newFunc;
  }

}


SubclassingDlg::~SubclassingDlg()
//===============================
{
}


void SubclassingDlg::updateDlg()
//==============================
{
}

void SubclassingDlg::replace(QString &string, const QString& search, const QString& replace)
//==========================================================================================
{
  int nextPos = string.find(search);
  unsigned int searchLength = search.length();
  while (nextPos>-1)
  {
    string = string.replace(nextPos,searchLength,replace);
    nextPos = string.find(search,nextPos+replace.length());
  }
}

bool SubclassingDlg::loadBuffer(QString &buffer, const QString& filename)
//======================================================================
{
  // open file and buffer it
  QFile dataFile(filename);
  if (!dataFile.open(IO_ReadOnly))
    return false;
  char *temp = new char[dataFile.size()+1];
  dataFile.readBlock(temp,dataFile.size());
  temp[dataFile.size()]='\0';
  buffer = temp;
  delete temp;
  dataFile.close();
  return true;
}

bool SubclassingDlg::replaceKeywords(QString &buffer)
//=====================================================
{
  replace(buffer,"$NEWFILENAMEUC$",m_edFileName->text().upper());
  replace(buffer,"$BASEFILENAMELC$",m_formName.lower());
  replace(buffer,"$NEWCLASS$",m_edClassName->text());
  replace(buffer,"$BASECLASS$",m_baseClassName);
  replace(buffer,"$NEWFILENAMELC$",m_edFileName->text().lower());
  return true;
}

bool SubclassingDlg::saveBuffer(QString &buffer, const QString& filename)
//=======================================================================
{
  // save buffer

  QFile dataFile(filename);
  if (!dataFile.open(IO_WriteOnly))
    return false;
  dataFile.writeBlock((buffer+"\n").ascii(),(buffer+"\n").length());
  dataFile.close();
  return true;
}


void SubclassingDlg::accept()
//===========================
{
  unsigned int i;

  // h - file

  QString public_decl =
    "/*$PUBLIC_SLOTS$*/\n  ";

  QString protected_decl =
    "/*$PROTECTED_SLOTS$*/\n  ";

  QString function_decl =
    "/*$FUNCTIONS$*/\n  ";

  QString buffer;
  loadBuffer(buffer,::locate("data", "kdevtrollproject/subclassing/subclass_template.h"));
  replaceKeywords(buffer);
  for (i=0; i<m_slots.count(); i++)
  {
    SlotItem *slitem = m_slots[i];
    QString decl;
    if (slitem->m_access=="public")
      decl = public_decl;
    if (slitem->m_access=="protected")
      decl = protected_decl;
    decl += slitem->m_specifier=="non virtual" ? "" : slitem->m_specifier + " ";
    decl += slitem->m_returnType + " ";
    QString spacer;
    if (slitem->m_access=="public")
    {
      decl += spacer.fill(' ',43-decl.length()) + slitem->m_methodName + ";";
      replace(buffer,"/*$PUBLIC_SLOTS$*/",decl);
    }
    if (slitem->m_access=="protected")
    {
      decl += spacer.fill(' ',46-decl.length()) + slitem->m_methodName + ";";
      replace(buffer,"/*$PROTECTED_SLOTS$*/",decl);
    }
  }
  saveBuffer(buffer,m_formPath + "/" + m_edFileName->text()+".h");

  // cpp - file

  QString implementation =
    "/*$SPECIALIZATION$*/\n"
    "$RETURNTYPE$ $NEWCLASS$::$METHOD$\n"
    "{\n"
    "}\n\n";

  loadBuffer(buffer,::locate("data", "kdevtrollproject/subclassing/subclass_template.cpp"));
  replaceKeywords(buffer);
  for (i=0; i<m_slots.count(); i++)
  {
    SlotItem *slitem = m_slots[i];
    QString impl = implementation;
    replace(impl,"$RETURNTYPE$",slitem->m_returnType);
    replace(impl,"$NEWCLASS$",m_edClassName->text());
    replace(impl,"$METHOD$", slitem->m_methodName);
    replace(buffer,"/*$SPECIALIZATION$*/",impl);
  }
  saveBuffer(buffer,m_formPath + "/" + m_edFileName->text()+".cpp");

  m_newFileNames.append(m_formPath + "/" + m_edFileName->text()+".cpp");
  m_newFileNames.append(m_formPath + "/" + m_edFileName->text()+".h");
  SubclassingDlgBase::accept();
}

void SubclassingDlg::onChangedClassName()
//=======================================
{
  m_edFileName->setText(m_edClassName->text().lower());
}
