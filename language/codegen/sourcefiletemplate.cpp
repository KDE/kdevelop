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

#include "sourcefiletemplate.h"
#include "templaterenderer.h"
#include "util/debug.h"

#include <interfaces/icore.h>

#include <KArchive>
#include <KConfig>
#include <KZip>
#include <KTar>
#include <KConfigGroup>

#include <QFileInfo>
#include <QDomDocument>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

using namespace KDevelop;
typedef SourceFileTemplate::ConfigOption ConfigOption;

class KDevelop::SourceFileTemplatePrivate
{
public:
    KArchive* archive;
    QString descriptionFileName;
    QStringList searchLocations;

    ConfigOption readEntry(const QDomElement& element, TemplateRenderer* renderer);
};

ConfigOption SourceFileTemplatePrivate::readEntry(const QDomElement& element,
                                                  TemplateRenderer* renderer)
{
    ConfigOption entry;

    entry.name = element.attribute("name");
    entry.type = element.attribute("type", "String");

    for (QDomElement e = element.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
    {
        QString tag = e.tagName();

        if (tag == "label")
        {
            entry.label = e.text();
        }
        else if (tag == "tooltip")
        {
            entry.label = e.text();
        }
        else if (tag == "whatsthis")
        {
            entry.label = e.text();
        }
        else if ( tag == "min" )
        {
            entry.minValue = e.text();
        }
        else if ( tag == "max" )
        {
            entry.maxValue = e.text();
        }
        else if ( tag == "default" )
        {
            entry.value = renderer->render(e.text(), entry.name);
        }
    }

    qCDebug(LANGUAGE) << "Read entry" << entry.name << "with default value" << entry.value;
    return entry;
}


SourceFileTemplate::SourceFileTemplate (const QString& templateDescription)
: d(new KDevelop::SourceFileTemplatePrivate)
{
    d->archive = 0;
    setTemplateDescription(templateDescription);
}

SourceFileTemplate::SourceFileTemplate()
: d(new KDevelop::SourceFileTemplatePrivate)
{
    d->archive = 0;
}

SourceFileTemplate::SourceFileTemplate (const SourceFileTemplate& other)
: d(new KDevelop::SourceFileTemplatePrivate)
{
    d->archive = 0;
    *this = other;
}

SourceFileTemplate::~SourceFileTemplate()
{
    delete d->archive;
    delete d;
}

SourceFileTemplate& SourceFileTemplate::operator=(const SourceFileTemplate& other)
{
    if (other.d == d) {
        return *this;
    }

    delete d->archive;
    if (other.d->archive) {
        if (other.d->archive->fileName().endsWith(".zip")) {
            d->archive = new KZip(other.d->archive->fileName());
        } else {
            d->archive = new KTar(other.d->archive->fileName());
        }
        d->archive->open(QIODevice::ReadOnly);
    } else {
        d->archive = 0;
    }
    d->descriptionFileName = other.d->descriptionFileName;
    return *this;
}

void SourceFileTemplate::setTemplateDescription(const QString& templateDescription, const QString& resourcePrefix)
{
    delete d->archive;

    d->descriptionFileName = templateDescription;
    QString archiveFileName;

    const QString templateBaseName = QFileInfo(templateDescription).baseName();

    d->searchLocations.append(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "/kdevfiletemplates/templates/", QStandardPaths::LocateDirectory));

    foreach(const QString& dir, d->searchLocations) {
        qDebug() << "search in:" << dir << "look for:" << templateBaseName;
        foreach(const auto& entry, QDir(dir).entryInfoList(QDir::Files)) {
            qDebug() << entry.baseName();
            if (entry.baseName() == templateBaseName) {
                archiveFileName = entry.absoluteFilePath();
                qCDebug(LANGUAGE) << "Found template archive" << archiveFileName;
                break;
            }
        }
    }

    if (archiveFileName.isEmpty() || !QFileInfo(archiveFileName).exists()) {
        qCWarning(LANGUAGE) << "Could not find a template archive for description" << templateDescription << ", archive file" << archiveFileName;
        d->archive = 0;
    } else {
        QFileInfo info(archiveFileName);

        if (info.suffix() == ".zip") {
            d->archive = new KZip(archiveFileName);
        } else {
            d->archive = new KTar(archiveFileName);
        }
        d->archive->open(QIODevice::ReadOnly);
    }
}

bool SourceFileTemplate::isValid() const
{
    return d->archive;
}

