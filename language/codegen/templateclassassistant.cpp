/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "templateclassassistant.h"
#include "templateselectionpage.h"
#include "templateoptionspage.h"
#include "classmemberspage.h"
#include "templateclassgenerator.h"

#include "interfaces/icore.h"
#include "interfaces/ilanguagecontroller.h"
#include "interfaces/ilanguagesupport.h"
#include "interfaces/ilanguage.h"
#include "interfaces/icreateclasshelper.h"

#include <KLocalizedString>
#include <KConfig>

using namespace KDevelop;

class KDevelop::TemplateClassAssistantPrivate
{
public:
    KPageWidgetItem* templateSelectionPage;
    KPageWidgetItem* dummyPage;
    KPageWidgetItem* templateOptionsPage;
    KPageWidgetItem* membersPage;
    ICreateClassHelper* helper;
};

TemplateClassAssistant::TemplateClassAssistant (QWidget* parent, const KUrl& baseUrl)
: CreateClassAssistant (parent, baseUrl)
, d(new TemplateClassAssistantPrivate)
{
    d->templateOptionsPage = 0;
    setup();
}

TemplateClassAssistant::~TemplateClassAssistant()
{
    delete d;
}

void TemplateClassAssistant::setup()
{
    setWindowTitle(i18n("Create New Class in %1", baseUrl().prettyUrl()));

    TemplateSelectionPage* page = newTemplateSelectionPage();
    d->templateSelectionPage = addPage(page, i18n("Language and Template"));
    connect (this, SIGNAL(accepted()), page, SLOT(saveConfig()));

    /*
     * All assistant pages except the first one require the helper to already be set.
     * However, we can only choose the helper aften the language is selected,
     * so other pages cannot be loaded here yet.
     *
     * OTOH, having only one page disables the "next" button and enables the "finish" button.
     * This is not wanted, so we create a dummy page and delete it when "next" is clicked
     */
    QWidget* dummy = new QWidget(this);
    d->dummyPage = addPage(dummy, QLatin1String("Dummy Page"));

    setCurrentPage(d->templateSelectionPage);
}

void TemplateClassAssistant::next()
{
    if (currentPage() == d->templateSelectionPage)
    {
        kDebug() << "Current page is template selection";
        QString description = currentPage()->widget()->property("selectedTemplate").toString();

        kDebug() << "Chosen template is" << description;

        KConfig config(description);
        KConfigGroup group(&config, "General");

        kDebug() << "Template name is" << group.readEntry("Name");

        QString languageName = group.readEntry("Category").split('/').first();

        ILanguage* language = ICore::self()->languageController()->language(languageName);

        if (!language)
        {
            kDebug() << "No language named" << languageName;
            return;
        }

        d->helper = language->languageSupport()->createClassHelper(this);

        if (!d->helper)
        {
            kDebug() << "No class creation helper for language" << languageName;
            return;
        }

        ClassGenerator* generator = d->helper->generator();
        if (!generator)
        {
            kDebug() << "No generator for language" << languageName;
            return;
        }


        setGenerator(generator);

        TemplateClassGenerator* templateGenerator = dynamic_cast<TemplateClassGenerator*>(generator);
        if (templateGenerator)
        {
            kDebug() << "Class generator uses templates";
            templateGenerator->setTemplateDescription(description);
        }

        removePage(d->dummyPage);
        KDevelop::CreateClassAssistant::setup();

        ClassMembersPage* membersPage = new ClassMembersPage(this);
        d->membersPage = addPage(membersPage, i18n("Data Members"));

        if (templateGenerator && templateGenerator->hasCustomOptions())
        {
            kDebug() << "Class generator has custom options";
            TemplateOptionsPage* options = new TemplateOptionsPage(this);
            d->templateOptionsPage = addPage(options, i18n("Template Options"));
        }

        return;
    }

    KDevelop::CreateClassAssistant::next();

    if (currentPage() == d->membersPage)
    {
        ClassDescription desc(generator()->name());
        foreach (DeclarationPointer declaration, generator()->declarations())
        {
            desc.methods << FunctionDescription(declaration);
        }
        d->membersPage->widget()->setProperty("description", QVariant::fromValue(desc));
    }

    if (d->templateOptionsPage && (currentPage() == d->templateOptionsPage))
    {
        TemplateOptionsPage* options = qobject_cast<TemplateOptionsPage*>(d->templateOptionsPage->widget());
        TemplateClassGenerator* templateGenerator = dynamic_cast<TemplateClassGenerator*>(generator());

        options->loadXML(templateGenerator->customOptions());
    }
}

void TemplateClassAssistant::accept()
{
    if (d->templateOptionsPage)
    {
        TemplateClassGenerator* templateGenerator = dynamic_cast<TemplateClassGenerator*>(generator());
        Q_ASSERT(templateGenerator);

        kDebug() << d->templateOptionsPage->widget()->property("templateOptions");
        kDebug() << d->templateOptionsPage->widget()->property("templateOptions").toHash();
        templateGenerator->addVariables(d->templateOptionsPage->widget()->property("templateOptions").toHash());
    }

    ClassDescription desc = d->membersPage->widget()->property("description").value<ClassDescription>();
    generator()->setDescription(desc);

    CreateClassAssistant::accept();
}

TemplateSelectionPage* TemplateClassAssistant::newTemplateSelectionPage()
{
    return new TemplateSelectionPage(this);
}

ClassIdentifierPage* TemplateClassAssistant::newIdentifierPage()
{
    return d->helper->identifierPage();
}

OverridesPage* TemplateClassAssistant::newOverridesPage()
{
    return d->helper->overridesPage();
}

ClassMembersPage* TemplateClassAssistant::newMembersPage()
{
    return new ClassMembersPage(this);
}

