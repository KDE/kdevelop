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

#include "templateclassassistant.h"
#include "templatesmodel.h"
#include "templateclassgenerator.h"

#include <interfaces/icreateclasshelper.h>
#include <interfaces/ilanguagesupport.h>
#include "interfaces/icore.h"
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguage.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

#include "ui_templateselection.h"
#include "ui_classmembers.h"

#include <KNS3/DownloadDialog>
#include <KLocalizedString>
#include <KComponentData>
#include <KFileDialog>
#include <KLineEdit>
#include <KIntNumInput>
#include <KMessageBox>

#include <QDomElement>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>

using namespace KDevelop;

const char* LastUsedTemplateEntry = "LastUsedTemplate";
const char* ClassTemplatesGroup = "ClassTemplates";

class KDevelop::TemplateSelectionPagePrivate
{
public:
    Ui::TemplateSelection* ui;
    QString selectedTemplate;
    TemplateClassAssistant* assistant;
    TemplatesModel* model;
};

class KDevelop::TemplateClassAssistantPrivate
{
public:
    KPageWidgetItem* templateSelectionPage;
    KPageWidgetItem* dummyPage;
    KPageWidgetItem* templateOptionsPage;
    KPageWidgetItem* membersPage;
    ICreateClassHelper* helper;
};

TemplateSelectionPage::TemplateSelectionPage (TemplateClassAssistant* parent, Qt::WindowFlags f)
: QWidget (parent, f)
, d(new TemplateSelectionPagePrivate)
{
    d->assistant = parent;
    
    d->ui = new Ui::TemplateSelection;
    d->ui->setupUi(this);
    
    d->model = new TemplatesModel(ICore::self()->componentData());
    d->model->setTemplateResourceType("filetemplates");
    d->model->setDescriptionResourceType("filetemplate_descriptions");
    d->model->refresh();
    
    d->ui->languageView->setModel(d->model);
    d->ui->templateView->setModel(d->model);
    
    connect (d->ui->languageView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), 
        this, SLOT(currentLanguageChanged(QModelIndex)));
    
    connect (d->ui->templateView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), 
        this, SLOT(currentTemplateChanged(QModelIndex)));
    
    connect (d->ui->getMoreButton, SIGNAL(clicked(bool)), this, SLOT(getMoreClicked()));
    connect (d->ui->loadFileButton, SIGNAL(clicked(bool)), this, SLOT(loadFileClicked()));
    
    QModelIndex languageIndex = d->model->index(0, 0);
    QModelIndex templateIndex = d->model->index(0, 0, languageIndex);
    
    while (templateIndex.child(0, 0).isValid())
    {
        templateIndex = templateIndex.child(0, 0);
    }
    
    IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl());
    if (project)
    {
        KConfigGroup group(project->projectConfiguration(), ClassTemplatesGroup);
        QString lastTemplate = group.readEntry(LastUsedTemplateEntry);
        
        QModelIndexList indexes = d->model->match(d->model->index(0, 0), TemplatesModel::DescriptionFileRole, lastTemplate);
        
        if (!indexes.isEmpty())
        {
            templateIndex = indexes.first();
            QStandardItem* item = d->model->itemFromIndex(templateIndex);
            
            while (item->parent() && item->parent() != d->model->invisibleRootItem())
            {
                item = item->parent();
            }
            languageIndex = item->index();
        }
    }
    
    d->ui->languageView->setCurrentIndex(languageIndex);
    d->ui->templateView->setCurrentIndex(templateIndex);
}

TemplateSelectionPage::~TemplateSelectionPage()
{
    delete d->ui;
    delete d;
}

QString TemplateSelectionPage::selectedTemplate() const
{
    return d->selectedTemplate;
}

void TemplateSelectionPage::currentLanguageChanged (const QModelIndex& index)
{
    d->ui->templateView->setRootIndex(index);
    d->ui->templateView->setCurrentIndex(d->model->index(0, 0, index));
}

void TemplateSelectionPage::currentTemplateChanged (const QModelIndex& index)
{
    if (!index.isValid() || d->model->index(0, 0, index).isValid())
    {
        // This index is invalid or has a child, so it is not a template
        d->assistant->setValid(d->assistant->currentPage(), false);
    }
    else
    {
        d->selectedTemplate = d->model->data(index, TemplatesModel::DescriptionFileRole).toString();
        d->assistant->setValid(d->assistant->currentPage(), true);
    }
}

void TemplateSelectionPage::getMoreClicked()
{
    KNS3::DownloadDialog dialog("kdevclassassistant.knsrc");
    dialog.exec();
    d->model->refresh();
}

void TemplateSelectionPage::loadFileClicked()
{
    QString filter = "application/x-desktop application/x-bzip-compressed-tar application/zip";
    QString fileName = KFileDialog::getOpenFileName(KUrl("kfiledialog:///kdevclasstemplate"), filter, this);

    if (!fileName.isEmpty())
    {
        QString destination = d->model->loadTemplateFile(fileName);
        QModelIndexList indexes = d->model->templateIndexes(destination);
        if (indexes.size() > 1)
        {
            d->ui->languageView->setCurrentIndex(indexes.first());
            d->ui->templateView->setCurrentIndex(indexes.last());
        }
    }
}

