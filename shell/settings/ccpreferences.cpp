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

#include "ccpreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>
#include <kaboutdata.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/codecompletioninterface.h>
#include <KShell>

#include "../core.h"
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>

#include "ccconfig.h"

#include "ui_ccsettings.h"
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>

#include "../completionsettings.h"

using namespace KTextEditor;

namespace KDevelop
{

K_PLUGIN_FACTORY(CCPreferencesFactory, registerPlugin<CCPreferences>();)
K_EXPORT_PLUGIN(CCPreferencesFactory(KAboutData("kcm_kdev_ccsettings", "kdevplatform", ki18n("Language Support Settings"), "0.1")))

CCPreferences::CCPreferences( QWidget *parent, const QVariantList &args )
 : KCModule( CCPreferencesFactory::componentData(), parent, args )
{
    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    preferencesDialog = new Ui::CCSettings;
    preferencesDialog->setupUi( w );
    connect(preferencesDialog->todoMarkerWords, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(preferencesDialog->completionDetail, SIGNAL(currentIndexChanged(int)), SLOT(changed()));

    l->addWidget( w );

    addConfig( CCSettings::self(), w );

    load();
}

void CCPreferences::load()
{
    KCModule::load();
    ICompletionSettings* s = ICore::self()->languageController()->completionSettings();
    QStringList words = s->todoMarkerWords();
    preferencesDialog->todoMarkerWords->setText(KShell::joinArgs(words));
    preferencesDialog->completionDetail->setCurrentIndex(s->completionLevel());
}

void CCPreferences::writeToSettings()
{
    CompletionSettings& settings(static_cast<CompletionSettings&>(*ICore::self()->languageController()->completionSettings()));
    settings.m_automatic = preferencesDialog->kcfg_automaticInvocation->isChecked();
    
    settings.m_level = ICompletionSettings::CompletionLevel(preferencesDialog->completionDetail->currentIndex());
    settings.m_localColorizationLevel = preferencesDialog->kcfg_localColorization->value();
    settings.m_globalColorizationLevel = preferencesDialog->kcfg_globalColorization->value();
    settings.m_highlightSemanticProblems = preferencesDialog->kcfg_highlightSemanticProblems->isChecked();
    settings.m_highlightProblematicLines = preferencesDialog->kcfg_highlightProblematicLines->isChecked();
    settings.m_showMultiLineInformation = preferencesDialog->kcfg_showMultiLineSelectionInformation->isChecked();
    settings.m_minFilesForSimplifiedParsing = preferencesDialog->kcfg_minFilesForSimplifiedParsing->value();
    settings.m_todoMarkerWords = KShell::splitArgs(preferencesDialog->todoMarkerWords->text());
    
    emit settings.emitChanged();
}

CCPreferences::~CCPreferences( )
{
    delete preferencesDialog;
}

void CCPreferences::save()
{
    KCModule::save();

    foreach (KDevelop::IDocument* doc, Core::self()->documentController()->openDocuments())
        if (Document* textDoc = doc->textDocument())
            foreach (View* view, textDoc->views())
                if (CodeCompletionInterface* cc = dynamic_cast<CodeCompletionInterface*>(view))
                    cc->setAutomaticInvocationEnabled(preferencesDialog->kcfg_automaticInvocation->isChecked());
    
    writeToSettings();
}

}

#include "ccpreferences.moc"

