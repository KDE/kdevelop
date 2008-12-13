/*
* KDevelop xUnit integration
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "executable.h"
#include <KProcess>
#include <KDebug>
#include <QFile>
#include <QDateTime>
#include <QFileInfo>

using QTest::Executable;

Executable::Executable()
{}

Executable::~Executable()
{}

void Executable::setLocation(const KUrl& url)
{
    m_location = url;
}

#define ASSERT_SANE_LOCATION \
    Q_ASSERT(!m_location.isEmpty()); \
    Q_ASSERT(m_location.isLocalFile());

QStringList Executable::fetchFunctions()
{
    ASSERT_SANE_LOCATION

    if (!QFile::exists(m_location.path())) {
        kError() << "Test executable " << m_location.path()
                 << " does not exist. Failed to retrieve test functions.";
        return QStringList();
    }

    KProcess proc;
    proc.setProgram(m_location.pathOrUrl(), QStringList() << "-functions");
    kDebug() << "executing " << proc.program().join(" ");
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.start();
    proc.waitForFinished(-1);
    // this output is typically minimal, a handful of lines
    QByteArray output = proc.readAllStandardOutput();
    return QString(output).split('\n');
}

QString Executable::name() const
{
    ASSERT_SANE_LOCATION
    return m_location.fileName().split('.')[0];
}

KUrl Executable::location() const
{
    ASSERT_SANE_LOCATION
    return m_location;
}

bool Executable::wasModified() const
{
    return lastModified() != m_timestamp;
}
    
void Executable::updateTimestamp()
{
    m_timestamp = lastModified();
}

QDateTime Executable::lastModified() const
{
    return QFileInfo(m_location.fileName()).lastModified();
}
