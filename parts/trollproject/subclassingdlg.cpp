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
  QDomDocument doc;
  DomUtil::openDOMFile(doc,formFile);
  m_baseClassName = DomUtil::elementByPathExt(doc,WIDGET_CLASS_NAME).text();
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
    m_slots << newFunc;
  }

}


SubclassingDlg::~SubclassingDlg()
//==============================================
{
}


void SubclassingDlg::updateDlg()
//==============================
{
}

void SubclassingDlg::accept()
//=========================
{

  /*
  QDomDocument doc;
  DomUtil::openDOMFile(doc,"/home/jsgaarde/programming/kdevelop/domapp/clean_dialog.ui");
  DomUtil::replaceText(doc,WIDGET_CLASS_NAME,"TestClass");
  DomUtil::replaceText(doc,WIDGET_CAPTION_NAME,"Test Dialog");
  QDomElement slotsElem = DomUtil::elementByPathExt(doc,WIDGET_SLOTS);
  QDomNodeList slotnodes = slotsElem.childNodes();
  for (unsigned int i=0; i<slotnodes.count();i++)
  {
    QString msg;
    QDomElement slotelem = slotnodes.item(i).toElement();
    msg.sprintf("Slotname: %s\nReturns: %s\nAccess: %s",
                    slotelem.text().ascii(),
                    slotelem.attributeNode("returnType").value().ascii(),
                    slotelem.attributeNode("access").value().ascii());
    QMessageBox::information(0,"Slots",msg);
  }
  */
  SubclassingDlgBase::accept();
}

