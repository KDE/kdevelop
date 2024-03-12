/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "templaterenderer.h"

#include "documentchangeset.h"
#include "sourcefiletemplate.h"
#include "templateengine.h"
#include "templateengine_p.h"
#include "archivetemplateloader.h"
#include <debug.h>

#include <serialization/indexedstring.h>

#include <KArchive>
#include <KTextTemplate/Context>

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QUrl>

using namespace KTextTemplate;

class NoEscapeStream
    : public OutputStream
{
public:
    NoEscapeStream();
    explicit NoEscapeStream (QTextStream* stream);

    QString escape (const QString& input) const override;
    QSharedPointer<OutputStream> clone (QTextStream* stream) const override;
};

NoEscapeStream::NoEscapeStream() : OutputStream()
{
}

NoEscapeStream::NoEscapeStream(QTextStream* stream) : OutputStream(stream)
{
}

QString NoEscapeStream::escape(const QString& input) const
{
    return input;
}

QSharedPointer<OutputStream> NoEscapeStream::clone(QTextStream* stream) const
{
    QSharedPointer<OutputStream> clonedStream = QSharedPointer<OutputStream>(new NoEscapeStream(stream));
    return clonedStream;
}

using namespace KDevelop;

namespace KDevelop {
class TemplateRendererPrivate
{
public:
    Engine* engine;
    KTextTemplate::Context context;
    TemplateRenderer::EmptyLinesPolicy emptyLinesPolicy;
    QString errorString;
};
}

TemplateRenderer::TemplateRenderer()
    : d_ptr(new TemplateRendererPrivate)
{
    Q_D(TemplateRenderer);

    d->engine = &TemplateEngine::self()->d_ptr->engine;
    d->emptyLinesPolicy = KeepEmptyLines;
}

TemplateRenderer::~TemplateRenderer() = default;

void TemplateRenderer::addVariables(const QVariantHash& variables)
{
    Q_D(TemplateRenderer);

    QVariantHash::const_iterator it = variables.constBegin();
    QVariantHash::const_iterator end = variables.constEnd();
    for (; it != end; ++it) {
        d->context.insert(it.key(), it.value());
    }
}

void TemplateRenderer::addVariable(const QString& name, const QVariant& value)
{
    Q_D(TemplateRenderer);

    d->context.insert(name, value);
}

QVariantHash TemplateRenderer::variables() const
{
    Q_D(const TemplateRenderer);

    return d->context.stackHash(0);
}

QString TemplateRenderer::render(const QString& content, const QString& name)
{
    Q_D(TemplateRenderer);

    Template t = d->engine->newTemplate(content, name);

    QString output;
    QTextStream textStream(&output);
    NoEscapeStream stream(&textStream);
    t->render(&stream, &d->context);

    if (t->error() != KTextTemplate::NoError) {
        d->errorString = t->errorString();
    } else {
        d->errorString.clear();
    }

    if (d->emptyLinesPolicy == TrimEmptyLines && output.contains(QLatin1Char('\n'))) {
        QStringList lines = output.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        QMutableStringListIterator it(lines);

        // Remove empty lines from the start of the document
        while (it.hasNext()) {
            if (it.next().trimmed().isEmpty()) {
                it.remove();
            } else
            {
                break;
            }
        }

        // Remove single empty lines
        it.toFront();
        bool prePreviousEmpty = false;
        bool previousEmpty = false;
        while (it.hasNext()) {
            bool currentEmpty = it.peekNext().trimmed().isEmpty();
            if (!prePreviousEmpty && previousEmpty && !currentEmpty) {
                it.remove();
            }
            prePreviousEmpty = previousEmpty;
            previousEmpty = currentEmpty;
            it.next();
        }

        // Compress multiple empty lines
        it.toFront();
        previousEmpty = false;
        while (it.hasNext()) {
            bool currentEmpty = it.next().trimmed().isEmpty();
            if (currentEmpty && previousEmpty) {
                it.remove();
            }
            previousEmpty = currentEmpty;
        }

        // Remove empty lines from the end
        it.toBack();
        while (it.hasPrevious()) {
            if (it.previous().trimmed().isEmpty()) {
                it.remove();
            } else
            {
                break;
            }
        }

        // Add a newline to the end of file
        it.toBack();
        it.insert(QString());

        output = lines.join(QLatin1Char('\n'));
    } else if (d->emptyLinesPolicy == RemoveEmptyLines) {
        QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        QMutableStringListIterator it(lines);
        while (it.hasNext()) {
            if (it.next().trimmed().isEmpty()) {
                it.remove();
            }
        }
        it.toBack();
        if (lines.size() > 1) {
            it.insert(QString());
        }
        output = lines.join(QLatin1Char('\n'));
    }

    return output;
}

