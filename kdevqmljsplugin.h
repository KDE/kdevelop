/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *  Copyright (C) 2012 by Milian Wolff <mail@milianw.de>                             *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef KDEVQMLJSPLUGIN_H
#define KDEVQMLJSPLUGIN_H

#include <interfaces/iplugin.h>
#include <language/interfaces/ilanguagesupport.h>

class KDevQmlJsPlugin : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::ILanguageSupport )
public:
    explicit KDevQmlJsPlugin( QObject* parent, const QVariantList& args = QVariantList() );

    virtual KDevelop::ParseJob* createParseJob(const KDevelop::IndexedString& url);
    virtual QString name() const;

    virtual KDevelop::ICodeHighlighting* codeHighlighting() const;
    virtual KDevelop::BasicRefactoring* refactoring() const;

    virtual KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);
    virtual KDevelop::SimpleRange specialLanguageObjectRange(const KUrl& url, const KDevelop::SimpleCursor& position);
    virtual QWidget* specialLanguageObjectNavigationWidget(const KUrl& url, const KDevelop::SimpleCursor& position);

private:
    KDevelop::ICodeHighlighting* m_highlighting;
    KDevelop::BasicRefactoring* m_refactoring;
};

#endif // KDEVQMLJSPLUGIN_H
