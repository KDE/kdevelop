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

namespace KParts {
class Part;
}

namespace KDevelop {

class ILanguageSupport;

class KDEVPLATFORMSHELL_EXPORT LanguageController : public ILanguageController {
    Q_OBJECT
public:
    LanguageController(QObject *parent);
    virtual ~LanguageController();

    void initialize();
    
    //After this was called, no more languages will be returned
    void cleanup();

    /** @copydoc ILanguageController::activeLanguages() */
    virtual QList<ILanguage*> activeLanguages();
    /** @copydoc ILanguageController::language() */
    virtual ILanguage *language(const QString &name) const;
    /** @copydoc ILanguageController::languageForUrl() */
    virtual QList<ILanguage*> languagesForUrl(const KUrl &url);
    /** @copydoc ILanguageController::backgroundParser() */
    Q_SCRIPTABLE virtual BackgroundParser *backgroundParser() const;

    virtual StaticAssistantsManager *staticAssistantsManager() const;

    virtual QList<ILanguage*> loadedLanguages() const;

    virtual ICompletionSettings *completionSettings() const;

    QList<ILanguage*> languagesForMimetype(const QString& mime);
    QList<QString> mimetypesForLanguageName(const QString& languageName);

protected:
    /**
     * functions for unit tests
     * @see TestLanguageController
     */
    void addLanguageSupport(KDevelop::ILanguageSupport* languageSupport, const QStringList& mimetypes);

private:

    struct LanguageControllerPrivate *d;

    Q_PRIVATE_SLOT(d, void documentActivated(KDevelop::IDocument *document))
};

}

#endif

