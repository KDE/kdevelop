/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KDEVPLATFORM_STATICASSISTANT_H
#define KDEVPLATFORM_STATICASSISTANT_H

#include <language/languageexport.h>

#include <interfaces/iassistant.h>

namespace KTextEditor {
class Document;
class View;
class Range;
}

namespace KDevelop {

class ILanguageSupport;

/**
 * @brief This class serves as a base for long-living assistants
 *
 * Normally, an assistant only lives for a short time, and is indirectly owned by a KDevelop::Problem.
 * Static assistants are owned by the language support plugins and exist as long as the language plugin is loaded.
 * They are not created by KDevelop::Problem instances, instead, they check for problematic code for themselves,
 * by tracking document changes via the textChanged() method.
 *
 * Note that static assistants are not bound to a single document/view.
 * Instead the current document/view we're looking at is passed via the textChanged() method.
 *
 * Register instances of this class view StaticAssistantsManager::registerAssistant
 *
 * @sa textChanged()
 * @sa StaticAssistantsManager::registerAssistant
 */
class KDEVPLATFORMLANGUAGE_EXPORT StaticAssistant : public IAssistant
{
    Q_OBJECT

public:
    using Ptr = QExplicitlySharedDataPointer<StaticAssistant>;

    StaticAssistant(ILanguageSupport* supportedLanguage);
    virtual ~StaticAssistant();

    /**
     * Language this static assistant supports
     *
     * textChanged() will only be called for documents with language equal to this assistant's language
     * @sa textChanged(0)
     */
    ILanguageSupport* supportedLanguage() const;

    /**
     * Invoked whenever text inside a view was changed by the user
     *
     * Reimplement in subclass
     */
    virtual void textChanged(KTextEditor::View* view, const KTextEditor::Range& invocationRange,
                             const QString& removedText = QString()) = 0;
    /**
     * Whether it's worth showing this assistant to the user
     *
     * Reimplement in subclass
     */
    virtual bool isUseful() const = 0;

private:
    struct Private;
    QScopedPointer<Private> const d;
};

}

#endif // KDEVPLATFORM_STATICASSISTANT_H
