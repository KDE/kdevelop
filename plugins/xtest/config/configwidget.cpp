#include "configwidget.h"
#include "ui_veritasconfig.h"

#include <KIcon>
#include <QToolButton>
#include <QLabel>

using Veritas::ConfigWidget;

ConfigWidget::ConfigWidget(QWidget* parent)
    : QWidget(parent)
{
    m_ui = new Ui::VeritasConfig;
    m_ui->setupUi(this);
    setupButtons();
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::expandDetails(bool checked)
{
    KIcon icon(checked ? "arrow-up-double" : "arrow-down-double");
    expandDetailsButton()->setIcon(icon);
    QLabel* l = new QLabel;
    l->setText("ALLO WORLD");
    m_ui->frameworkLayout->addWidget(l);
    l->show();
    repaint();
}

void ConfigWidget::setupButtons()
{
    expandDetailsButton()->setToolButtonStyle( Qt::ToolButtonIconOnly );
    expandDetailsButton()->setIcon(KIcon("arrow-down-double"));
    expandDetailsButton()->setCheckable(true);
    expandDetailsButton()->setChecked(false);
    connect(expandDetailsButton(), SIGNAL(toggled(bool)), SLOT(expandDetails(bool)));

    addExecutableButton()->setToolButtonStyle( Qt::ToolButtonIconOnly );
    addExecutableButton()->setIcon(KIcon("list-add"));

    removeExecutableButton()->setToolButtonStyle( Qt::ToolButtonIconOnly );
    removeExecutableButton()->setIcon(KIcon("list-remove"));
}

void ConfigWidget::fto_clickExpandDetails() const
{
    expandDetailsButton()->toggle();
}

QToolButton* ConfigWidget::expandDetailsButton() const
{
    return m_ui->expandDetails;
}

QToolButton* ConfigWidget::addExecutableButton() const
{
    return m_ui->addExecutable;
}

QToolButton* ConfigWidget::removeExecutableButton() const
{
    return m_ui->removeExecutable;
}

#include "configwidget.moc"
