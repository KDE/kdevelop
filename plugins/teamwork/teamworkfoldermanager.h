/***************************************************************************
  copyright            : (C) 2006 by David Nolden
  email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TEAMWORKFOLDERMANAGER_H
#define TEAMWORKFOLDERMANAGER_H

#include <kurl.h>
#include <QString>
#include <QMap>

/**Since KDevelop now is no single-project application any more,
 * a notion of a workspace is necessary so teamwork is possible across multiple projects.
 * The workspace-directory is the directory where the .teamwork-subfolder with all the teamwork-data
 * will be created, and file-collaboration is possible on subfolders of that workspace.
 * Of course the workspace-directory could be a simple project-directory.
 * This class manages the .teamwork-subfolder of the workspace.
 * */
class TeamworkFolderManager {
  public:
    TeamworkFolderManager();
    ~TeamworkFolderManager();
    /**
     * This function creates an empty file.
     * This function throws QString on errors.
     * @param subFolder can be a whole sub-tree which will be created if it doesn't exist.
     * @param extension the extension of the file to create, without dot(example "cpp")
     * @param name name of the file. If a file with that name already exists, it will be prefixed to be unique.
     * @return the absolute URL of the file
     * */
    static QString createUniqueFile( QString subFolder, QString extension, QString name, QString namePrefix, QString nameSuffix ) throw(QString);

    static QString createUniqueDirectory( QString subFolder, QString name, QString namePrefix = "", QString nameSuffix = "" ) throw(QString);
    
    /** This takes a complete reference-filename which may include folders(those will be created), and from which extension and name will be extracted.(see function above)
     * */
    static QString createUniqueFile( QString subFolder, QString fileName, QString namePrefix = "", QString nameSuffix = "" ) throw(QString);

    /**
     * This may be called with a file/folder. On destruction of the class(close of the workspace), that folder will automatically be deleted.
     * @param url Complete url of the file/folder to be deleted on destruction. It must be a subfolder of the .teamwork-folder.
     * */
    static void registerTempItem( const QString& file );

    /**
     * @param subfolder a subfolder of .teamwork that the url will be relative to
     * @return the URL relative to the .teamwork-directory
     * */
    static QString relative( const QString& path, const QString& subfolder = QString() );
    
    /**
     * @param subfolder a subfolder of workspace-dir that the url will be relative to
     * @return the URL relative to the workspace-directory
     * */
    static QString workspaceRelative( const QString& path, const QString& subfolder = QString() );
    
    /**
     * @param subFolder an additional subFolder of workspace-dir that the sub-url is in
     * @return the absolute directory of the given sub-directory
     * */
    static QString workspaceAbsolute( const QString& subPath, const QString& subFolder = QString() );
    
    /**
     * @param subFolder an additional subFolder of .teamwork that the sub-url is in
     * @return the absolute directory of the given sub-directory
     * */
    static QString absolute( const QString& subPath, const QString& subFolder = QString() );

    static QString workspaceDirectory();
    
    static TeamworkFolderManager* self();

  private:
    static TeamworkFolderManager* m_self;

    QString m_teamworkDir;
    QString m_workspaceDir;
    
    QMap<QString, bool> m_tempItems;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
