/* This file is part of the KDE project
   Copyright (C) 2005 Ian Reinhart Geiser <geiseri@kde.org>
   
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

#ifndef KDEVPROJECTIFACE_H
#define KDEVPROJECTIFACE_H

#include <dcopobject.h>
#include <qstringlist.h>
#include <QMap>

class KDevProject;

/**
DCOP Interface for the @ref KDevProject object.

@author KDevelop Authors
*/
class KDevProjectIface : public QObject, public DCOPObject
{
  Q_OBJECT
      K_DCOP
public:
  KDevProjectIface(KDevProject *prj);

    ~KDevProjectIface();
k_dcop:
    void openProject(const QString &dirName, const QString &projectName);
    void closeProject();
    uint options() const;
    QString projectDirectory() const;
    QString projectName() const;
    QString mainProgram(bool relative = false) const;
    QString runDirectory() const;
    QString activeDirectory() const;
    QString buildDirectory() const;
    QStringList allFiles() const;
    QStringList distFiles() const;
    void addFiles(const QStringList &fileList);
    void addFile(const QString &fileName);
    void removeFiles(const QStringList& fileList);
    void removeFile(const QString &fileName);
    void changedFiles(const QStringList &fileList);
    void changedFile(const QString &fileName);
    bool isProjectFile(const QString &absFileName);
    QString relativeProjectFile(const QString &absFileName);
    QStringList symlinkProjectFiles();
    
    // Project DOM access
    QString readEntry(const QString &path, const QString &defaultEntry = QString());
    int readIntEntry(const QString &path, int defaultEntry = 0);
    bool readBoolEntry(const QString &path, bool defaultEntry = false);
    QStringList readListEntry(const QString &path, const QString &tag);
    QMap<QString, QString> readMapEntry(const QString &path);
    
    void writeEntry(const QString &path, const QString &value);
    void writeIntEntry(const QString &path, int value);
    void writeBoolEntry( const QString &path, bool value);
    void writeListEntry(const QString &path, const QString &tag, const QStringList &value);
    void writeMapEntry(const QString& path, const QMap<QString,QString> &map);
    
  private:
    KDevProject *m_prj;
};

#endif