void TemplateSelectionPage::saveConfig()
{
    IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl());
    if (project)
    {
        KConfigGroup group(project->projectConfiguration(), ClassTemplatesGroup);
        group.writeEntry(LastUsedTemplateEntry, selectedTemplate());
        group.sync();
    }
}

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

class KDevelop::ClassMembersPagePrivate
{
public:
    TemplateClassAssistant* parent;
    ClassDescriptionModel* model;
    Ui::ClassMembersPage* ui;
};

ClassMembersPage::ClassMembersPage(TemplateClassAssistant* parent)
: QWidget(parent)
, d(new ClassMembersPagePrivate)
{
    d->model = 0;

    d->ui = new Ui::ClassMembersPage;
    d->ui->setupUi(this);
    
    connect (d->ui->topButton, SIGNAL(clicked(bool)), SLOT(moveTop()));
    connect (d->ui->upButton, SIGNAL(clicked(bool)), SLOT(moveUp()));
    connect (d->ui->downButton, SIGNAL(clicked(bool)), SLOT(moveDown()));
    connect (d->ui->bottomButton, SIGNAL(clicked(bool)), SLOT(moveBottom()));
    
    connect (d->ui->addItemButton, SIGNAL(clicked(bool)), SLOT(addItem()));
    connect (d->ui->removeItemButton, SIGNAL(clicked(bool)), SLOT(removeItem()));
}

ClassMembersPage::~ClassMembersPage()
{

}

void ClassMembersPage::setDescription(const ClassDescription& description)
{
    KMessageBox::information(this, "Setting description and model");
    d->model = new ClassDescriptionModel(description, this);
    d->ui->itemView->setModel(d->model);
    d->ui->itemView->setRootIndex(d->model->index(ClassDescriptionModel::MembersRow, 0));
    
    connect (d->ui->itemView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(currentSelectionChanged(QItemSelection)));
    
    currentSelectionChanged(QItemSelection());
}

ClassDescription ClassMembersPage::description() const
{
    Q_ASSERT(d->model);
    return d->model->description();
}

void ClassMembersPage::currentSelectionChanged(const QItemSelection& current)
{
    bool up = false;
    bool down = false;
    bool remove = false;
    
    if (!current.indexes().isEmpty())
    {
        up = current.indexes().first().row() > 0;
        down = current.indexes().first().row() < rows()-1;
        remove = true;
    }
    
    d->ui->topButton->setEnabled(up);
    d->ui->upButton->setEnabled(up);
    d->ui->downButton->setEnabled(down);
    d->ui->bottomButton->setEnabled(down);
    
    d->ui->removeItemButton->setEnabled(remove);
}

int ClassMembersPage::rows()
{
    return d->model->rowCount(d->ui->itemView->rootIndex());
}


void ClassMembersPage::moveRowTo(int destination, bool relative)
{
    Q_ASSERT(d->model);
    Q_ASSERT(d->ui->itemView->selectionModel());
    QModelIndexList indexes = d->ui->itemView->selectionModel()->selectedRows();
    
    if (indexes.isEmpty())
    {
        return;
    }
    
    int source = indexes.first().row();
    
    if (relative)
    {
        destination = source + destination;
    }
    
    d->model->moveRow(source, destination, d->ui->itemView->rootIndex());
    d->ui->itemView->setCurrentIndex(d->model->index(destination, 0, d->ui->itemView->rootIndex()));
}

void ClassMembersPage::moveTop()
{
    moveRowTo(0, false);
}

void ClassMembersPage::moveBottom()
{
    moveRowTo(d->model->rowCount(d->ui->itemView->rootIndex())-1, false);
}

void ClassMembersPage::moveUp()
{
    moveRowTo(-1, true);
}

void ClassMembersPage::moveDown()
{
    moveRowTo(1, true);
}

void ClassMembersPage::addItem()
{
    Q_ASSERT(d->model);
    d->model->insertRow(0, d->ui->itemView->rootIndex());
}

void ClassMembersPage::removeItem()
{
    Q_ASSERT(d->model);
    QModelIndexList indexes = d->ui->itemView->selectionModel()->selectedRows();
    if (indexes.isEmpty())
    {
        return;
    }

    d->model->removeRow(indexes.first().row(), d->ui->itemView->rootIndex());
}



TemplateClassAssistant::TemplateClassAssistant (QWidget* parent, const KUrl& baseUrl)
: CreateClassAssistant (parent, baseUrl)
, d(new TemplateClassAssistantPrivate)
{
    d->templateOptionsPage = 0;
    setup();
}

TemplateClassAssistant::~TemplateClassAssistant()
{
    delete d;
}

