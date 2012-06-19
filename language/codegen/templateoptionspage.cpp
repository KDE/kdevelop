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

#include "templateoptionspage.h"
#include "templateclassassistant.h"
#include "templateclassgenerator.h"

#include <KLineEdit>
#include <KIntNumInput>

#include <QDomElement>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>

using namespace KDevelop;

struct ConfigEntry
{
    QString name;
    QString label;
    QVariant value;
    QString context;

    QString maxValue;
    QString minValue;
    QString type;
};

class KDevelop::TemplateOptionsPagePrivate
{
public:
    TemplateClassAssistant* assistant;
    QList<ConfigEntry> entries;
    QHash<QString, QWidget*> controls;
    QHash<QString, QByteArray> typeProperties;

    ConfigEntry readEntry(const QDomElement& element, QWidget* parent, QFormLayout* layout);
};

ConfigEntry TemplateOptionsPagePrivate::readEntry(const QDomElement& element, QWidget* parent, QFormLayout* layout)
{
    ConfigEntry entry;

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
            TemplateClassGenerator* gen = dynamic_cast<TemplateClassGenerator*>(assistant->generator());
            entry.value = gen->renderString(e.text());
        }
    }

    kDebug() << "Read entry" << entry.name << "with default value" << entry.value;

    QLabel* label = new QLabel(entry.label, parent);
    QWidget* control = 0;
    const QString type = entry.type;
    if (type == "String")
    {
        control = new KLineEdit(entry.value.toString(), parent);
    }
    else if (type == "Int")
    {
        KIntNumInput* input = new KIntNumInput(entry.value.toInt(), parent);
        if (!entry.minValue.isEmpty())
        {
            input->setMinimum(entry.minValue.toInt());
        }
        if (!entry.maxValue.isEmpty())
        {
            input->setMaximum(entry.maxValue.toInt());
        }
        control = input;
    }
    else if (type == "Bool")
    {
        bool checked = (QString::compare(entry.value.toString(), "true", Qt::CaseInsensitive) == 0);
        QCheckBox* checkBox = new QCheckBox(entry.label, parent);
        checkBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
    else
    {
        kDebug() << "Unrecognized option type" << entry.type;
    }

    if (control)
    {
        layout->addRow(label, control);
        entries << entry;
        controls.insert(entry.name, control);
    }

    return entry;
}

TemplateOptionsPage::TemplateOptionsPage(TemplateClassAssistant* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, d(new TemplateOptionsPagePrivate)
{
    d->assistant = parent;

    d->typeProperties.insert("String", "text");
    d->typeProperties.insert("Int", "value");
    d->typeProperties.insert("Bool", "checked");
}

TemplateOptionsPage::~TemplateOptionsPage()
{
    delete d;
}

void TemplateOptionsPage::loadXML(const QByteArray& contents)
{
    /*
     * Copied from kconfig_compiler.kcfg
     */

    QLayout* layout = new QVBoxLayout();

    QDomDocument doc;
    QString errorMsg;
    int errorRow;
    int errorCol;
    if ( !doc.setContent( contents, &errorMsg, &errorRow, &errorCol ) ) {
        kDebug() << "Unable to load document.";
        kDebug() << "Parse error in line " << errorRow << ", col " << errorCol << ": " << errorMsg;
        return;
    }

    QDomElement cfgElement = doc.documentElement();
    if ( cfgElement.isNull() ) {
        kDebug() << "No document in kcfg file";
        return;
    }

    QDomNodeList groups = cfgElement.elementsByTagName("group");
    for (int i = 0; i < groups.size(); ++i)
    {
        QDomElement group = groups.at(i).toElement();

        QGroupBox* box = new QGroupBox(this);
        box->setTitle(group.attribute("name"));

        QFormLayout* formLayout = new QFormLayout;

        QDomNodeList entries = group.elementsByTagName("entry");
        for (int j = 0; j < entries.size(); ++j)
        {
            QDomElement entry = entries.at(j).toElement();
            ConfigEntry cfgEntry = d->readEntry(entry, box, formLayout);
        }

        box->setLayout(formLayout);
        layout->addWidget(box);
    }
    setLayout(layout);
}

QVariantHash TemplateOptionsPage::templateOptions() const
{
    QVariantHash values;

    foreach (const ConfigEntry& entry, d->entries)
    {
        Q_ASSERT(d->controls.contains(entry.name));
        Q_ASSERT(d->typeProperties.contains(entry.type));

        values.insert(entry.name, d->controls[entry.name]->property(d->typeProperties[entry.type]));
    }

    kDebug() << values.size() << d->entries.size();

    return values;
}
