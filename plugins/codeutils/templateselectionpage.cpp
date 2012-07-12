/*
 *
 */

#include "templateselectionpage.h"
#include "ui_templateselectionpage.h"

#include <language/codegen/templatesmodel.h>
#include <interfaces/icore.h>

#include <KComponentData>

TemplateSelectionPage::TemplateSelectionPage (QWidget* parent)
: QWidget(parent)
{
    ui = new Ui::TemplateSelectionPage;
    ui->setupUi(this);

    m_model = new KDevelop::TemplatesModel(KDevelop::ICore::self()->componentData(), this);
    m_model->setDescriptionResourceType("filetemplate_descriptions");
    m_model->setTemplateResourceType("filetemplates");
    m_model->refresh();

    ui->treeView->setModel(m_model);
    connect (ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(contentsChanged()));
    connect (ui->identifierLineEdit, SIGNAL(textChanged(QString)), SLOT(contentsChanged()));
    connect (ui->directoryUrlRequester, SIGNAL(textChanged(QString)), SLOT(contentsChanged()));
}

TemplateSelectionPage::~TemplateSelectionPage()
{
    delete ui;
}

QString TemplateSelectionPage::selectedTemplate() const
{
    QStandardItem* item = m_model->itemFromIndex(ui->treeView->selectionModel()->currentIndex());
    return item->data(KDevelop::TemplatesModel::DescriptionFileRole).toString();
}

void TemplateSelectionPage::contentsChanged()
{
    QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    QStandardItem* item = m_model->itemFromIndex(index);
    bool valid = item && !item->hasChildren() && !item->data(KDevelop::TemplatesModel::DescriptionFileRole).toString().isEmpty();
    valid = valid && !ui->identifierLineEdit->text().isEmpty() && ui->directoryUrlRequester->url().isValid();
    emit templateValid(valid);
}

QString TemplateSelectionPage::identifier() const
{
    return ui->identifierLineEdit->text();
}

KUrl TemplateSelectionPage::baseUrl() const
{
    return ui->directoryUrlRequester->url();
}

void TemplateSelectionPage::setBaseUrl (const KUrl& url)
{
    ui->directoryUrlRequester->setUrl(url);
}

