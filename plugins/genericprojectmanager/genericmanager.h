/* This file is part of KDevelop
    CopyRight 2010 Milian Wolff <mail@milianw.de>
    Copyright 2004,2005 Roberto Raggi <roberto@kdevelop.org>

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
#ifndef KDEVPLATFORM_PLUGIN_GENERICMANAGER_H
#define KDEVPLATFORM_PLUGIN_GENERICMANAGER_H

#include <project/abstractfilemanagerplugin.h>

class GenericProjectManager: public KDevelop::AbstractFileManagerPlugin
{
    Q_OBJECT

public:
    explicit GenericProjectManager( QObject* parent = 0, const QVariantList& args = QVariantList() );

    virtual KJob* createImportJob( KDevelop::ProjectFolderItem* item );
    virtual bool reload( KDevelop::ProjectFolderItem* item );

protected:
    virtual bool isValid( const KDevelop::Path& path, const bool isFolder, KDevelop::IProject* project ) const;

private:
    typedef QPair<QStringList, QStringList> IncludeRules;
    void updateIncludeRules( KDevelop::IProject* project );
    QMap< KDevelop::IProject*, IncludeRules > m_includeRules;
};

#endif // KDEVPLATFORM_PLUGIN_GENERICIMPORTER_H
