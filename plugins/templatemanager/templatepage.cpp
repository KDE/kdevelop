#include "templatepage.h"
#include "ui_templatepage.h"
#include <interfaces/itemplateprovider.h>
#include <KFileDialog>
#include <KNS3/DownloadDialog>
#include <knewstuff3/uploaddialog.h>

TemplatePage::TemplatePage (KDevelop::ITemplateProvider* provider, QWidget* parent) : QWidget (parent),
m_provider(provider)
{
    ui = new Ui::TemplatePage;
    ui->setupUi(this);

    ui->getNewButton->setVisible(!m_provider->knsConfigurationFile().isEmpty());
    connect(ui->getNewButton, SIGNAL(clicked(bool)),
            this, SLOT(getMoreTemplates()));

    ui->shareButton->setVisible(!m_provider->knsConfigurationFile().isEmpty());
    connect(ui->shareButton, SIGNAL(clicked(bool)),
            this, SLOT(shareTemplates()));

    ui->loadButton->setVisible(!m_provider->supportedMimeTypes().isEmpty());
    connect(ui->loadButton, SIGNAL(clicked(bool)),
            this, SLOT(loadFromFile()));

    provider->reload();
    ui->treeView->setModel(provider->templatesModel());
}

TemplatePage::~TemplatePage()
{
    delete ui;
}

void TemplatePage::loadFromFile()
{
    QString filename = KFileDialog::getOpenFileName(KUrl("kfiledialog:///kdevtemplates"), m_provider->supportedMimeTypes().join(" "), this);

    if (!filename.isEmpty())
    {
        m_provider->loadTemplate(filename);
    }

    m_provider->reload();
}

void TemplatePage::getMoreTemplates()
{
    KNS3::DownloadDialog dialog(m_provider->knsConfigurationFile(), this);
    dialog.exec();

    if (!dialog.changedEntries().isEmpty())
    {
        m_provider->reload();
    }
}

void TemplatePage::shareTemplates()
{
    KNS3::UploadDialog dialog(m_provider->knsConfigurationFile(), this);
    dialog.exec();
}

