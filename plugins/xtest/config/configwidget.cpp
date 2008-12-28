/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "configwidget.h"
#include "ui_veritasconfig.h"

#include <KIcon>
#include <QToolButton>
#include <KUrlRequester>
#include <QLabel>

using Veritas::ConfigWidget;

ConfigWidget::ConfigWidget(QWidget* parent)
    : QWidget(parent), m_details(0)
{
    m_ui = new Ui::VeritasConfig;
    m_ui->setupUi(this);
    setupButtons();
    connect(frameworkBox(), SIGNAL(activated(QString)),
            SIGNAL(frameworkSelected(QString)));
    expandDetailsButton()->setEnabled(false);
    m_allowUserModification = true;
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::setReadOnly()
{
    m_allowUserModification = false;
    addExecutableButton()->setEnabled(false);
    QList<KUrlRequester*> requesters = findChildren<KUrlRequester*>();
    foreach(KUrlRequester* req, requesters) {
        req->setEnabled(false);
    }
    foreach(QToolButton* rem, m_removeButtons) {
        rem->setEnabled( false );
    }
}

void ConfigWidget::setDetailsWidget(QWidget* detailsWidget)
{
    if (m_details) {
        expandDetailsButton()->setChecked(false);
    }
    m_details = detailsWidget;
    expandDetailsButton()->setEnabled(detailsWidget!=0);
    if (m_details) {
        m_ui->frameworkLayout->addWidget(m_details);
        m_details->hide();
    }
}

void ConfigWidget::expandDetails(bool checked)
{
    Q_ASSERT(m_details);
    KIcon icon(checked ? "arrow-up-double" : "arrow-down-double");
    expandDetailsButton()->setIcon(icon);
    checked ? m_details->show() : m_details->hide();
    repaint();
}

void ConfigWidget::addTestExecutableField(const KUrl& testExecutable)
{
    QHBoxLayout* lay = new QHBoxLayout();
    KUrlRequester* req = new KUrlRequester(this);
    connect(req, SIGNAL(textChanged(QString)), SIGNAL(changed()));
    req->setUrl(testExecutable);
    req->setEnabled(m_allowUserModification);
    QToolButton* remove = new QToolButton(this);
    remove->setEnabled(m_allowUserModification);
    remove->setToolButtonStyle( Qt::ToolButtonIconOnly );
    remove->setIcon(KIcon("list-remove"));
    lay->addWidget(req);
    lay->addWidget(remove);
    m_removeButtons << remove;

    QVBoxLayout* exeFields = qobject_cast<QVBoxLayout*>(executableFieldsLayout()); // contains all the KUrlRequesters (in their own QHBoxLayout)
    Q_ASSERT(exeFields);
    int newIndex = exeFields->count()-1; // the last item is a spacer, insert in front of that.
    exeFields->insertLayout(newIndex, lay);
    connect(remove, SIGNAL(clicked(bool)), SLOT(removeTestExecutableField()));

    exeFields->invalidate();
    exeFields->update();
    repaint();
}

QComboBox* ConfigWidget::frameworkBox() const
{
    return m_ui->frameworkSelection;
}

QString ConfigWidget::currentFramework() const
{
    return frameworkBox()->currentText();
}

void ConfigWidget::setCurrentFramework(const QString& name)
{
    int index = frameworkBox()->findText(name);
    if (index != -1) {
        frameworkBox()->setCurrentIndex(index);
    }
}


void ConfigWidget::removeTestExecutableField()
{
    QToolButton* removeButton = qobject_cast<QToolButton*>(sender());
    Q_ASSERT(removeButton);
    // retrieve the executable-bar index this button belongs to
    int fieldIndex = m_removeButtons.indexOf(removeButton);
    Q_ASSERT(fieldIndex != -1);
    m_removeButtons.removeAll(removeButton);
    QLayoutItem* i = executableFieldsLayout()->takeAt(fieldIndex);
    Q_ASSERT(i);
    KUrlRequester* req = qobject_cast<KUrlRequester*>(i->layout()->itemAt(0)->widget());
    Q_ASSERT(req);
    if (!req->url().isEmpty()) emit changed();
    delete i->layout()->itemAt(0)->widget();
    delete i->layout()->itemAt(1)->widget();
    delete i->layout();
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
    connect(addExecutableButton(), SIGNAL(clicked(bool)), SLOT(addTestExecutableField()));
}

void ConfigWidget::fto_clickExpandDetails() const
{
    expandDetailsButton()->toggle();
}

void ConfigWidget::fto_clickAddTestExeField() const
{
    addExecutableButton()->click();
}

void ConfigWidget::fto_clickRemoveTestExeField(int fieldIndex) const
{
    removeExecutableButton(fieldIndex)->click();
}

QStringList ConfigWidget::fto_frameworkComboBoxContents() const
{
    int nrof = frameworkBox()->count();
    QStringList frameworks;
    for (int i=0; i<nrof; ++i) {
        frameworks << frameworkBox()->itemText(i);
    }
    return frameworks;
}

void ConfigWidget::appendFramework(const QString& framework)
{
    frameworkBox()->addItem(framework);
}

QToolButton* ConfigWidget::expandDetailsButton() const
{
    return m_ui->expandDetails;
}

QToolButton* ConfigWidget::addExecutableButton() const
{
    return m_ui->addExecutable;
}

QLayout* ConfigWidget::executableFieldsLayout() const
{
    return m_ui->scrollExeLayout;
}

QToolButton* ConfigWidget::removeExecutableButton(int fieldIndex) const
{
    Q_ASSERT(fieldIndex >= 0);
    Q_ASSERT(fieldIndex < findChildren<KUrlRequester*>().count());
    QLayoutItem* removeItem = executableFieldsLayout()->itemAt(fieldIndex)->layout()->itemAt(1);
    Q_ASSERT(removeItem);
    QToolButton* remove = qobject_cast<QToolButton*>(removeItem->widget());
    Q_ASSERT(remove);
    return remove;
}

KUrl::List ConfigWidget::executables() const
{
    KUrl::List exes;
    QList<KUrlRequester*> urlRequesters = findChildren<KUrlRequester*>();
    foreach(KUrlRequester* ureq, urlRequesters) {
        if (!ureq->url().isEmpty()) {
            exes << ureq->url();
        }
    }
    return exes;
}

int ConfigWidget::numberOfTestExecutableFields() const
{
    return findChildren<KUrlRequester*>().count();
}

#include "configwidget.moc"
