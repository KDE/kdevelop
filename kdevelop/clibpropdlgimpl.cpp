/***************************************************************************
                          clibpropdlgimpl.cpp  -  description
                             -------------------
    begin                : Fri Mar 16 2001
    copyright            : (C) 2001 by kdevelop-team
    email                : kdevelop-team@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "clibpropdlgimpl.h"
#include "clibpropdlgimpl.moc"
#include "cproject.h"

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>

CLibPropDlgImpl::CLibPropDlgImpl(TMakefileAmInfo* info, QWidget *parent, const char *name ) :
  CLibPropDlg(parent,name, true),
  m_info(info)
{
  bool sharedLib = (m_info->type == "shared_library");

  enableShared->setChecked(sharedLib);
  enableStatic->setChecked(!sharedLib);

  libNameEdit->setText(m_info->sharedlibRootName);
  LDFlagsEdit->setText(m_info->sharedlibLDFLAGS);
  GroupBox1->setEnabled(sharedLib);

  connect(rbGroup_LibType, SIGNAL(clicked(int)), this, SLOT(slotLibChanged(int)));
}

CLibPropDlgImpl::~CLibPropDlgImpl()
{
}

// OK has been pressed so update the callers structure.
void CLibPropDlgImpl::accept()
{
  if (enableShared->isChecked())
    m_info->type = "shared_library";
  else
    m_info->type = "static_library";

  m_info->sharedlibRootName = libNameEdit->text();
  m_info->sharedlibLDFLAGS = LDFlagsEdit->text();

  CLibPropDlg::accept();
}

#define SHARED_LIB_ID 0
#define STATIC_LIB_ID 1

// When the user wants a static library there are no details needed
// so we disable the shared lib detail fields
void CLibPropDlgImpl::slotLibChanged(int rbId)
{
  GroupBox1->setEnabled(rbId == SHARED_LIB_ID);
}
