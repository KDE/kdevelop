/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KDEVPROJECT_H
#define KDEVPROJECT_H

#include "kdevplugin.h"
#include "domutil.h"

#include <qstringlist.h>
#include <qmap.h>

/**
@file kdevproject.h
KDevelop project interface.
*/

class QTimer;

/**
KDevelop project interface.
Plugins implementing the KDevProject interfaces are used to manage projects.

Project can be considered as a way of grouping files (in text editors) or
as a way of providing support for a build system (like it is done in KDevelop IDE buildtools).
*/
class KDevProject: public KDevPlugin
{
    Q_OBJECT
public:
    /**Constructs a project plugin.
    @param info Important information about the plugin - plugin internal and generic
    (GUI) name, description, a list of authors, etc. That information is used to show
    plugin information in various places like "about application" dialog, plugin selector
    dialog, etc. Plugin does not take ownership on info object, also its lifetime should
    be equal to the lifetime of the plugin.
    @param parent The parent object for the plugin. Parent object must implement @ref KDevApi
    interface. Otherwise the plugin will not be constructed.
    @param name The internal name which identifies the plugin.*/
    KDevProject(const KDevPluginInfo *info, QObject *parent=0, const char *name=0);
    /**Destructor.*/
    virtual ~KDevProject();

    /**Options of the project plugin.*/
    enum Options { 
        UsesOtherBuildSystem = 0        /**<Project uses unknown or unspecified build system or build system is not used at all.*/,
        UsesAutotoolsBuildSystem = 1    /**<Project uses autotools for building.*/,
        UsesQMakeBuildSystem =2         /**<Project uses qmake for building.*/
    };

    /**This method is invoked when the project is opened
    (i.e. actually just after this class has been
    instantiated).
    @param dirName The project directory, which should afterwards be returned by
    the projectDirectory() method.
    @param projectName The project name, which is equivalent
    to the project file name without the suffix.*/
    virtual void openProject(const QString &dirName, const QString &projectName);
    
    /**This method is invoked when the project is about to be closed.*/
    virtual void closeProject() = 0;

    /**Reimplement this method to set project plugin options. Default implementation
    returns KDevProject::UsesOtherBuildSystem.*/
    virtual Options options() const;

    /**@return The canonical absolute directory of the project. Canonical means that 
    a path does not contain symbolic links or redundant "." or ".." elements.*/
    virtual QString projectDirectory() const = 0;
    
    /**Returns the name of the project.*/
    virtual QString projectName() const = 0;
    
    /**@return The environment variables that sould be set before running mainProgram().*/
    virtual DomUtil::PairList runEnvironmentVars() const = 0;
    
    /**@return The path to main binary program of the project.
    @param relative if true then the path returned is relative to the project directory.*/
    virtual QString mainProgram(bool relative = false) const = 0;
    
    /**Absolute path (directory) from where the mainProgram() should be run.*/
    virtual QString runDirectory() const = 0;
    
    /**The command line arguments that the mainProgram() should be run with.*/
    virtual QString runArguments() const = 0;
    
    /**Returns the path (relative to the project directory)
    of the active directory. All newly automatically generated 
    classes and files are usually added here.*/
    virtual QString activeDirectory() const = 0;
    
    /**@return The canonical build directory of the project.
    If the separate build directory is not supported, this should 
    return the same as projectDiretory(). Canonical means that 
    a path does not contain symbolic links or redundant "." or ".." elements.*/
    virtual QString buildDirectory() const = 0;
    
    /**@return The list of all files in the project. The names are relative to 
    the project directory.*/
    virtual QStringList allFiles() const = 0;
    
    /**@return The list of files that are part of the distribution but not under 
    project control. Used mainly to package and publish extra files among with the project.*/
    virtual QStringList distFiles() const = 0;
    
    /**Adds a list of files to the project. Provided for convenience when adding many files.
    @param fileList The list of file names relative to the project directory.*/
    virtual void addFiles(const QStringList &fileList) = 0;
    
    /**Adds a file to the project.
    @param fileName The file name relative to the project directory.*/
    virtual void addFile(const QString &fileName)= 0;
    
    /**Removes a list of files from the project. Provided for convenience when removing many files.
    @param fileList The list of file names relative to the project directory.*/
    virtual void removeFiles(const QStringList& fileList)= 0;
    
    /**Removes a file from the project.
    @param fileName The file name relative to the project directory.*/
    virtual void removeFile(const QString &fileName) = 0;
    
    /**Notifies the project about changes to the files. Provided for
    convenience when changing many files.
    @param fileList The list of file names relative to the project directory..*/
    virtual void changedFiles(const QStringList &fileList);
    
    /**Notifies the project of a change to one of the files.
    @param fileName The file name relative to the project directory.*/
    virtual void changedFile(const QString &fileName);
    
    /**@return true if the file @p absFileName is a part of the project.
    @param absFileName Absolute name of a file to check.*/
    virtual bool isProjectFile(const QString &absFileName);
    
    /**@return The path (relative to the project directory) of the file @p absFileName.
    @param absFileName Absolute name of a file.*/
    virtual QString relativeProjectFile(const QString &absFileName);

    /**@return The list of files known to the project through symlinks.*/
    virtual QStringList symlinkProjectFiles();

protected:

    /** Default implementation of runDirectory method.
     *  Uses 'projectPluginName' to obtain configuration from
     *  project DOM and returns:
     *
     *   if /<projectPluginName>/run/directoryradio == executable
     *        The directory where the executable is.
     *   if /<projectPluginName>/run/directoryradio == build
     *        The build directory.
     *   if /kdevautoproject/run/directoryradio == custom
     *        The custom directory absolute path.
     * Derived classes are supposed to explicitly call this implementation
     */
    QString defaultRunDirectory(const QString& projectPluginName) const;
    
private slots:
    void buildFileMap();
    void slotBuildFileMap();
    void slotAddFilesToFileMap(const QStringList & fileList );
    void slotRemoveFilesFromFileMap(const QStringList & fileList );

signals:
    /**Emitted when a new list of files has been added to the
    project. Provided for convenience when many files were added. 
    @param fileList The file names relative to the project directory.*/
    void addedFilesToProject(const QStringList& fileList);
    
    /**Emitted when a list of files has been removed from the project.
    Provided for convenience when many files were removed. 
    @param fileList The file names relative to the project directory.*/
    void removedFilesFromProject(const QStringList& fileList);
    
    /**Emitted when a list of files has changed in the project.
    @param fileList The file names relative to the project directory.*/
    void changedFilesInProject(const QStringList& fileList);

    /**Emitted when one compile related command (make, make install, make ...) ends sucessfuly.
    Used to reparse the files after a sucessful compilation.*/
    void projectCompiled();
    
private:
    class Private;
    Private *d;
};

#endif
