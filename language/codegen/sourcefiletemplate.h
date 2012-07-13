/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KDEVELOP_SOURCEFILETEMPLATE_H
#define KDEVELOP_SOURCEFILETEMPLATE_H

#include <QString>
#include <QList>

#include "../languageexport.h"

class KArchiveDirectory;
namespace KDevelop
{

class KDEVPLATFORMLANGUAGE_EXPORT SourceFileTemplate
{
public:
    struct OutputFile
    {
        QString identifier;
        QString fileName;
        QString label;
        QString outputName;
    };

    SourceFileTemplate(const QString& templateDescription);
    ~SourceFileTemplate();

    QString name() const;
    const KArchiveDirectory* directory();
    QList<OutputFile> outputFiles() const;

    /**
     * @return true if the template uses any custom options, false otherwise
     **/
    bool hasCustomOptions();

    /**
     * Returns the contents of the template's configuration options file.
     * The contents are in the format of .kcfg files used by KConfig XT.
     **/
    QByteArray customOptions();

private:
    class SourceFileTemplatePrivate* const d;
};
}

#endif // KDEVELOP_SOURCEFILETEMPLATE_H
