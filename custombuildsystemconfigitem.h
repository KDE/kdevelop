/************************************************************************
* KDevelop4 Custom Buildsystem Support                                 *
*                                                                      *
* Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
*                                                                      *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 3 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* This program is distributed in the hope that it will be useful, but  *
* WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
* General Public License for more details.                             *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program; if not, see <http://www.gnu.org/licenses/>. *
************************************************************************/

#ifndef CUSTOMBUILDSYSTEMCONFIGITEM_H
#define CUSTOMBUILDSYSTEMCONFIGITEM_H

#include <project/projectmodel.h>
#include <QObject>

class CustomBuildSystemConfigItem : public QObject, public KDevelop::ProjectBaseItem
{
Q_OBJECT
public:
    CustomBuildSystemConfigItem( KDevelop::ProjectBaseItem* );
    virtual int type() const;
    virtual bool lessThan( const ProjectBaseItem* ) const;
    virtual QVariant data(int role = Qt::UserRole + 1) const;
    virtual void setData(const QVariant& value, int role = Qt::UserRole + 1);
private slots:
    void projectConfigChanged( KDevelop::IProject* );
private:
    void setConfigText();
    void updateCurrentConfig( KDevelop::IProject* );
    KDevelop::ProjectBaseItem* projectitem;
    QString currentConfig;
};

#endif

