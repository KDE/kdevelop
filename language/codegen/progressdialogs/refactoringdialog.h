/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>     *
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

#ifndef PROGRESSDIALOGS_H
#define PROGRESSDIALOGS_H

#include <QDialog>
#include <language/duchain/topducontext.h>

#include "ui_refactoringdialog.h"

namespace KDevelop
{
class UsesCollector;

class RefactoringProgressDialog : public QDialog
{
    Q_OBJECT
public:
    RefactoringProgressDialog(const QString& action, UsesCollector* collector);

private slots:
    void progress(uint done, uint max);
    void maximumProgress(uint max);
    void processUses(const KDevelop::ReferencedTopDUContext& context);

private:
    UsesCollector* m_collector;
    Ui::RefactoringDialog m_rd;
};

}
#endif
