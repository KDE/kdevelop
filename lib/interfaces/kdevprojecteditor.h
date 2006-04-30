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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KDEVPROJECTEDITOR_H
#define KDEVPROJECTEDITOR_H

#include "kdevprojectimporter.h"

typedef QList<KDevProjectFileItem*> FileItemList;


/**
@author Roberto Raggi
*/
class KDEVINTERFACES_EXPORT KDevProjectEditor: public KDevProjectImporter
{
    Q_OBJECT
public:
    enum Features
    {
        Folders = 0x01, ///< Folders are supported by the project
        Targets = 0x02, ///< Targets are supported by the project
        Files   = 0x04, ///< Files are supported by the project

        None = 0x00,    ///< This project supports nothing
        All = Folders | Targets | Files
    };

public:
    KDevProjectEditor(QObject *parent = 0);
    virtual ~KDevProjectEditor();

    virtual KDevProjectEditor *editor() const
    { return const_cast<KDevProjectEditor*>(this); }

    bool hasFeature(Features f) const
    { return (features() & f) == f; }

    virtual Features features() const = 0;

    /**
     * Add a folder to the project
     *
     * Adds the folder specified by @p folder to @p parent and modifies the
     * underlying build system if needed
     */
    virtual bool addFolder(KDevProjectFolderItem *folder, KDevProjectFolderItem *parent) = 0;
    
    /**
     * Add a target to a folder
     * 
     * Adds the target specified by @p target to the folder @p parent and
     * modifies the underlying build system if needed
     */
    virtual bool addTarget(KDevProjectTargetItem *target, KDevProjectFolderItem *parent) = 0;
    
    /**
     * Add a file to a folder
     * 
     * Adds the file specified by @p file to the folder @p parent and modifies
     * the underlying build system if needed. The file is not added to a target
     */
    virtual bool addFile(KDevProjectFileItem *file, KDevProjectFolderItem *parent) = 0;

    /**
     * Add a file to a target
     * 
     * Adds the file specified by @pfile to the target @p parent and modifies
     * the underlying build system if needed.
     */
    virtual bool addFile(KDevProjectFileItem *file, KDevProjectTargetItem *parent) = 0;

    /**
     * Remove a folder from the project
     * 
     * Removes the folder specified by @p folder from folder @p parent and
     * modifies the underlying build system if needed.
     */
    virtual bool removeFolder(KDevProjectFolderItem *folder, KDevProjectFolderItem *parent) = 0;

    /**
     * Remove a target from a folder
     * 
     * Removes the target specified by @p target from the folder @p parent and
     * modifies the underlying build system if needed.
     */
    virtual bool removeTarget(KDevProjectTargetItem *target, KDevProjectFolderItem *parent) = 0;

    /**
     * Remove a file from a folder
     * 
     * Removes the file specified by @p file from the folder @p parent and 
     * modifies the underlying build system if needed. If the file being 
     * removed is also part of a target, it should be removed from the target as well
     */
    virtual bool removeFile(KDevProjectFileItem *file, KDevProjectFolderItem *parent) = 0;

    /**
     * Remove a file from a target
     * 
     * Removes the file specified by @p file from the folder @parent and
     * modifies the underlying build system if needed. The file is not removed
     * from the folder it is in
     */
    virtual bool removeFile(KDevProjectFileItem *file, KDevProjectTargetItem *parent) = 0;

    /**
     * Get a list of all the targets in this project
     *
     * The list returned by this function should be checked to verify it is not
     * empty before using it
     *
     * @return The list of targets for this project
     */
    virtual QList<KDevProjectTargetItem*> targets() const = 0;

    /**
     * Get a list of all the files for a target
     *
     * The list returned by this function should be checked to verify it is not
     * empty before using it.
     *
     * @return The list of files in the target
     */
    virtual FileItemList filesForTarget( KDevProjectTargetItem* target ) const = 0;

};

#endif
//kate: space-indent on; indent-width 4; auto-insert-doxygen on; indent-mode cstyle;