QString SourceFileTemplate::name() const
{
    KConfig templateConfig(d->descriptionFileName);
    KConfigGroup cg(&templateConfig, "General");
    return cg.readEntry("Name");
}

QString SourceFileTemplate::type() const
{
    KConfig templateConfig(d->descriptionFileName);
    KConfigGroup cg(&templateConfig, "General");
    return cg.readEntry("Type", QString());
}

QString SourceFileTemplate::languageName() const
{
    KConfig templateConfig(d->descriptionFileName);
    KConfigGroup cg(&templateConfig, "General");
    return cg.readEntry("Language", QString());
}

QStringList SourceFileTemplate::category() const
{
    KConfig templateConfig(d->descriptionFileName);
    KConfigGroup cg(&templateConfig, "General");
    return cg.readEntry("Category", QStringList());
}

QStringList SourceFileTemplate::defaultBaseClasses() const
{
    KConfig templateConfig(d->descriptionFileName);
    KConfigGroup cg(&templateConfig, "General");
    return cg.readEntry("BaseClasses", QStringList());
}

const KArchiveDirectory* SourceFileTemplate::directory() const
{
    Q_ASSERT(isValid());
    return d->archive->directory();
}

QList< SourceFileTemplate::OutputFile > SourceFileTemplate::outputFiles() const
{
    QList<SourceFileTemplate::OutputFile> outputFiles;

    KConfig templateConfig(d->descriptionFileName);
    KConfigGroup group(&templateConfig, "General");

    QStringList files = group.readEntry("Files", QStringList());
    qCDebug(LANGUAGE) << "Files in template" << files;
    foreach (const QString& fileGroup, files)
    {
        KConfigGroup cg(&templateConfig, fileGroup);
        OutputFile f;
        f.identifier = cg.name();
        f.label = cg.readEntry("Name");
        f.fileName = cg.readEntry("File");
        f.outputName = cg.readEntry("OutputFile");
        outputFiles << f;
    }

    return outputFiles;
}

bool SourceFileTemplate::hasCustomOptions() const
{
    Q_ASSERT(isValid());

    KConfig templateConfig(d->descriptionFileName);
    KConfigGroup cg(&templateConfig, "General");
    bool hasOptions = d->archive->directory()->entries().contains(cg.readEntry("OptionsFile", "options.kcfg"));

    qCDebug(LANGUAGE) << cg.readEntry("OptionsFile", "options.kcfg") << hasOptions;
    return hasOptions;
}

QHash< QString, QList<ConfigOption> > SourceFileTemplate::customOptions(TemplateRenderer* renderer) const
{
    Q_ASSERT(isValid());

    KConfig templateConfig(d->descriptionFileName);
    KConfigGroup cg(&templateConfig, "General");
    const KArchiveEntry* entry = d->archive->directory()->entry(cg.readEntry("OptionsFile", "options.kcfg"));

    QHash<QString, QList<ConfigOption> > options;

    if (!entry->isFile())
    {
        return options;
    }
    const KArchiveFile* file = static_cast<const KArchiveFile*>(entry);

    /*
     * Copied from kconfig_compiler.kcfg
     */
    QDomDocument doc;
    QString errorMsg;
    int errorRow;
    int errorCol;
    if ( !doc.setContent( file->data(), &errorMsg, &errorRow, &errorCol ) ) {
        qCDebug(LANGUAGE) << "Unable to load document.";
        qCDebug(LANGUAGE) << "Parse error in line " << errorRow << ", col " << errorCol << ": " << errorMsg;
        return options;
    }

    QDomElement cfgElement = doc.documentElement();
    if ( cfgElement.isNull() ) {
        qCDebug(LANGUAGE) << "No document in kcfg file";
        return options;
    }

    QDomNodeList groups = cfgElement.elementsByTagName("group");
    for (int i = 0; i < groups.size(); ++i)
    {
        QDomElement group = groups.at(i).toElement();
        QList<ConfigOption> optionGroup;
        QString groupName = group.attribute("name");

        QDomNodeList entries = group.elementsByTagName("entry");
        for (int j = 0; j < entries.size(); ++j)
        {
            QDomElement entry = entries.at(j).toElement();
            optionGroup << d->readEntry(entry, renderer);
        }

        options.insert(groupName, optionGroup);
    }
    return options;
}

void SourceFileTemplate::addAdditionalSearchLocation(const QString& location)
{
    if(!d->searchLocations.contains(location))
        d->searchLocations.append(location);
}
