/* KDevelop CMake Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakecachereader.h"

#include <QString>
#include <QFile>
#include <QUrl>

void CacheLine::readLine(const QString& line)
{
    m_line=line;
    int i;
    for(i=0; i<line.count() && line[i]!='='; i++)
    {
        if(line[i]==':')
        {
            colon=i;
            if(endName<0)
                endName=i;
        }
        else if(line[i]=='-')
        {
            dash=i;
                endName=i;
        }
    }
    equal=i;
}

QString CacheLine::name() const
{ return m_line.left( endName ); }

QString CacheLine::flag() const
{
    if(dash>0)
        return m_line.mid( dash+1, colon-dash-1 );
    else
        return QString();
}

QString CacheLine::type() const
{
    return m_line.mid(colon+1, equal-colon-1);
}

QString CacheLine::value() const
{
    return m_line.right(m_line.size()-equal-1);
}
