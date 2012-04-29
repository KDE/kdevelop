/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef CTESTOUTPUTMODEL_H
#define CTESTOUTPUTMODEL_H

#include <outputview/outputmodel.h>
#include <outputview/ioutputviewmodel.h>

class CTestSuite;

class CTestOutputModel : public KDevelop::OutputModel, public KDevelop::IOutputViewModel
{
    Q_OBJECT
public:
    CTestOutputModel(CTestSuite* suite, QObject* parent = 0);
    virtual ~CTestOutputModel();
    
    virtual QModelIndex previousHighlightIndex(const QModelIndex& currentIndex);
    virtual QModelIndex nextHighlightIndex(const QModelIndex& currentIndex);
    virtual void activate(const QModelIndex& index);
    
private:
    CTestSuite* m_suite;
    bool isHighlighted(const QModelIndex& index);
};

#endif // CTESTOUTPUTMODEL_H
