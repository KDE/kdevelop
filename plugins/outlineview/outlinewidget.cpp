/*
 *   KDevelop outline view
 *   Copyright 2010, 2015 Alex Richardson <alex.richardson@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "outlinewidget.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QLineEdit>
#include <QIcon>

#include <KLocalizedString>

#include <interfaces/idocumentcontroller.h>

#include "outlineviewplugin.h"
#include "outlinemodel.h"
#include "outlineproxymodel.h"

using namespace KDevelop;

OutlineWidget::OutlineWidget(QWidget* parent, OutlineViewPlugin* plugin)
    : QWidget(parent), m_plugin(plugin), m_model(new OutlineModel(this)),
      m_tree(new QTreeView(this)), m_proxy(new OutlineProxyModel(this)),
      m_filter(new QLineEdit(this))
{
    setObjectName("Outline View");
    setWindowTitle(i18n("Outline"));
    setWhatsThis(i18n("Outline View"));
    setWindowIcon(QIcon::fromTheme("code-class")); //TODO: better icon?

    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_tree->setModel(m_proxy);
    m_tree->setHeaderHidden(true);

    //filter
    connect(m_filter, SIGNAL(textChanged(QString)), m_proxy, SLOT(setFilterFixedString(QString)));
    connect(m_tree, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));

    QHBoxLayout* filterLayout = new QHBoxLayout();

    m_filter->setPlaceholderText(i18n("Filter..."));
    filterLayout->addWidget(m_filter);
    setFocusProxy(m_filter);

    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    vbox->addLayout(filterLayout);
    vbox->addWidget(m_tree);
    setLayout(vbox);
}

void OutlineWidget::activated(QModelIndex index)
{
    QModelIndex realIndex = m_proxy->mapToSource(index);
    m_model->activate(realIndex);
}

OutlineWidget::~OutlineWidget()
{
}
