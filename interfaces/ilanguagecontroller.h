/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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
#ifndef KDEVPLATFORM_ILANGUAGECONTROLLER_H
#define KDEVPLATFORM_ILANGUAGECONTROLLER_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QUrl>

#include "interfacesexport.h"

namespace KDevelop {

class ILanguage;
class BackgroundParser;
class ICompletionSettings;
class StaticAssistantsManager;

/**
 * @class ILanguageController
 */
class KDEVPLATFORMINTERFACES_EXPORT ILanguageController: public QObject {
    Q_OBJECT
public:
    ILanguageController(QObject *parent = 0);

    /** @return the currently active languages loaded for the currently active file.
    The list is empty if the file's language is unsupported. */
    Q_SCRIPTABLE virtual QList<ILanguage*>activeLanguages() = 0;
    /** @return the language for given @p name. */
    Q_SCRIPTABLE virtual ILanguage* language(const QString &name) const = 0;
    /** @return the languages that support the MIME type of @p url.
      * @warning If this is called from within the foreground thread,
      *          the language support is loaded if required.
      *          If it is called from a background thread, it can not
      *          be loaded, and thus zero will be returned.
      * */
    Q_SCRIPTABLE virtual QList<ILanguage*> languagesForUrl(const QUrl &url) = 0;

    /** @return All languages currently loaded */
    Q_SCRIPTABLE virtual QList<ILanguage*> loadedLanguages() const = 0;

    /** @return the background parser used to parse source files */
    Q_SCRIPTABLE virtual BackgroundParser *backgroundParser() const = 0;

    /** @return The global code assistant manager (manages assistants such as the RenameAssistant) */
    Q_SCRIPTABLE virtual StaticAssistantsManager *staticAssistantsManager() const = 0;

    /** Access to the completion settings */
    Q_SCRIPTABLE virtual ICompletionSettings *completionSettings() const = 0;
};

}

#endif

