/* KDevelop Project Settings
*
* Copyright 2006  Matt Rogers <mattr@kde.org>
* Copyright 2007  Hamish Rodda <rodda@kde.org>
* Copyright 2009  David Nolden <david.nolden.kdevelop@art-master.de>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#include "languagepreferences.h"

#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KTextEditor/CodeCompletionInterface>

#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>

#include "../completionsettings.h"
#include "../core.h"
#include "languageconfig.h"
#include "ui_languagepreferences.h"

using namespace KTextEditor;

namespace KDevelop
{

LanguagePreferences::LanguagePreferences(QWidget* parent)
    : ConfigPage(nullptr, LanguageConfig::self(), parent)
{
    preferencesDialog = new Ui::LanguagePreferences;
    preferencesDialog->setupUi(this);
    preferencesDialog->kcfg_minFilesForSimplifiedParsing->setSuffix(ki18np(" file", " files"));
}

void LanguagePreferences::notifySettingsChanged()
{
    auto& settings(static_cast<CompletionSettings&>(*ICore::self()->languageController()->completionSettings()));

    settings.emitChanged();
}

LanguagePreferences::~LanguagePreferences( )
{
    delete preferencesDialog;
}

void LanguagePreferences::apply()
{
    ConfigPage::apply();

    const auto documents = Core::self()->documentController()->openDocuments();
    for (KDevelop::IDocument* doc : documents) {
        if (Document* textDoc = doc->textDocument()) {
            const auto views = textDoc->views();
            for (View* view : views) {
                if (auto* cc = qobject_cast<CodeCompletionInterface*>(view)) {
                    cc->setAutomaticInvocationEnabled(preferencesDialog->kcfg_automaticInvocation->isChecked());
                }
            }
        }
    }

    notifySettingsChanged();
}

QString LanguagePreferences::name() const
{
    return i18n("Language Support");
}

QString LanguagePreferences::fullName() const
{
    return i18n("Configure Code-Completion and Semantic Highlighting");
}

QIcon LanguagePreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("page-zoom"));
}

}