void TemplateClassAssistant::setup()
{
    setWindowTitle(i18n("Create New Class in %1", baseUrl().prettyUrl()));

    TemplateSelectionPage* page = newTemplateSelectionPage();
    d->templateSelectionPage = addPage(page, i18n("Language and Template"));
    connect (this, SIGNAL(accepted()), page, SLOT(saveConfig()));
    
    /*
     * All assistant pages except the first one require the helper to already be set.
     * However, we can only choose the helper aften the language is selected,
     * so other pages cannot be loaded here yet. 
     * 
     * OTOH, having only one page disables the "next" button and enables the "finish" button.
     * This is not wanted, so we create a dummy page and delete it when "next" is clicked
     */
    QWidget* dummy = new QWidget(this);
    d->dummyPage = addPage(dummy, QLatin1String("Dummy Page"));
    
    setCurrentPage(d->templateSelectionPage);
}

void TemplateClassAssistant::next()
{
    if (currentPage() == d->templateSelectionPage)
    {
        kDebug() << "Current page is template selection";
        QString description = currentPage()->widget()->property("selectedTemplate").toString();
        
        kDebug() << "Chosen template is" << description;
        
        KConfig config(description);
        KConfigGroup group(&config, "General");
        
        kDebug() << "Template name is" << group.readEntry("Name");
        
        QString languageName = group.readEntry("Category").split('/').first();
        
        ILanguage* language = ICore::self()->languageController()->language(languageName);
        
        if (!language)
        {
            kDebug() << "No language named" << languageName;
            return;
        }
        
        d->helper = language->languageSupport()->createClassHelper(this);
        
        if (!d->helper)
        {
            kDebug() << "No class creation helper for language" << languageName;
            return;
        }
        
        ClassGenerator* generator = d->helper->generator();
        if (!generator)
        {
            kDebug() << "No generator for language" << languageName;
            return;
        }
        
        
        setGenerator(generator);
        
        TemplateClassGenerator* templateGenerator = dynamic_cast<TemplateClassGenerator*>(generator);
        if (templateGenerator)
        {
            kDebug() << "Class generator uses templates";
            templateGenerator->setTemplateDescription(description);
        }
        
        removePage(d->dummyPage);
        KDevelop::CreateClassAssistant::setup();
        
        ClassMembersPage* membersPage = new ClassMembersPage(this);
        d->membersPage = addPage(membersPage, i18n("Data Members"));
        
        if (templateGenerator && templateGenerator->hasCustomOptions())
        {
            kDebug() << "Class generator has custom options";
            TemplateOptionsPage* options = new TemplateOptionsPage(this);
            d->templateOptionsPage = addPage(options, i18n("Template Options"));
            connect (this, SIGNAL(accepted()), this, SLOT(updateTemplateOptions()));
        }
        
        return;
    }
    
    KDevelop::CreateClassAssistant::next();

    if (currentPage() == d->membersPage)
    {
        ClassDescription desc(generator()->name());
        foreach (DeclarationPointer declaration, generator()->declarations())
        {
            desc.methods << FunctionDescription(declaration);
        }
        desc.members << VariableDescription("Sample type", "Sample name");
        
        qobject_cast<ClassMembersPage*>(d->membersPage->widget())->setDescription(desc);
    }
    
    if (d->templateOptionsPage && (currentPage() == d->templateOptionsPage))
    {
        TemplateOptionsPage* options = qobject_cast<TemplateOptionsPage*>(d->templateOptionsPage->widget());
        TemplateClassGenerator* templateGenerator = dynamic_cast<TemplateClassGenerator*>(generator());

        options->loadXML(templateGenerator->customOptions());
    }
}

void TemplateClassAssistant::accept()
{
    if (d->templateOptionsPage)
    {
        TemplateClassGenerator* templateGenerator = dynamic_cast<TemplateClassGenerator*>(generator());
        Q_ASSERT(templateGenerator);
        
        kDebug() << d->templateOptionsPage->widget()->property("templateOptions");
        kDebug() << d->templateOptionsPage->widget()->property("templateOptions").toHash();
        templateGenerator->addVariables(d->templateOptionsPage->widget()->property("templateOptions").toHash());
    }
    
    ClassDescription desc = d->membersPage->widget()->property("description").value<ClassDescription>();
    generator()->setDescription(desc);
    
    CreateClassAssistant::accept();
}


void TemplateClassAssistant::updateTemplateOptions()
{
    
}

TemplateSelectionPage* TemplateClassAssistant::newTemplateSelectionPage()
{
    return new TemplateSelectionPage(this);
}

ClassIdentifierPage* TemplateClassAssistant::newIdentifierPage()
{
    return d->helper->identifierPage();
}

OverridesPage* TemplateClassAssistant::newOverridesPage()
{
    return d->helper->overridesPage();
}

ClassMembersPage* TemplateClassAssistant::newMembersPage()
{
    return new ClassMembersPage(this);
}

