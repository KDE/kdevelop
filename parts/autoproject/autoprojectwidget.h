/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _AUTOPROJECTWIDGET_H_
#define _AUTOPROJECTWIDGET_H_

#include <qdict.h>
#include <qlistview.h>
#include <qmap.h>
#include <qstrlist.h>
#include <qvbox.h>
#include <kiconloader.h>
#include <klocale.h>

class AutoProjectPart;
class KListView;
class SubprojectItem;
class TargetItem;
class FileItem;


/**
 * Base class for all items appearing in ProjectOverview and ProjectDetails.
 */
class ProjectItem : public QListViewItem
{
public:
    enum Type { Subproject, Target, File };
    
    ProjectItem(Type type, QListView *parent, const QString &text);
    ProjectItem(Type type, ProjectItem *parent, const QString &text);

    void paintCell(QPainter *p, const QColorGroup &cg,
                   int column, int width, int alignment);
    void setBold(bool b)
    { bld = b; }
    bool isBold() const
    { return bld; }
    Type type()
    { return typ; }
    
private:
    Type typ;
    bool bld;
    //void init();
};


/**
 * Stores the content of one Makefile.am
 */
class SubprojectItem : public ProjectItem
{
public:
    SubprojectItem(QListView *parent, const QString &text);
    SubprojectItem(SubprojectItem *parent, const QString &text);

    QString subdir;
    QString path;
    QMap<QCString, QCString> prefixes;
    QMap<QCString, QCString> variables;
    QList<TargetItem> targets;

private:
    void init();
    bool bld;
};


/**
 * Stores one target
 * For e.g. the line
 *    bin_LTLIBRARIES = foo.la
 * generates a target with name 'foo.la', primary LTLIBRARIES and prefix 'bin'
 * In order to make things not too simple ;-) headers and data are handled
 * a bit different from programs, libraries and scripts: All headers for a
 * certain prefix (analogously for data) are put in _one_ TargetItem object,
 * and the names of the files are put in the sources variable. This avoids
 * cluttering the list view with lots of header items.
 */
class TargetItem : public ProjectItem
{
public:
    TargetItem(QListView *lv, bool group, const QString &text);
    
    // Name of target, e.g. foo
    QCString name;
    // One of PROGRAMS, LIBRARIES, LTLIBRARIES, SCRIPTS, HEADERS, DATA, JAVA
    QCString primary;
    // May be bin, pkglib, noinst, check, sbin, pkgdata, java...
    QCString prefix;
    // Content of foo_SOURCES (or java_JAVA) assignment
    QList<FileItem> sources;
    // Content of foo_LDFLAGS assignment
    QCString ldflags;
    // Content of foo_LDADD assignment
    QCString ldadd;
    // Content of foo_LIBADD assignment
    QCString libadd;
    // Content of foo_DEPENDENCIES assignment
    QCString dependencies;
};


// Not sure if this complexity is really necessary...
class FileItem : public ProjectItem
{
public:
    FileItem(QListView *lv, const QString &text);

    QString name;
};


class AutoProjectWidget : public QVBox
{
    Q_OBJECT
    
public: 
    AutoProjectWidget(AutoProjectPart *part, bool kde);
    ~AutoProjectWidget();

    void openProject(const QString &dirName);
    void closeProject();

    /**
     * A list of the (relative) names of all subprojects (== subdirectories)
     */
    QStringList allSubprojects();
    /**
     * A list of the (relative) names of all libraries
     */
    QStringList allLibraries();
    /**
     * A list of all files that belong to the project
     **/
    QStringList allSourceFiles();
    /**
     * The top level directory of the project.
     **/
    QString projectDirectory();
    /**
     * The directory of the currently shown subproject.
     */
    QString subprojectDirectory();
    /**
     * The build directory.
     */
    QString buildDirectory();
    /**
     * Are we in KDE mode?
     */
    bool kdeMode() const
    { return m_kdeMode; }

    /**
     * Sets the given target active. The argument is given
     * relative to the project directory.
     */
    void setActiveTarget(const QString &targetPath);
    /**
     * Returns the active target as path relative to
     * the project directory.
     */
    QString activeDirectory();

    /**
     * Adds a file to the active target. The argument must
     * not contain / characters.
     */
    void addFile(const QString &name);
    /**
     * Removes the file fileName from the directory directory.
     * (not implemented currently)
     */
    void removeFile(const QString &fileName);
    
    TargetItem *createTargetItem(const QCString &name,
                                 const QCString &prefix, const QCString &primary);
    FileItem *createFileItem(const QString &name);

    void emitAddedFile(const QString &name);
    void emitRemovedFile(const QString &name);
    
private slots:
    void slotItemExecuted(QListViewItem *item);
    void slotContextMenu(KListView *, QListViewItem *item, const QPoint &p);

private:
    void parsePrimary(SubprojectItem *item, QCString lhs, QCString rhs);
    void parseSubdirs(SubprojectItem *item, QCString lhs, QCString rhs);
    void parsePrefix(SubprojectItem *item, QCString lhs, QCString rhs);
    void parse(SubprojectItem *item);

    KListView *overview;
    KListView *details;
    bool m_kdeMode;
    AutoProjectPart *m_part;
    SubprojectItem *m_shownSubproject;
    SubprojectItem *m_activeSubproject;
    TargetItem * m_activeTarget;
};

#endif