QString TemplateRenderer::renderFile(const QUrl& url, const QString& name)
{
    QFile file(url.toLocalFile());
    file.open(QIODevice::ReadOnly);

    const QString content = QString::fromUtf8(file.readAll());
    qCDebug(LANGUAGE) << content;

    return render(content, name);
}

QStringList TemplateRenderer::render(const QStringList& contents)
{
    Q_D(TemplateRenderer);

    qCDebug(LANGUAGE) << d->context.stackHash(0);
    QStringList ret;
    ret.reserve(contents.size());
    for (const QString& content : contents) {
        ret << render(content);
    }

    return ret;
}

void TemplateRenderer::setEmptyLinesPolicy(TemplateRenderer::EmptyLinesPolicy policy)
{
    Q_D(TemplateRenderer);

    d->emptyLinesPolicy = policy;
}

TemplateRenderer::EmptyLinesPolicy TemplateRenderer::emptyLinesPolicy() const
{
    Q_D(const TemplateRenderer);

    return d->emptyLinesPolicy;
}

DocumentChangeSet TemplateRenderer::renderFileTemplate(const SourceFileTemplate& fileTemplate,
                                                       const QUrl& baseUrl,
                                                       const QHash<QString, QUrl>& fileUrls)
{
    Q_ASSERT(fileTemplate.isValid());

    DocumentChangeSet changes;
    const QDir baseDir(baseUrl.path());

    static const QRegularExpression nonAlphaNumeric(QStringLiteral("\\W"));
    for (QHash<QString, QUrl>::const_iterator it = fileUrls.constBegin(); it != fileUrls.constEnd(); ++it) {
        QString cleanName = it.key().toLower();
        cleanName.replace(nonAlphaNumeric, QStringLiteral("_"));
        const QString path = it.value().toLocalFile();
        addVariable(QLatin1String("output_file_") + cleanName, baseDir.relativeFilePath(path));
        addVariable(QLatin1String("output_file_") + cleanName + QLatin1String("_absolute"), path);
    }

    const KArchiveDirectory* directory = fileTemplate.directory();
    ArchiveTemplateLocation location(directory);
    const auto outputFiles = fileTemplate.outputFiles();
    for (const SourceFileTemplate::OutputFile& outputFile : outputFiles) {
        const KArchiveEntry* entry = directory->entry(outputFile.fileName);
        if (!entry) {
            qCWarning(LANGUAGE) << "Entry" << outputFile.fileName << "is mentioned in group" << outputFile.identifier <<
                "but is not present in the archive";
            continue;
        }

        const auto* file = dynamic_cast<const KArchiveFile*>(entry);
        if (!file) {
            qCWarning(LANGUAGE) << "Entry" << entry->name() << "is not a file";
            continue;
        }

        QUrl url = fileUrls[outputFile.identifier];
        IndexedString document(url);
        KTextEditor::Range range(KTextEditor::Cursor(0, 0), 0);

        DocumentChange change(document, range, QString(),
            render(QString::fromUtf8(file->data()), outputFile.identifier));
        changes.addChange(change);
        qCDebug(LANGUAGE) << "Added change for file" << document.str();
    }

    return changes;
}

QString TemplateRenderer::errorString() const
{
    Q_D(const TemplateRenderer);

    return d->errorString;
}
