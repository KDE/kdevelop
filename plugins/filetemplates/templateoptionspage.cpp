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
#include "qtcompat_p.h"

#include <language/codegen/sourcefiletemplate.h>

#include <KLocalizedString>

#include <QLineEdit>
#include <QSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>

using namespace KDevelop;

class KDevelop::TemplateOptionsPagePrivate
{
public:
    QVector<SourceFileTemplate::ConfigOption> entries;
    QHash<QString, QWidget*> controls;
    QHash<QString, QByteArray> typeProperties;
    QWidget *firstEditWidget;
    QList<QWidget*> groupBoxes;
};

TemplateOptionsPage::TemplateOptionsPage(QWidget* parent)
: QWidget(parent)
, d(new TemplateOptionsPagePrivate)
{
    d->firstEditWidget = nullptr;

    d->typeProperties.insert(QStringLiteral("String"), "text");
    d->typeProperties.insert(QStringLiteral("Enum"), "currentText");
    d->typeProperties.insert(QStringLiteral("Int"), "value");
    d->typeProperties.insert(QStringLiteral("Bool"), "checked");
}

TemplateOptionsPage::~TemplateOptionsPage()
{
    delete d;
}

void TemplateOptionsPage::load(const SourceFileTemplate& fileTemplate, TemplateRenderer* renderer)
{
    // TODO: keep any old changed values, as it comes by surprise to have them lost
    // when going back and forward

    // clear anything as there is on reentering the page
    d->entries.clear();
    d->controls.clear();
    // clear any old option group boxes & the base layout
    d->firstEditWidget = nullptr;
    qDeleteAll(d->groupBoxes);
    d->groupBoxes.clear();
    delete layout();

    auto* layout = new QVBoxLayout();

    const auto customOptions = fileTemplate.customOptions(renderer);
    d->groupBoxes.reserve(customOptions.size());
    d->entries.reserve(customOptions.size());
    for (const auto& optionGroup : customOptions) {
        auto* box = new QGroupBox(this);
        d->groupBoxes.append(box);

        box->setTitle(optionGroup.name);

        auto* formLayout = new QFormLayout;

        d->entries << optionGroup.options;
        for (const auto& entry : optionGroup.options) {
            QWidget* control = nullptr;
            const QString type = entry.type;
            if (type == QLatin1String("String"))
            {
                control = new QLineEdit(entry.value.toString(), box);
            }
            else if (type == QLatin1String("Enum"))
            {
                auto input = new QComboBox(box);
                input->addItems(entry.values);
                input->setCurrentText(entry.value.toString());
                control = input;
            }
            else if (type == QLatin1String("Int"))
            {
                auto input = new QSpinBox(box);
                input->setValue(entry.value.toInt());
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
            else if (type == QLatin1String("Bool"))
            {
                bool checked = (QString::compare(entry.value.toString(), QStringLiteral("true"), Qt::CaseInsensitive) == 0);
                auto* checkBox = new QCheckBox(box);
                checkBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
                control = checkBox;
            }
            else
            {
                qCDebug(PLUGIN_FILETEMPLATES) << "Unrecognized option type" << entry.type;
            }
            if (control)
            {
                const QString entryLabelText = i18n("%1:", entry.label);
                QLabel* label = new QLabel(entryLabelText, box);
                formLayout->addRow(label, control);
                d->controls.insert(entry.name, control);
                if (d->firstEditWidget == nullptr) {
                    d->firstEditWidget = control;
                }
            }
        }

        box->setLayout(formLayout);
        layout->addWidget(box);
    }

    layout->addStretch();

    setLayout(layout);
}

QVariantHash TemplateOptionsPage::templateOptions() const
{
    QVariantHash values;

    for (const SourceFileTemplate::ConfigOption& entry : qAsConst(d->entries)) {
        Q_ASSERT(d->controls.contains(entry.name));
        Q_ASSERT(d->typeProperties.contains(entry.type));

        values.insert(entry.name, d->controls[entry.name]->property(d->typeProperties[entry.type].constData()));
    }

    qCDebug(PLUGIN_FILETEMPLATES) << values.size() << d->entries.size();

    return values;
}

void TemplateOptionsPage::setFocusToFirstEditWidget()
{
    if (d->firstEditWidget) {
        d->firstEditWidget->setFocus();
    }
}
