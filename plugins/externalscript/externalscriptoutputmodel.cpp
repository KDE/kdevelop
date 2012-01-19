/*
    This plugin is part of KDevelop.

    Copyright (C) 2010 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "externalscriptoutputmodel.h"

ExternalScriptOutputModel::ExternalScriptOutputModel( QObject* parent )
    : OutputModel( parent )
{

}

QStringList outHelper(const QMap<int, int>& ranges, const ExternalScriptOutputModel* model)
{
    QStringList out;
    QMap< int, int >::const_iterator it = ranges.constBegin();
    while ( it != ranges.constEnd() ) {
        for ( int i = it.key(), c = it.key() + it.value(); i < c; ++i ) {
            out << model->data( model->index( i, 0 ) ).toString();
        }
        ++it;
    }
    return out;
}

QStringList ExternalScriptOutputModel::stdErr() const
{
    return outHelper(m_stderrRanges, this);
}

QStringList ExternalScriptOutputModel::stdOut() const
{
    return outHelper(m_stdoutRanges, this);
}

void ExternalScriptOutputModel::appendStderrLines( const QStringList& lines )
{
    addPending();
    m_stderrRanges[rowCount()] = lines.count();
    appendLines(lines);
}

void ExternalScriptOutputModel::appendStdoutLines( const QStringList& lines )
{
    addPending();
    m_stdoutRanges[rowCount()] = lines.count();
    appendLines(lines);
}

#include "externalscriptoutputmodel.moc"
