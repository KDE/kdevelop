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

#include <KUrlRequester>
#include <KIntNumInput>
#include <KDebug>
#include <KFileDialog>

#include <QSignalMapper>
#include <QLabel>

namespace KDevelop {

struct OutputPagePrivate
{
    OutputPagePrivate()
        : output(0)
    {
    }

    Ui::OutputLocationDialog* output;
    CreateClassAssistant* parent;
    QSignalMapper urlChangedMapper;

    QHash<QString, KUrlRequester*> outputFiles;
    QHash<QString, KIntNumInput*> outputLines;
    QHash<QString, KIntNumInput*> outputColumns;

    void updateRanges(KIntNumInput * line, KIntNumInput * column, bool enable);
};

void OutputPagePrivate::updateRanges(KIntNumInput * line, KIntNumInput * column, bool enable)
{
    kDebug() << "Updating Ranges, file exists: " << enable;
    line->setEnabled(enable);
    column->setEnabled(enable);
}

OutputPage::OutputPage(CreateClassAssistant* parent)
: QWidget(parent)
, d(new OutputPagePrivate)
{
    d->parent = parent;

    d->output = new Ui::OutputLocationDialog;
    d->output->setupUi(this);

    foreach (const QString& text, d->parent->generator()->fileLabels())
    {
        QLabel* label = new QLabel(text, this);
        KUrlRequester* requester = new KUrlRequester(this);
        requester->setMode( KFile::File | KFile::LocalOnly );
        requester->fileDialog()->setOperationMode( KFileDialog::Saving );

        d->urlChangedMapper.setMapping(requester, text);
        connect(requester, SIGNAL(textChanged(QString)), &d->urlChangedMapper, SLOT(map()));

        d->output->urlFormLayout->addRow(label, requester);
        d->outputFiles.insert(text, requester);

        label = new QLabel(text, this);
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
        d->outputLines.insert(text, line);
        d->outputColumns.insert(text, column);
    }

    connect(&d->urlChangedMapper, SIGNAL(mapped(QString)), SLOT(updateFileRange(QString)));
    connect(d->output->lowerFilenameCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateFileNames()));
}

void OutputPage::initializePage()
{
     //Read the setting for lower case filenames
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup codegenGroup( config, "CodeGeneration" );
    bool lower = codegenGroup.readEntry( "LowerCaseFilenames", true );
    d->output->lowerFilenameCheckBox->setChecked(lower);

    updateFileNames();
}

void OutputPage::updateFileNames()
{
    QHash<QString, KUrl> urls = d->parent->generator()->fileUrlsFromBase(d->parent->baseUrl(), d->output->lowerFilenameCheckBox->isChecked());

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

bool OutputPage::validatePage()
{
    for (QHash<QString,KUrlRequester*>::const_iterator it = d->outputFiles.constBegin(); it != d->outputFiles.constEnd(); ++it)
    {
        d->parent->generator()->setFileUrl(it.key(), it.value()->url());

        int line = d->outputLines[it.key()]->value();
        int column = d->outputColumns[it.key()]->value();
        d->parent->generator()->setFilePosition(it.key(), SimpleCursor(line, column));
    }
    return true;
}

OutputPage::~OutputPage()
{
    delete d;
}

}
