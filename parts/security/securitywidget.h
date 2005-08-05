/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef SECURITYWIDGET_H
#define SECURITYWIDGET_H

#include <klistview.h>

class SecurityPart;

class SecurityWidget: public KListView {
    Q_OBJECT
public:
    SecurityWidget(SecurityPart* part, const char* name = 0);

    void clearResultsForFile(const QString &file);
    
    void reportWarning(const QString &file, int line, const QString &warning, const QString &suggestion);
    void reportProblem(const QString &file, int line, const QString &problem, const QString &suggestion);
    
public slots:
    void gotoLine(QListViewItem *item);
    
private:
    SecurityPart *m_part;
};

#endif
