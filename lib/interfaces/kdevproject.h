/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

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
#ifndef _KDEVPROJECT_H_
#define _KDEVPROJECT_H_

#include "kdevplugin.h"
#include "domutil.h"

#include <qstringlist.h>
#include <qmap.h>

class QTimer;

/**
KDevelop project interface.
*/
class KDevProject : public KDevPlugin
{
    Q_OBJECT

public:
    KDevProject(const KDevPluginInfo *info,
        QObject *parent=0, const char *name=0 );
    virtual ~KDevProject();

    enum Options { UsesAutotoolsBuildSystem = 1, UsesQMakeBuildSystem };

    /**
    * This method is invoked when the project is opened
    * (i.e. actually just after this class has been
    * instantiated). The first parameter is the project
    * directory, which should afterwards be returned by
    * the projectDirectory() method. The second parameter
    * is the project name, which is equivalent with the
    * project file name without the .kdevelop suffix.
    */
    virtual void openProject(const QString &dirName, const QString &projectName);
    /**
    * This method is invoked when the project is about
    * to be closed.
    */
    virtual void closeProject() = 0;

    virtual Options options() const;

    /**
    * Returns the canonical toplevel directory of the project.
    */
    virtual QString projectDirectory() const = 0;
    /**
    * Returns the name of the project.
    */
    virtual QString projectName() const = 0;
    /**
    * The environment variables that sould be set before running mainProgram()
    */
    virtual DomUtil::PairList runEnvironmentVars() const = 0;
    /**
    * Returns the absolute path to main binary program of the project.
    */
    virtual QString mainProgram(bool relative = false) const = 0;
    /**
    * Absolute path (directory) from where the mainProgram() should be run
    */
    virtual QString runDirectory() const = 0;
    /**
    * The command line arguments that the mainProgram() should be run with
    */
    virtual QString runArguments() const = 0;
    /**
    * Returns the path (relative to the project directory)
    * of the active directory. All newly generated classes
    * are added here.
    */
    virtual QString activeDirectory() const = 0;
    /**
    * Returns the canonical build directory of the project.
    * If the separate build directory is not supported, this should
    * return the same as projectDiretory().
    */
    virtual QString buildDirectory() const = 0;
    /**
    * Returns a list of all files in the project.
    * The files are relative to the project directory.
    */
    virtual QStringList allFiles() const = 0;
    
    /**
    * Returns a list of files that are part of the distribution
    * but not under project control.
    */
    virtual QStringList distFiles() const = 0;
    
    /**
    * Adds a list of files to the project. Provided for convenience when adding many files.
    * The given file names must be relative to the project directory.
    */
    virtual void addFiles(const QStringList &fileList ) = 0;
    /**
    * Adds a file to the project. The given file name
    * must be relative to the project directory.
    */
    virtual void addFile(const QString &fileName)= 0;
    /**
    * Removes a list of files from the project. Provided for convenience when removing many files.
    * The given file names must be relative to the project directory.
    */
    virtual void removeFiles ( const QStringList& fileList )= 0;
    /**
    * Removes a file from the project. The given file name
    * must be relative to the project directory.
    */
    virtual void removeFile(const QString &fileName) = 0;
    /**
    * Notifies the project of changes to the files. Provided for convenience when changing many files.
    * The given file names must be relative to the project directory.
    */
    virtual void changedFiles( const QStringList & fileList );
    /**
    * Notifies the project of a change to one of the files. The given file name
    * must be relative to the project directory.
    */
    virtual void changedFile( const QString & fileName );
    /**
    * Returns true if the file absFileName is part of the project
    */
    virtual bool isProjectFile( const QString & absFileName );
    /**
    * Returns the path (relative to the project directory)
    * of the file absFileName.
    */
    virtual QString relativeProjectFile( const QString & absFileName );

    /**
     * @return a list of files know to the project through symlinks
     */
    virtual QStringList symlinkProjectFiles();
    
private slots:
    void buildFileMap();
    void slotBuildFileMap();
    void slotAddFilesToFileMap(const QStringList & fileList );
    void slotRemoveFilesFromFileMap(const QStringList & fileList );

signals:
    /**
    * Emitted when a new file has been added to the
    * project. The fileName is relative to the project directory.
    */
    //void addedFileToProject(const QString &fileName);
    /**
    * Emitted when a new list of files has been added to the
    * project. Provided for convenience when many files were added. The file names are relative to the project directory.
    */
    void addedFilesToProject( const QStringList& fileList );
    /**
    * Emitted when a file has been removed from the
    * project. The fileName is relative to the project directory.
    */
    //void removedFileFromProject(const QString &fileName);
    /**
    * Emitted when a list of files has been removed from the project.
    * Provided for convenience when many files were removed. The file names are relative to the project directory.
    */
    void removedFilesFromProject(const QStringList& fileList );
    /**
    * Emitted when a list of files has changed in the project.
    * The file names are relative to the project directory.
    */
    void changedFilesInProject(const QStringList& fileList );

    /**
    * Emitted when one compile related command (make, make install, make ...) ends sucessfuly.
    * Used to reparse the files after a sucessful compilation
    */
    void projectCompiled();
    
private:
    QMap<QString, QString> m_absToRel;
    QStringList m_symlinkList;
    QTimer *m_timer;
};

#endif
