/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2011 Martin Heide <martin.heide@gmx.net>                *
 *   Copyright (C) 2011 Julien Desgats <julien.desgats@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef QMAKEBUILDDIRCHOOSERDIALOG_H
#define QMAKEBUILDDIRCHOOSERDIALOG_H

#include "qmakebuilddirchooser.h"
#include <KDialog>

namespace KDevelop {
    class IProject;
}

class QMakeBuildDirChooserDialog : public KDialog, private QMakeBuildDirChooser
{
    Q_OBJECT
    
public:
    explicit QMakeBuildDirChooserDialog(KDevelop::IProject* project, QWidget *parent = 0);
    virtual ~QMakeBuildDirChooserDialog();
    
protected slots:
    virtual void slotButtonClicked(int button);
    void validate();
};

#endif
