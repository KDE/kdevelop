/*
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakecachereader.h"

#include <QString>

void CacheLine::readLine(const QString& line)
{
    m_line=line;
    int i;
    for (i=0; i<line.count() && line[i] != QLatin1Char('='); ++i) {
        if(line[i] == QLatin1Char(':')) {
            colon=i;
            if(endName<0)
                endName=i;
        } else if(line[i] == QLatin1Char('-')) {
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
    return m_line.mid(equal+1);
}
