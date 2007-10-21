/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KDEVIMPORTPROJECTJOB_H
#define KDEVIMPORTPROJECTJOB_H

#include <kjob.h>
// #include "projectmodel.h"
#include <projectexport.h>

class QStandardItem;

namespace ThreadWeaver
{
class Job;
}

namespace KDevelop
{

class IProjectFileManager;

class KDEVPLATFORMPROJECT_EXPORT ImportProjectJob: public KJob
{
    Q_OBJECT
public:
    ImportProjectJob(QStandardItem *folder, IProjectFileManager *importer);
    virtual ~ImportProjectJob();

public:
    void start();

private:
    struct ImportProjectJobPrivate* const d;
};

}
#endif // IMPORTPROJECTJOB_H


