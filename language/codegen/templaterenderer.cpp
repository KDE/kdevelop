/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "templaterenderer.h"

#include "documentchangeset.h"
#include "sourcefiletemplate.h"
#include "templateengine.h"
#include "templateengine_p.h"
#include "archivetemplateloader.h"
#include "util/debug.h"

#include <serialization/indexedstring.h>

#include <grantlee/context.h>

#include <QUrl>
#include <KArchive>
#include <QDebug>
#include <QFile>
#include <QDir>

using namespace Grantlee;

class NoEscapeStream : public OutputStream
{
public:
    NoEscapeStream();
    explicit NoEscapeStream (QTextStream* stream);

    virtual QString escape (const QString& input) const override;
    virtual QSharedPointer< OutputStream > clone (QTextStream* stream) const override;
};

NoEscapeStream::NoEscapeStream() : OutputStream()
{

}

NoEscapeStream::NoEscapeStream(QTextStream* stream) : OutputStream (stream)
{

}

QString NoEscapeStream::escape(const QString& input) const
{
    return input;
}

QSharedPointer<OutputStream> NoEscapeStream::clone(QTextStream* stream) const
{
    QSharedPointer<OutputStream> clonedStream = QSharedPointer<OutputStream>( new NoEscapeStream( stream ) );
    return clonedStream;
}

using namespace KDevelop;

namespace KDevelop {

class TemplateRendererPrivate
{
public:
    Engine* engine;
    Grantlee::Context context;
    TemplateRenderer::EmptyLinesPolicy emptyLinesPolicy;
    QString errorString;
};

}

TemplateRenderer::TemplateRenderer()
    : d(new TemplateRendererPrivate)
{
    d->engine = &TemplateEngine::self()->d->engine;
    d->emptyLinesPolicy = KeepEmptyLines;
}

TemplateRenderer::~TemplateRenderer()
{
    delete d;
}

void TemplateRenderer::addVariables(const QVariantHash& variables)
{
    QVariantHash::const_iterator it = variables.constBegin();
    QVariantHash::const_iterator end = variables.constEnd();
    for (; it != end; ++it)
    {
        d->context.insert(it.key(), it.value());
    }
}

void TemplateRenderer::addVariable(const QString& name, const QVariant& value)
{
    d->context.insert(name, value);
}

QVariantHash TemplateRenderer::variables() const
{
    return d->context.stackHash(0);
}

QString TemplateRenderer::render(const QString& content, const QString& name) const
{
    Template t = d->engine->newTemplate(content, name);

    QString output;
    QTextStream textStream(&output);
    NoEscapeStream stream(&textStream);
    t->render(&stream, &d->context);

    if (t->error() != Grantlee::NoError)
    {
        d->errorString = t->errorString();
    }
    else
    {
        d->errorString.clear();
    }

    if (d->emptyLinesPolicy == TrimEmptyLines && output.contains('\n'))
    {
        QStringList lines = output.split('\n', QString::KeepEmptyParts);
        QMutableStringListIterator it(lines);

        // Remove empty lines from the start of the document
        while (it.hasNext())
        {
            if (it.next().trimmed().isEmpty())
            {
                it.remove();
            }
            else
            {
                break;
            }
        }

        // Remove single empty lines
        it.toFront();
        bool prePreviousEmpty = false;
        bool previousEmpty = false;
        while (it.hasNext())
        {
            bool currentEmpty = it.peekNext().trimmed().isEmpty();
            if (!prePreviousEmpty && previousEmpty && !currentEmpty)
            {
                it.remove();
            }
            prePreviousEmpty = previousEmpty;
            previousEmpty = currentEmpty;
            it.next();
        }

        // Compress multiple empty lines
        it.toFront();
        previousEmpty = false;
        while (it.hasNext())
        {
            bool currentEmpty = it.next().trimmed().isEmpty();
            if (currentEmpty && previousEmpty)
            {
                it.remove();
            }
            previousEmpty = currentEmpty;
        }

        // Remove empty lines from the end
        it.toBack();
        while (it.hasPrevious())
        {
            if (it.previous().trimmed().isEmpty())
            {
                it.remove();
            }
            else
            {
                break;
            }
        }

        // Add a newline to the end of file
        it.toBack();
        it.insert(QString());

        output = lines.join("\n");
    }
    else if (d->emptyLinesPolicy == RemoveEmptyLines)
    {
        QStringList lines = output.split('\n', QString::SkipEmptyParts);
        QMutableStringListIterator it(lines);
        while (it.hasNext())
        {
            if (it.next().trimmed().isEmpty())
            {
                it.remove();
            }
        }
        it.toBack();
        if (lines.size() > 1)
        {
            it.insert(QString());
        }
        output = lines.join("\n");
    }

    return output;
}

QString TemplateRenderer::renderFile(const QUrl& url, const QString& name) const
{
    QFile file(url.toLocalFile());
    file.open(QIODevice::ReadOnly);

    QString content(file.readAll());
    qCDebug(LANGUAGE) << content;

    return render(content, name);
}

QStringList TemplateRenderer::render(const QStringList& contents) const
{
    qCDebug(LANGUAGE) << d->context.stackHash(0);
    QStringList ret;
    foreach (const QString& content, contents)
    {
        ret << render(content);
    }
    return ret;
}

void TemplateRenderer::setEmptyLinesPolicy(TemplateRenderer::EmptyLinesPolicy policy)
{
    d->emptyLinesPolicy = policy;
}

TemplateRenderer::EmptyLinesPolicy TemplateRenderer::emptyLinesPolicy() const
{
    return d->emptyLinesPolicy;
}

DocumentChangeSet TemplateRenderer::renderFileTemplate(const SourceFileTemplate& fileTemplate,
                                                       const QUrl& baseUrl,
                                                       const QHash<QString, QUrl>& fileUrls)
{
    DocumentChangeSet changes;
    const QDir baseDir(baseUrl.path());

    QRegExp nonAlphaNumeric("\\W");
    for (QHash<QString,QUrl>::const_iterator it = fileUrls.constBegin(); it != fileUrls.constEnd(); ++it)
    {
        QString cleanName = it.key().toLower();
        cleanName.replace(nonAlphaNumeric, "_");
        const QString path = it.value().toLocalFile();
        addVariable("output_file_" + cleanName, baseDir.relativeFilePath(path));
        addVariable("output_file_" + cleanName + "_absolute", path);
    }

    const KArchiveDirectory* directory = fileTemplate.directory();
    ArchiveTemplateLocation location(directory);
    foreach (const SourceFileTemplate::OutputFile& outputFile, fileTemplate.outputFiles())
    {
        const KArchiveEntry* entry = directory->entry(outputFile.fileName);
        if (!entry)
        {
            qCWarning(LANGUAGE) << "Entry" << outputFile.fileName << "is mentioned in group" << outputFile.identifier << "but is not present in the archive";
            continue;
        }

        const KArchiveFile* file = dynamic_cast<const KArchiveFile*>(entry);
        if (!file)
        {
            qCWarning(LANGUAGE) << "Entry" << entry->name() << "is not a file";
            continue;
        }

        QUrl url = fileUrls[outputFile.identifier];
        IndexedString document(url);
        KTextEditor::Range range(KTextEditor::Cursor(0, 0), 0);

        DocumentChange change(document, range, QString(), render(file->data(), outputFile.identifier));
        changes.addChange(change);
        qCDebug(LANGUAGE) << "Added change for file" << document.str();
    }

    return changes;
}

QString TemplateRenderer::errorString() const
{
    return d->errorString;
}
