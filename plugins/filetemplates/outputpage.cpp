/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "outputpage.h"
#include "ui_outputlocation.h"
#include "debug.h"

#include <language/codegen/sourcefiletemplate.h>
#include <language/codegen/templaterenderer.h>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>
#include <KUrlRequester>

#include <QSpinBox>
#include <QLabel>

namespace KDevelop {

struct OutputPagePrivate
{
    explicit OutputPagePrivate(OutputPage* page_)
    : page(page_)
    , output(nullptr)
    { }
    OutputPage* page;
    Ui::OutputLocationDialog* output;

    QHash<QString, KUrlRequester*> outputFiles;
    QHash<QString, QSpinBox*> outputLines;
    QHash<QString, QSpinBox*> outputColumns;
    QList<QLabel*> labels;

    QHash<QString, QUrl> defaultUrls;
    QHash<QString, QUrl> lowerCaseUrls;
    QStringList fileIdentifiers;

    void updateRanges(QSpinBox* line, QSpinBox* column, bool enable);
    /**
     * This implementation simply enables the position widgets on a file that exists.
     * Derived classes should overload to set the ranges where class generation should be allowed
     *
     * @param field the name of the file to be generated (Header, Implementation, etc)
     */
    void updateFileRange(const QString& field);
    void updateFileNames();
    void validate();
};

void OutputPagePrivate::updateRanges(QSpinBox* line, QSpinBox* column, bool enable)
{
    qCDebug(PLUGIN_FILETEMPLATES) << "Updating Ranges, file exists: " << enable;
    line->setEnabled(enable);
    column->setEnabled(enable);
}

void OutputPagePrivate::updateFileRange(const QString& field)
{
    const auto outputFileIt = outputFiles.constFind(field);
    if (outputFileIt == outputFiles.constEnd()) {
        return;
    }

    const QString url = (*outputFileIt)->url().toLocalFile();
    QFileInfo info(url);

    updateRanges(outputLines[field], outputColumns[field], info.exists() && !info.isDir());

    validate();
}

void OutputPagePrivate::updateFileNames()
{
    bool lower = output->lowerFilenameCheckBox->isChecked();

    const QHash<QString, QUrl> urls = lower ? lowerCaseUrls : defaultUrls;
    for (QHash<QString, KUrlRequester*>::const_iterator it = outputFiles.constBegin();
         it != outputFiles.constEnd(); ++it)
    {
        const QUrl url = urls.value(it.key());
        if (!url.isEmpty())
        {
            it.value()->setUrl(url);
        }
    }

    //Save the setting for next time
    KConfigGroup codegenGroup(KSharedConfig::openConfig(), QStringLiteral("CodeGeneration"));
    codegenGroup.writeEntry( "LowerCaseFilenames", output->lowerFilenameCheckBox->isChecked() );

    validate();
}

void OutputPagePrivate::validate()
{
    QStringList invalidFiles;
    for(QHash< QString, KUrlRequester* >::const_iterator it = outputFiles.constBegin();
        it != outputFiles.constEnd(); ++it)
    {
        if (!it.value()->url().isValid()) {
            invalidFiles << it.key();
        } else if (it.value()->url().isLocalFile() && !QFileInfo(it.value()->url().adjusted(QUrl::RemoveFilename).toLocalFile()).isWritable()) {
            invalidFiles << it.key();
        }
    }

    bool valid = invalidFiles.isEmpty();
    if (valid) {
        output->messageWidget->animatedHide();
    } else {
        std::sort(invalidFiles.begin(), invalidFiles.end());
        output->messageWidget->setMessageType(KMessageWidget::Error);
        output->messageWidget->setCloseButtonVisible(false);
        output->messageWidget->setText(i18np("Invalid output file: %2", "Invalid output files: %2", invalidFiles.count(), invalidFiles.join(QLatin1String(", "))));
        output->messageWidget->animatedShow();
    }
    emit page->isValid(valid);
}

OutputPage::OutputPage(QWidget* parent)
: QWidget(parent)
, d(new OutputPagePrivate(this))
{
    d->output = new Ui::OutputLocationDialog;
    d->output->setupUi(this);
    d->output->messageWidget->setVisible(false);

    connect(d->output->lowerFilenameCheckBox, &QCheckBox::stateChanged,
            this, [&] { d->updateFileNames(); });
}

OutputPage::~OutputPage()
{
    delete d->output;
    delete d;
}

void OutputPage::prepareForm(const SourceFileTemplate& fileTemplate)
{
    // First clear any existing file configurations
    // This can happen when going back and forth between assistant pages
    d->fileIdentifiers.clear();
    d->defaultUrls.clear();
    d->lowerCaseUrls.clear();

    while (d->output->urlFormLayout->count() > 0)
    {
        d->output->urlFormLayout->takeAt(0);
    }
    while (d->output->positionFormLayout->count() > 0)
    {
        d->output->positionFormLayout->takeAt(0);
    }

    qDeleteAll(d->outputFiles);
    qDeleteAll(d->outputLines);
    qDeleteAll(d->outputColumns);
    qDeleteAll(d->labels);

    d->outputFiles.clear();
    d->outputLines.clear();
    d->outputColumns.clear();
    d->labels.clear();

    const auto outputFiles = fileTemplate.outputFiles();

    const int outputFilesCount = outputFiles.count();
    d->output->urlGroupBox->setTitle(i18ncp("@title:group", "Output File", "Output Files", outputFilesCount));
    d->output->positionGroupBox->setTitle(i18ncp("@title:group", "Location within Existing File", "Location within Existing Files", outputFilesCount));

    for (const SourceFileTemplate::OutputFile& file : outputFiles) {
        const QString id = file.identifier;
        d->fileIdentifiers << id;

        const QString fileLabelText = i18nc("@label:chooser file name arg", "%1:", file.label);
        auto* label = new QLabel(fileLabelText, this);
        d->labels << label;
        auto* requester = new KUrlRequester(this);
        requester->setMode( KFile::File | KFile::LocalOnly );

        connect(requester, &KUrlRequester::textChanged, this, [this, id] () { d->updateFileRange(id); });

        d->output->urlFormLayout->addRow(label, requester);
        d->outputFiles.insert(file.identifier, requester);

        label = new QLabel(fileLabelText, this);
        d->labels << label;
        auto* layout = new QHBoxLayout;

        auto line = new QSpinBox(this);
        line->setPrefix(i18n("Line: "));
        line->setValue(0);
        line->setMinimum(0);
        layout->addWidget(line);

        auto column = new QSpinBox(this);
        column->setPrefix(i18n("Column: "));
        column->setValue(0);
        column->setMinimum(0);
        layout->addWidget(column);

        d->output->positionFormLayout->addRow(label, layout);
        d->outputLines.insert(file.identifier, line);
        d->outputColumns.insert(file.identifier, column);
    }
}

void OutputPage::loadFileTemplate(const SourceFileTemplate& fileTemplate,
                                   const QUrl& _baseUrl,
                                   TemplateRenderer* renderer)
{
    QUrl baseUrl = _baseUrl;
    if (!baseUrl.path().endsWith(QLatin1Char('/'))) {
        baseUrl.setPath(baseUrl.path() + QLatin1Char('/'));
    }

    KConfigGroup codegenGroup(KSharedConfig::openConfig(), QStringLiteral("CodeGeneration"));
    bool lower = codegenGroup.readEntry( "LowerCaseFilenames", true );
    d->output->lowerFilenameCheckBox->setChecked(lower);

    const auto outputFiles = fileTemplate.outputFiles();
    for (const SourceFileTemplate::OutputFile& file : outputFiles) {
        d->fileIdentifiers << file.identifier;

        QUrl url = baseUrl.resolved(QUrl(renderer->render(file.outputName)));
        d->defaultUrls.insert(file.identifier, url);

        url = baseUrl.resolved(QUrl(renderer->render(file.outputName).toLower()));
        d->lowerCaseUrls.insert(file.identifier, url);
    }

    d->updateFileNames();
}

QHash< QString, QUrl > OutputPage::fileUrls() const
{
    QHash<QString, QUrl> urls;
    for (QHash<QString, KUrlRequester*>::const_iterator it = d->outputFiles.constBegin(); it != d->outputFiles.constEnd(); ++it)
    {
        urls.insert(it.key(), it.value()->url());
    }
    return urls;
}

QHash< QString, KTextEditor::Cursor > OutputPage::filePositions() const
{
    QHash<QString, KTextEditor::Cursor> positions;
    for (const QString& identifier : std::as_const(d->fileIdentifiers)) {
        positions.insert(identifier, KTextEditor::Cursor(d->outputLines[identifier]->value(), d->outputColumns[identifier]->value()));
    }
    return positions;
}

void OutputPage::setFocusToFirstEditWidget()
{
    d->output->lowerFilenameCheckBox->setFocus();
}

}

#include "moc_outputpage.cpp"
