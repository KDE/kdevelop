/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ILANGUAGECONTROLLER_H
#define KDEVPLATFORM_ILANGUAGECONTROLLER_H

#include <QList>
#include <QObject>
#include <QUrl>

#include "interfacesexport.h"

namespace KDevelop {

class ILanguageSupport;

class BackgroundParser;
class ICompletionSettings;
class StaticAssistantsManager;
class ProblemModelSet;

/**
 * @class ILanguageController
 */
class KDEVPLATFORMINTERFACES_EXPORT ILanguageController: public QObject {
    Q_OBJECT
public:
    explicit ILanguageController(QObject *parent = nullptr);

    /**
     * @return the currently active languages loaded for the currently active file.
     *
     * The list is empty if the file's language is unsupported.
     */
    virtual QList<ILanguageSupport*> activeLanguages() = 0;

    /**
     *@return the language for given @p name.
     */
    virtual ILanguageSupport* language(const QString &name) const = 0;

    /**
     * @return the languages that support the MIME type of @p url.
     * @warning If this is called from within the foreground thread,
     *          the language support is loaded if required.
     *          If it is called from a background thread, it can not
     *          be loaded, and thus zero will be returned.
     */
    virtual QList<ILanguageSupport*> languagesForUrl(const QUrl &url) = 0;

    /** @return All languages currently loaded */
    virtual QList<ILanguageSupport*> loadedLanguages() const = 0;

    /** @return the background parser used to parse source files */
    virtual BackgroundParser *backgroundParser() const = 0;

    /** @return The global code assistant manager (manages assistants such as the RenameAssistant) */
    virtual StaticAssistantsManager *staticAssistantsManager() const = 0;

    /** Access to the completion settings */
    virtual ICompletionSettings *completionSettings() const = 0;

    virtual ProblemModelSet* problemModelSet() const = 0;
};

}

#endif

