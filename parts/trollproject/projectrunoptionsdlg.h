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

#ifndef _PROJECTRUNOPTIONSDLG_H_
#define _PROJECTRUNOPTIONSDLG_H_

#include "projectrunoptionsdlgbase.h"
#include "trollprojectpart.h"

class ProjectRunOptionsDlg : public ProjectRunOptionsDlgBase
{
    Q_OBJECT

public:
    ProjectRunOptionsDlg( TrollProjectPart *part, QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ProjectRunOptionsDlg();

private:
    TrollProjectPart *m_part;

public slots:
    virtual void browseMainProgram();

};

#endif

