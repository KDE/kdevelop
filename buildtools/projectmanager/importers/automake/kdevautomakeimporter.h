/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2002 by Victor R�er <victor_roeder@gmx.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KDEVAUTOMAKEIMPORTER_H
#define KDEVAUTOMAKEIMPORTER_H

#include "automakeprojectmodel.h"
#include <kdevprojecteditor.h>

class KDevAutomakeImporter: public KDevProjectEditor
{
    Q_OBJECT
public:
    typedef QMap<QString, QVariant> Environment;
    
public:
    KDevAutomakeImporter(QObject *parent = 0, const char *name = 0, const QStringList &args = QStringList());
    virtual ~KDevAutomakeImporter();
    
//
// KDevProjectImporter inerface
//
    virtual KDevProject *project() const;
    
    virtual ProjectItemDom import(ProjectFolderDom dom, const QString &fileName);
    virtual QString findMakefile(ProjectFolderDom dom) const;
    virtual QStringList findMakefiles(ProjectFolderDom dom) const;
  
//
// KDevProjectEditor interface
//  
    virtual ProjectFolderDom addFolder(ProjectFolderDom folder, const QString &name);
    virtual ProjectTargetDom addTarget(ProjectFolderDom folder, const QString &name);
    virtual ProjectFileDom addFile(ProjectFolderDom folder, const QString &name);
    virtual ProjectFileDom addFile(ProjectTargetDom target, const QString &name);
    
private:    
    static void setup(AutomakeTargetDom dom, const QString &name, const QString &prefix, const QString &primary);
    static QString nicePrimary(const QString &primary);
    static void parseMakefile(const QString &fileName, ProjectItemDom dom);
    static void saveMakefile(const QString &fileName, ProjectItemDom dom);    
    static void modifyMakefile(const QString &fileName, const Environment &env);
    static void removeFromMakefile(const QString &fileName, const Environment &env);
    static QString canonicalize(const QString &str);
    
    QStringList findMakefiles(ProjectFolderDom dom);
    AutomakeTargetDom findNoinstHeaders(ProjectFolderDom item);
    
    void parse(ProjectFolderDom item);    
    void parseKDEDOCS(ProjectItemDom item, const QString &lhs, const QString &rhs);
    void parseKDEICON(ProjectItemDom item, const QString &lhs, const QString &rhs);
    void parsePrimary(ProjectItemDom item, const QString &lhs, const QString &rhs);
    void parsePrefix(ProjectItemDom item, const QString &lhs, const QString &rhs);
    void parseSUBDIRS(ProjectItemDom item, const QString &lhs, const QString &rhs);
    
private:
    KDevProject *m_project;
    QStringList headers; // ### remove me!!
};

#endif // KDEVAUTOMAKEIMPORTER_H
