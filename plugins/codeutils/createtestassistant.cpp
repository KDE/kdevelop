/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "createtestassistant.h"
#include "testcaseswidget.h"
#include "testoutputpage.h"
#include "templateselectionpage.h"

#include <language/codegen/sourcefiletemplate.h>
#include <language/codegen/templaterenderer.h>
#include <language/codegen/documentchangeset.h>

#include <KLocalizedString>

class CreateTestAssistantPrivate
{
public:
    KPageWidgetItem* templateSelectionPage;
    KPageWidgetItem* testCasesPage;
    KPageWidgetItem* testOutputPage;

    QString selectedTemplate;

    KUrl baseUrl;
    KDevelop::TemplateRenderer renderer;
    KDevelop::SourceFileTemplate* fileTemplate;
};

CreateTestAssistant::CreateTestAssistant (const KUrl& baseUrl, QWidget* parent, Qt::WFlags flags)
: KAssistantDialog (parent, flags)
, d(new CreateTestAssistantPrivate)
{
    d->baseUrl = baseUrl;
    d->fileTemplate = 0;

    TemplateSelectionPage* selection = new TemplateSelectionPage(this);
    selection->setBaseUrl(baseUrl);
    d->templateSelectionPage = addPage(selection, i18n("Template Selection"));
    setValid(d->templateSelectionPage, false);

    d->testOutputPage = addPage(new TestOutputPage(this), i18n("Output Files"));
    d->testCasesPage = addPage(new TestCasesWidget(this), i18n("Test Cases"));

    connect (d->templateSelectionPage->widget(), SIGNAL(templateValid(bool)), SLOT(templateValid(bool)));
}

CreateTestAssistant::~CreateTestAssistant()
{

}

void CreateTestAssistant::templateValid (bool valid)
{
    setValid(d->templateSelectionPage, valid);
}

void CreateTestAssistant::next()
{
    if (currentPage() == d->templateSelectionPage)
    {
        d->selectedTemplate = d->templateSelectionPage->widget()->property("selectedTemplate").toString();
        d->fileTemplate = new KDevelop::SourceFileTemplate(d->selectedTemplate);
        d->baseUrl = d->templateSelectionPage->widget()->property("baseUrl").value<KUrl>();
        d->renderer.addVariable("name", d->templateSelectionPage->widget()->property("identifier"));

        TestOutputPage::UrlHash urls;
        QHash<QString,QString> labels;
        foreach (const KDevelop::SourceFileTemplate::OutputFile& file, d->fileTemplate->outputFiles())
        {
            KUrl url(d->baseUrl);
            url.addPath(d->renderer.render(file.outputName).toLower());
            urls.insert(file.identifier, url);
            labels.insert(file.identifier, file.label);
        }
        TestOutputPage* page = dynamic_cast<TestOutputPage*>(d->testOutputPage->widget());
        Q_ASSERT(page);
        page->setFileUrls(urls, labels);
    }
    KAssistantDialog::next();
}

void CreateTestAssistant::accept()
{
    TestOutputPage* output = dynamic_cast<TestOutputPage*>(d->testOutputPage->widget());
    Q_ASSERT(output);

    d->renderer.addVariable("testCases", d->testCasesPage->widget()->property("testCases"));
    d->renderer.setEmptyLinesPolicy(KDevelop::TemplateRenderer::TrimEmptyLines);

    KDevelop::DocumentChangeSet changes = d->renderer.renderFileTemplate(d->fileTemplate, d->baseUrl, output->fileUrls());
    changes.applyAllChanges();
    KAssistantDialog::accept();
}
