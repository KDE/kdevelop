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
#include <kpushbutton.h>
#include <domutil.h>
#include <qdom.h>
#include <kstdguiitem.h>

#define WIDGET_CAPTION_NAME "widget/property|name=caption/string"
#define WIDGET_CLASS_NAME   "class"
#define WIDGET_SLOTS        "slots"

NewWidgetDlg::NewWidgetDlg(QStringList &newFileNames,QWidget* parent, const char* name, bool modal, WFlags fl)
: NewWidgetDlgBase(parent,name,modal,fl),
m_newFileNames(newFileNames)
//=================================================
{
// Remove in kde 3.4 support
	okayButton->setGuiItem(KStdGuiItem::ok());
	cancelButton->setGuiItem(KStdGuiItem::cancel());
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

void NewWidgetDlg::accept()
//=========================
{
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
  DomUtil::saveDOMFile(doc,"/home/jsgaarde/programming/kdevelop/domapp/clean_dialog2.ui");
  NewWidgetDlgBase::accept();
}

