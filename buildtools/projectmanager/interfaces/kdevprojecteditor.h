/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KDEVPROJECTEDITOR_H
#define KDEVPROJECTEDITOR_H

#include <kdevprojectimporter.h>

/**
@author Roberto Raggi
*/
class KDevProjectEditor: public KDevProjectImporter
{
    Q_OBJECT
public:
    KDevProjectEditor(QObject *parent = 0, const char *name = 0);
    virtual ~KDevProjectEditor();

    virtual KDevProjectEditor *editor() const
    { return const_cast<KDevProjectEditor*>(this); }
    
    virtual ProjectFolderDom addFolder(ProjectFolderDom folder, const QString &name) = 0;
    virtual ProjectTargetDom addTarget(ProjectFolderDom folder, const QString &name) = 0;
    virtual ProjectFileDom addFile(ProjectFolderDom target, const QString &name) = 0;
    virtual ProjectFileDom addFile(ProjectTargetDom target, const QString &name) = 0;
};

#endif
