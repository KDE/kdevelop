/*****************************************************************************
 * Copyright (c) 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>            *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#ifndef KDEVCLANGLANGUAGESUPPORT_H
#define KDEVCLANGLANGUAGESUPPORT_H

#include <interfaces/iplugin.h>
#include <language/interfaces/ilanguagesupport.h>

#include <QtCore/QVariant>

namespace KDevelop
{
class ICodeHighlighting;
class ParseJob;
}

class FileManager;

class ClangLanguageSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ILanguageSupport)

public:
    explicit ClangLanguageSupport(QObject *parent, const QVariantList& args = QVariantList());
    virtual ~ClangLanguageSupport();

    /** Name Of the Language */
    virtual QString name() const;

    /** Parsejob used by background parser to parse given Url */
    virtual KDevelop::ParseJob *createParseJob(const KUrl &url);

    /** the actual language object */
    virtual KDevelop::ILanguage *language();

    /** the code highlighter */
    virtual const KDevelop::ICodeHighlighting* codeHighlighting() const;

private:
    const KDevelop::ICodeHighlighting *const m_highlighting;
    FileManager *const m_fileManager;
};

#endif

