/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_LANGUAGECONTROLLER_H
#define KDEVPLATFORM_LANGUAGECONTROLLER_H

#include <interfaces/ilanguagecontroller.h>

#include "shellexport.h"

namespace KDevelop {

class ILanguageSupport;
class LanguageControllerPrivate;

class KDEVPLATFORMSHELL_EXPORT LanguageController : public ILanguageController {
    Q_OBJECT
public:
    explicit LanguageController(QObject *parent);
    ~LanguageController() override;

    void initialize();
    
    //After this was called, no more languages will be returned
    void cleanup();

    /** @copydoc ILanguageController::activeLanguages() */
    QList<ILanguageSupport*> activeLanguages() override;
    /** @copydoc ILanguageController::language() */
    ILanguageSupport* language(const QString &name) const override;
    /** @copydoc ILanguageController::languageForUrl() */
    QList<ILanguageSupport*> languagesForUrl(const QUrl &url) override;
    /** @copydoc ILanguageController::backgroundParser() */
    BackgroundParser *backgroundParser() const override;

    StaticAssistantsManager *staticAssistantsManager() const override;

    QList<ILanguageSupport*> loadedLanguages() const override;

    ICompletionSettings *completionSettings() const override;

    ProblemModelSet* problemModelSet() const override;

    QList<ILanguageSupport*> languagesForMimetype(const QString& mime);
    QList<QString> mimetypesForLanguageName(const QString& languageName);

protected:
    /**
     * functions for unit tests
     * @see TestLanguageController
     */
    void addLanguageSupport(KDevelop::ILanguageSupport* languageSupport, const QStringList& mimetypes);

private:
    const QScopedPointer<class LanguageControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(LanguageController)
};

}

#endif

