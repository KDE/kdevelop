/***************************************************************************
 *   Copyright 2006 Adam Treat <treat@kde.org>                         *
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

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

