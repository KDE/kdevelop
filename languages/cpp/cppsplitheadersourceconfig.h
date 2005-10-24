/*
 * KDevelop config for split header/source
 *
 * Copyright (c) 2005 Adam Treat <treat@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef CPPSPLITHEADERSOURCECONFIG_H
#define CPPSPLITHEADERSOURCECONFIG_H

#include <qobject.h>

class CppSupportPart;
class QDomDocument;

class CppSplitHeaderSourceConfig : public QObject
{
    Q_OBJECT
public:
    CppSplitHeaderSourceConfig( CppSupportPart* part, QDomDocument* dom );
    virtual ~CppSplitHeaderSourceConfig();

    bool splitEnabled() const
    {
        return m_splitEnable;
    }
    void CppSplitHeaderSourceConfig::setSplitEnable( bool b );

    bool autoSync() const
    {
        return m_splitSync;
    }
    void CppSplitHeaderSourceConfig::setAutoSync( bool b );

    QString orientation() const
    {
        return m_splitOrientation;
    }
    void CppSplitHeaderSourceConfig::setOrientation( const QString &o );

public slots:
    void store();

private:
    void init();

signals:
    void stored();

private:
    CppSupportPart* m_part;
    QDomDocument* m_dom;
    bool m_splitEnable;
    bool m_splitSync;
    QString m_splitOrientation;

    static QString defaultPath;
};

#endif 
// kate: indent-mode csands; tab-width 4;
