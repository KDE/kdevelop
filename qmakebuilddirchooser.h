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

#ifndef QMAKEBUILDDIRCHOOSER_H
#define QMAKEBUILDDIRCHOOSER_H

#include <KDialog>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/abstractfilemanagerplugin.h>
#include <interfaces/iproject.h>
#include "ui_qmakebuilddirchooser.h"

namespace Ui 
{
    class QMakeBuildDirChooser;
}

class QMakeBuildDirChooser : public KDialog, private Ui::QMakeBuildDirChooser
{
    Q_OBJECT
    
public:
    explicit QMakeBuildDirChooser(KDevelop::IProject* project, QWidget *parent = 0);
    virtual ~QMakeBuildDirChooser();

private:
    bool isValid();
    
    KDevelop::IProject* m_project;

protected slots:
    virtual void slotButtonClicked(int button);
    
private slots:
    void validate();
    void saveConfig();
};

#endif
