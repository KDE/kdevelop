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
#ifndef KDEVPROJECTIMPORTER_H
#define KDEVPROJECTIMPORTER_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

class KDevProjectEditor;
class KDevProjectModel;
class KDevProject;
class KDevProjectItem;
class KDevProjectFolderItem;
class KDevProjectTargetItem;
class KDevProjectFileItem;
class KDialogBase;

/**
@author Roberto Raggi

@short Base class for the KDevProjectManager importers

KDevProjectImporter is the class you want to implement for integrating
a project manager in KDevelop.
*/
class KDevProjectImporter: public QObject
{
    Q_OBJECT
public:
    KDevProjectImporter(QObject *parent = 0);
    virtual ~KDevProjectImporter();

    /** @return The current project. */
    virtual KDevProject *project() const = 0;

    /** @return The instance of the editor interface. */
    virtual KDevProjectEditor *editor() const
    { return 0; }

    /** This method initialize the model item @arg dom
        @return The list of the sub folders
     */
    virtual QList<KDevProjectFolderItem*> parse(KDevProjectFolderItem *dom) = 0;

    /** This method creates the root item from the file @arg fileName
        @return The created item
     */
    virtual KDevProjectItem *import(KDevProjectModel *model, const QString &fileName) = 0;

    /** @return The Makefile associated to the item model @p dom.
        @note The Makefile list must contain absolute file names

        For instance, in <b>Automake</b> projects you can use something like:

        @code
        return dom->name() + "/Makefile.am";
        @endcode
      */
    virtual QString findMakefile(KDevProjectFolderItem *dom) const = 0;

    /** @return The list of the Makefiles from the item model @p dom.
        @note The Makefile list must contains absolute file names */
    virtual QStringList findMakefiles(KDevProjectFolderItem *dom) const = 0;

signals:
    void projectItemConfigWidget(const QList<KDevProjectItem*> &dom, KDialogBase *dialog);
};

#endif
