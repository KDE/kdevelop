/***************************************************************************
                          toolscheckpage.cpp  -  description
                             -------------------
    begin                : Sat Jun 2 2001
    copyright            : (C) 2001 by Falk Brettschneider
    email                : falk.brettschneider@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qlistbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qvbox.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include "../ctoolclass.h"
#include "ccreatedocdatabasedlg.h"
#include "ckdevinstallstate.h"
#include "wizardtoolscheckpage.h"

WizardToolsCheckPage::WizardToolsCheckPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState)
: WizardBasePage(parent, name, infoText, installPictPathAndFilename, pInstallState)
{
  m_vbox = new QVBox(this);
  new QLabel(i18n("These tools have been detected on your computer:")+"\n", m_vbox);
  QListBox* lb = new QListBox(m_vbox);
  searchForAllTools(lb);
}

void WizardToolsCheckPage::searchForAllTools(QListBox* lb)
{
  bool autoconf     = CToolClass::searchInstProgram("autoconf");
  bool autoheader   = CToolClass::searchInstProgram("autoheader");
  bool automake     = CToolClass::searchInstProgram("automake");
  bool perl         = CToolClass::searchInstProgram("perl");
  bool sgml2html    = CToolClass::searchInstProgram("sgml2html");
// bool ksgml2html   = CToolClass::searchInstProgram("ksgml2html");
  bool kdoc         = CToolClass::searchInstProgram("kdoc");
  bool a2ps         = CToolClass::searchInstProgram("a2ps");
  bool enscript     = CToolClass::searchInstProgram("enscript");
  bool dbg          = CToolClass::searchInstProgram("kdbg");
  bool kiconedit    = CToolClass::searchInstProgram("kiconedit");
  bool ark          = CToolClass::searchInstProgram("ark");
//  bool ktranslator  = CToolClass::searchInstProgram("ktranslator");
  bool kbabel       = CToolClass::searchInstProgram("kbabel");
  bool gimp         = CToolClass::searchInstProgram("gimp");
  bool kpaint       = CToolClass::searchInstProgram("kpaint");
  bool designer     = CToolClass::searchInstProgram("designer");
  bool linguist     = CToolClass::searchInstProgram("linguist");
  bool ctags        = CToolClass::searchInstProgram("ctags");

  m_pInstallState->make	 = CToolClass::searchInstProgram("make");
  m_pInstallState->gmake = CToolClass::searchInstProgram("gmake");

  m_pInstallState->glimpse      = CToolClass::searchInstProgram("glimpse");
  m_pInstallState->glimpseindex = CToolClass::searchInstProgram("glimpseindex");
  m_pInstallState->htdig        = CToolClass::searchInstProgram("htdig");
  m_pInstallState->htsearch     = CToolClass::searchInstProgram("htsearch");
  // Support htsearch in /usr/lib/cgi-bin
  if(!(m_pInstallState->htsearch))
    m_pInstallState->htsearch   = CToolClass::searchInstProgram("/usr/lib/cgi-bin/htsearch");

  ToolAppList toolList;

  QString found=i18n(" was found.");
  QString not_found=i18n(" was not found.");

  QString make_str;
  if(m_pInstallState->make){
    make_str="make"+found+"\n";
  }
  else
    make_str="make"+not_found+"\n";

  QString gmake_str;
  if(m_pInstallState->gmake){
    gmake_str="gmake"+found+i18n(" -- setting make-command to gmake\n");
  }
  else
    gmake_str="gmake"+not_found+"\n";

  QString autoconf_str;
  if(autoconf)
    autoconf_str="autoconf"+found+"\n";
  else
    autoconf_str="autoconf"+not_found+"\n";

  QString autoheader_str;
  if(autoheader)
    autoheader_str="autoheader"+found+"\n";
  else
    autoheader_str="autoheader"+not_found+"\n";
  QString automake_str;
  if(automake)
    automake_str="automake"+found+"\n";
  else
    automake_str="automake"+not_found+"\n";

  QString sgml2html_str;
  if(sgml2html)
    sgml2html_str="sgml2html"+found+"\n";
  else
    sgml2html_str="sgml2html"+not_found+i18n(" -- generating application handbooks will not be possible\n");
  QString kdoc_str;
  if(kdoc)
    kdoc_str="kdoc"+found+"\n";
  else
    kdoc_str="kdoc"+not_found+i18n(" -- generating API-documentations will not be possible\n");

  QString dbg_str;
  if(dbg){
    dbg_str="kdbg"+found+"\n";
    toolList.append(CToolApp("K&Dbg", "kdbg"));
  }
  else
    dbg_str="kdbg"+not_found;

  QString kiconedit_str;
  if(kiconedit){
    kiconedit_str="KIconedit"+found+"\n";
    toolList.append(CToolApp("K&IconEdit", "kiconedit"));
  }
  else
    kiconedit_str="KIconedit"+not_found+i18n(" -- editing icons will not be possible\n");

  QString ark_str;
  if(ark){
    ark_str="Archiever (ark)"+found+"\n";
    toolList.append(CToolApp("&Ark", "ark"));
  }
  else
    ark_str="Archiever (ark)"+not_found+i18n(" -- viewing compressed files will not be possible\n");

  QString kpaint_str;
  if(gimp){
    kpaint_str="GIMP"+found+"\n";
    toolList.append(CToolApp("&GIMP", "gimp"));
  }
  else if(kpaint){
      kpaint_str="KPaint"+found+"\n";
      toolList.append(CToolApp("K&Paint", "kpaint"));
  }
  else
    kpaint_str=i18n("GIMP/KPaint ")+not_found+"\n";

  QString designer_str;
  if(designer){
    designer_str="Qt Designer"+found+"\n";
    toolList.append(CToolApp("&Qt Designer", "designer", " -client "));
  }
  else
    designer_str=i18n("Qt Designer")+not_found+"\n";

  QString linguist_str;
  if(linguist){
    linguist_str="Qt Linguist"+found+"\n";
    toolList.append(CToolApp("&Qt linguist", "linguist"));
  }
  else
    linguist_str=i18n("Qt Linguist")+not_found+"\n";

  QString ctags_str;
  if(ctags){
    ctags_str="ctags"+found+"\n";
  }
  else
    ctags_str=i18n("ctags")+not_found
             +i18n(" -- browse functionality will not be provided")+"\n";

  QString kbabel_str;
  if(kbabel){
    kbabel_str="KBabel"+found+"\n";
    toolList.append(CToolApp("K&Babel", "kbabel"));
  }
  else
    kbabel_str="KBabel"+not_found+"\n";

  QString glimpse_str;
  if(m_pInstallState->glimpse)
    glimpse_str="Glimpse"+found+"\n";
  else
    glimpse_str="Glimpse"+not_found; //+ i18n(" -- search functionality will not be provided\n");
  QString glimpseindex_str;
  if(m_pInstallState->glimpseindex)
    glimpseindex_str="Glimpseindex"+found+"\n";
  else
    glimpseindex_str="Glimpseindex"+not_found; //+ i18n(" -- search functionality will not be provided\n");
  QString htdig_str;
  if(m_pInstallState->htdig)
    htdig_str="htdig"+found+"\n";
  else
    htdig_str="htdig"+not_found; //+ i18n(" -- search functionality will not be provided\n");
  QString htsearch_str;
  if(m_pInstallState->htsearch)
    htsearch_str="htsearch"+found+"\n";
  else
    htsearch_str="htsearch"+not_found; //+ i18n(" -- search functionality will not be provided\n");
  if (!m_pInstallState->glimpse || !m_pInstallState->glimpseindex)
  {
    if (!m_pInstallState->htsearch)
    {
      htsearch_str += i18n(" -- search functionality will not be provided\n");
      if (!m_pInstallState->htdig)
        htdig_str += "\n";
    }
    else if (!m_pInstallState->htdig)
    {
      htdig_str += i18n(" -- search functionality will not be provided\n");
      if (!m_pInstallState->htsearch)
        htsearch_str += "\n";
    }
    if (!m_pInstallState->glimpse)
      glimpse_str += "\n";
    if (!m_pInstallState->glimpseindex)
      glimpseindex_str += "\n";
  }
  else
  {
    if (!m_pInstallState->htdig)
      htdig_str += "\n";
    if (!m_pInstallState->htsearch)
      htsearch_str += "\n";
  }
  QString perl_str;
  if(perl)
    perl_str="Perl"+found+"\n";
  else
    perl_str="Perl"+not_found+ i18n(" -- generation of new frame applications will not be possible\n");
  QString print_str;
  if(a2ps || enscript)
    print_str="a2ps / enscript"+found+ i18n("  -- printing available\n");
  else
    print_str="a2ps / enscript"+not_found+ i18n(" -- printing can only use lpr\n");


  lb->insertItem(make_str);
  lb->insertItem(gmake_str);
  lb->insertItem(autoconf_str);
  lb->insertItem(autoheader_str);
  lb->insertItem(automake_str);
  lb->insertItem(perl_str);
  lb->insertItem(sgml2html_str);
  lb->insertItem(kdoc_str);
  lb->insertItem(glimpse_str);
  lb->insertItem(glimpseindex_str);
  lb->insertItem(htdig_str);
  lb->insertItem(htsearch_str);
  lb->insertItem(print_str);
  lb->insertItem(dbg_str);
  lb->insertItem(kiconedit_str);
  lb->insertItem(ark_str);
  lb->insertItem(kpaint_str);
  lb->insertItem(kbabel_str);
  lb->insertItem(linguist_str);
  lb->insertItem(ctags_str);
  lb->insertItem(designer_str);

  CToolClass::writeToolConfig(toolList);
}
