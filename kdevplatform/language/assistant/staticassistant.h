/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_STATICASSISTANT_H
#define KDEVPLATFORM_STATICASSISTANT_H

#include <language/languageexport.h>
#include <serialization/indexedstring.h>
#include <language/duchain/topducontext.h>

#include <interfaces/iassistant.h>

namespace KTextEditor {
class Document;
class View;
class Range;
}

namespace KDevelop {
class ILanguageSupport;
class StaticAssistantPrivate;

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
class KDEVPLATFORMLANGUAGE_EXPORT StaticAssistant
    : public IAssistant
{
    Q_OBJECT

public:
    using Ptr = QExplicitlySharedDataPointer<StaticAssistant>;

    explicit StaticAssistant(ILanguageSupport* supportedLanguage);
    ~StaticAssistant() override;

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
    virtual void textChanged(KTextEditor::Document* doc, const KTextEditor::Range& invocationRange,
                             const QString& removedText = QString()) = 0;
    /**
     * Whether it's worth showing this assistant to the user
     *
     * Reimplement in subclass
     */
    virtual bool isUseful() const = 0;

    /**
     * The range the assistant should be displayed in.
     */
    virtual KTextEditor::Range displayRange() const = 0;

    virtual void updateReady(const IndexedString&, const KDevelop::ReferencedTopDUContext&) { }

private:
    const QScopedPointer<class StaticAssistantPrivate> d_ptr;
    Q_DECLARE_PRIVATE(StaticAssistant)
};
}

#endif // KDEVPLATFORM_STATICASSISTANT_H
