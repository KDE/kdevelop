/* This file is part of KDevelop
    Copyright 2008 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "outputpage.h"
#include "ui_outputlocation.h"

#include "createclass.h"
#include "sourcefiletemplate.h"
#include "templaterenderer.h"

#include <KUrlRequester>
#include <KIntNumInput>
#include <KDebug>
#include <KFileDialog>

#include <QSignalMapper>
#include <QLabel>

namespace KDevelop {

struct OutputPagePrivate
{
    Ui::OutputLocationDialog* output;
    QSignalMapper urlChangedMapper;

    QHash<QString, KUrlRequester*> outputFiles;
    QHash<QString, KIntNumInput*> outputLines;
    QHash<QString, KIntNumInput*> outputColumns;

    QHash<QString, KUrl> defaultUrls;
    QHash<QString, KUrl> lowerCaseUrls;
    QStringList fileIdentifiers;
    
    void updateRanges(KIntNumInput * line, KIntNumInput * column, bool enable);
};

void OutputPagePrivate::updateRanges(KIntNumInput * line, KIntNumInput * column, bool enable)
{
    kDebug() << "Updating Ranges, file exists: " << enable;
    line->setEnabled(enable);
    column->setEnabled(enable);
}

OutputPage::OutputPage(QWidget* parent)
: QWidget(parent)
, d(new OutputPagePrivate)
{
    d->output = new Ui::OutputLocationDialog;
    d->output->setupUi(this);

    connect(&d->urlChangedMapper, SIGNAL(mapped(QString)), SLOT(updateFileRange(QString)));
    connect(d->output->lowerFilenameCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateFileNames()));
}

void OutputPage::loadFileTemplate (const SourceFileTemplate& fileTemplate, const KUrl& baseUrl, TemplateRenderer* renderer)
{
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup codegenGroup( config, "CodeGeneration" );
    bool lower = codegenGroup.readEntry( "LowerCaseFilenames", true );
    d->output->lowerFilenameCheckBox->setChecked(lower);

    foreach (const SourceFileTemplate::OutputFile& file, fileTemplate.outputFiles())
    {
        d->fileIdentifiers << file.identifier;

        KUrl url = baseUrl;
        url.addPath(renderer->render(file.outputName));
        d->defaultUrls.insert(file.identifier, url);

        url = baseUrl;
        url.addPath(renderer->render(file.outputName).toLower());
        d->lowerCaseUrls.insert(file.identifier, url);
        
        QLabel* label = new QLabel(file.label, this);
        KUrlRequester* requester = new KUrlRequester(this);
        requester->setMode( KFile::File | KFile::LocalOnly );
        requester->fileDialog()->setOperationMode( KFileDialog::Saving );

        d->urlChangedMapper.setMapping(requester, file.identifier);
        connect(requester, SIGNAL(textChanged(QString)), &d->urlChangedMapper, SLOT(map()));

        d->output->urlFormLayout->addRow(label, requester);
        d->outputFiles.insert(file.identifier, requester);

        label = new QLabel(file.label, this);
        QHBoxLayout* layout = new QHBoxLayout(this);

        KIntNumInput* line = new KIntNumInput(this);
        line->setPrefix(i18n("Line: "));
        line->setValue(0);
        line->setMinimum(0);
        layout->addWidget(line);

        KIntNumInput* column = new KIntNumInput(this);
        column->setPrefix(i18n("Column: "));
        column->setValue(0);
        column->setMinimum(0);
        layout->addWidget(column);

        d->output->positionFormLayout->addRow(label, layout);
        d->outputLines.insert(file.identifier, line);
        d->outputColumns.insert(file.identifier, column);
    }

    updateFileNames();
}

void OutputPage::updateFileNames()
{
    bool lower = d->output->lowerFilenameCheckBox->isChecked();

    QHash<QString, KUrl> urls = lower ? d->lowerCaseUrls : d->defaultUrls;
    for (QHash<QString, KUrlRequester*>::const_iterator it = d->outputFiles.constBegin(); it != d->outputFiles.constEnd(); ++it)
    {
        if (urls.contains(it.key()))
        {
            it.value()->setUrl(urls[it.key()]);
        }
    }

    //Save the setting for next time
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup codegenGroup( config, "CodeGeneration" );
    codegenGroup.writeEntry( "LowerCaseFilenames", d->output->lowerFilenameCheckBox->isChecked() );

    emit isValid(isComplete());
}

void OutputPage::updateFileRange (const QString& field)
{
    if (!d->outputFiles.contains(field))
    {
        return;
    }

    QString url = d->outputFiles[field]->url().toLocalFile();
    QFileInfo info(url);

    d->updateRanges(d->outputLines[field], d->outputColumns[field], info.exists() && !info.isDir());

    emit isValid(isComplete());
}

bool OutputPage::isComplete() const
{
    foreach (KUrlRequester* requester, d->outputFiles)
    {
        if (!requester->url().isValid())
        {
            return false;
        }
    }
    return true;
}

QHash< QString, KUrl > OutputPage::fileUrls() const
{
    QHash<QString, KUrl> urls;
    for (QHash<QString, KUrlRequester*>::const_iterator it = d->outputFiles.constBegin(); it != d->outputFiles.constEnd(); ++it)
    {
        urls.insert(it.key(), it.value()->url());
    }
    return urls;
}

QHash< QString, SimpleCursor > OutputPage::filePositions() const
{
    QHash<QString, SimpleCursor> positions;
    foreach (const QString& identifier, d->fileIdentifiers)
    {
        positions.insert(identifier, SimpleCursor(d->outputLines[identifier]->value(), d->outputColumns[identifier]->value()));
    }
    return positions;
}

OutputPage::~OutputPage()
{
    delete d;
}

}
