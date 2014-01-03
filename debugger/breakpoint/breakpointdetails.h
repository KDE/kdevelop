/*
 * This file is part of KDevelop
 *
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_BREAKPOINTDETAILS_H
#define KDEVPLATFORM_BREAKPOINTDETAILS_H

#include <QtGui/QWidget>
#include "../debuggerexport.h"

class QSpinBox;
class QLabel;

namespace KDevelop {
class Breakpoint;

class KDEVPLATFORMDEBUGGER_EXPORT BreakpointDetails : public QWidget
{
    Q_OBJECT
public:
    BreakpointDetails(QWidget *parent);

    void setItem(Breakpoint *b);

private Q_SLOTS:
    void showExplanation(const QString& link);

    void setIgnoreHits(int ignoreHits);

private:
    QLabel* status_;
    QLabel* hits_;
    QSpinBox* ignore_;
    Breakpoint* m_currentBreakpoint;
};


}

#endif // KDEVPLATFORM_BREAKPOINTDETAILS_H
