/* 
   Copyright (C) 2003 ian reinhart geiser <geiseri@kde.org> 

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KJSPROBLEMS_H
#define KJSPROBLEMS_H

#include <klistview.h>
class kjsSupportPart;

/**
@author ian reinhart geiser
*/
class KJSProblems : public KListView
{
Q_OBJECT
public:

    KJSProblems(kjsSupportPart *part, QWidget *parent = 0L, const char *name = 0L);
    ~KJSProblems();

    void clearItems();
    void addLine(const QString &file, int lineNo, const QString &message);

private:
    kjsSupportPart *m_part;
};

#endif
