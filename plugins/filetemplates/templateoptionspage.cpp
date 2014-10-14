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
#include "debug.h"

#include <language/codegen/templateclassgenerator.h>
#include <language/codegen/sourcefiletemplate.h>

#include <KLineEdit>
#include <KIntNumInput>

#include <QDomElement>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>

using namespace KDevelop;

class KDevelop::TemplateOptionsPagePrivate
{
public:
    QList<SourceFileTemplate::ConfigOption> entries;
    QHash<QString, QWidget*> controls;
    QHash<QString, QByteArray> typeProperties;
};

TemplateOptionsPage::TemplateOptionsPage(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, d(new TemplateOptionsPagePrivate)
{
    d->typeProperties.insert("String", "text");
    d->typeProperties.insert("Int", "value");
    d->typeProperties.insert("Bool", "checked");
}

TemplateOptionsPage::~TemplateOptionsPage()
{
    delete d;
}

void TemplateOptionsPage::load(const SourceFileTemplate& fileTemplate, TemplateRenderer* renderer)
{
    d->entries.clear();

    QLayout* layout = new QVBoxLayout();
    QHash<QString, QList<SourceFileTemplate::ConfigOption> > options = fileTemplate.customOptions(renderer);
    QHash<QString, QList<SourceFileTemplate::ConfigOption> >::const_iterator it;

    for (it = options.constBegin(); it != options.constEnd(); ++it)
    {
        QGroupBox* box = new QGroupBox(this);
        box->setTitle(it.key());

        QFormLayout* formLayout = new QFormLayout;

        d->entries << it.value();
        foreach (const SourceFileTemplate::ConfigOption& entry, it.value())
        {
            QLabel* label = new QLabel(entry.label, box);
            QWidget* control = 0;
            const QString type = entry.type;
            if (type == "String")
            {
                control = new KLineEdit(entry.value.toString(), box);
            }
            else if (type == "Int")
            {
                KIntNumInput* input = new KIntNumInput(entry.value.toInt(), box);
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
                QCheckBox* checkBox = new QCheckBox(entry.label, box);
                checkBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
            }
            else
            {
                qCDebug(PLUGIN_FILETEMPLATES) << "Unrecognized option type" << entry.type;
            }
            if (control)
            {
                formLayout->addRow(label, control);
                d->controls.insert(entry.name, control);
            }
        }

        box->setLayout(formLayout);
        layout->addWidget(box);
    }
    setLayout(layout);
}

QVariantHash TemplateOptionsPage::templateOptions() const
{
    QVariantHash values;

    foreach (const SourceFileTemplate::ConfigOption& entry, d->entries)
    {
        Q_ASSERT(d->controls.contains(entry.name));
        Q_ASSERT(d->typeProperties.contains(entry.type));

        values.insert(entry.name, d->controls[entry.name]->property(d->typeProperties[entry.type]));
    }

    qCDebug(PLUGIN_FILETEMPLATES) << values.size() << d->entries.size();

    return values;
}
