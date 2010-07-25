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

#ifndef EXTERNALSCRIPTOUTPUTMODEL_H
#define EXTERNALSCRIPTOUTPUTMODEL_H

#include <outputview/outputmodel.h>

class ExternalScriptOutputModel : public KDevelop::OutputModel
{
    Q_OBJECT

public:
    ExternalScriptOutputModel(QObject* parent = 0);

    QStringList stdOut() const;
    QStringList stdErr() const;

public slots:
    void appendStdoutLines(const QStringList& lines);
    void appendStderrLines(const QStringList& lines);

private:
    QMap<int, int> m_stdoutRanges;
    QMap<int, int> m_stderrRanges;
};

#endif // EXTERNALSCRIPTOUTPUTMODEL_H
