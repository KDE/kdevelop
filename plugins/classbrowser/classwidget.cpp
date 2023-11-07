/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "classwidget.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>

#include <KLocalizedString>
#include <QLineEdit>
#include <QTimer>

#include "language/classmodel/classmodel.h"
#include "classtree.h"
#include "classbrowserplugin.h"

using namespace KDevelop;

ClassWidget::ClassWidget(QWidget* parent, ClassBrowserPlugin* plugin)
    : QWidget(parent)
    , m_plugin(plugin)
    , m_model(new ClassModel())
    , m_tree(new ClassTree(this, plugin))
    , m_searchLine(new QLineEdit(this))
    , m_filterTimer(new QTimer(this))
{
    setObjectName(QStringLiteral("Class Browser Tree"));
    setWindowTitle(i18nc("@title:window", "Classes"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("code-class"), windowIcon()));

    // Set tree in the plugin
    m_plugin->setActiveClassTree(m_tree);

    // Set model in the tree view
    m_tree->setModel(m_model);
    m_tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tree->header()->setStretchLastSection(false);
    m_tree->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::TopEdge}));

    // We need notification in the model for the collapse/expansion of nodes.
    connect(m_tree, &ClassTree::collapsed,
            m_model, &ClassModel::collapsed);
    connect(m_tree, &ClassTree::expanded,
            m_model, &ClassModel::expanded);

    // Init filter timer
    m_filterTimer->setSingleShot(true);
    m_filterTimer->setInterval(500);
    connect(m_filterTimer, &QTimer::timeout, this, [this]() {
        m_tree->setCurrentIndex(QModelIndex());
        m_model->updateFilterString(m_filterText);

        if (m_filterText.isEmpty())
            m_tree->collapseAll();
        else
            m_tree->expandToDepth(0);
    });

    // Init search box
    m_searchLine->setClearButtonEnabled(true);
    connect(m_searchLine, &QLineEdit::textChanged, this, [this](const QString& newFilter) {
        m_filterText = newFilter;
        m_filterTimer->start();
    });

    auto* searchLabel = new QLabel(i18nc("@label:textbox", "S&earch:"), this);
    searchLabel->setBuddy(m_searchLine);

    auto* layout = new QHBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(
        style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
        style()->pixelMetric(QStyle::PM_LayoutTopMargin),
        style()->pixelMetric(QStyle::PM_LayoutRightMargin),
        style()->pixelMetric(QStyle::PM_LayoutBottomMargin)
    );
    layout->addWidget(searchLabel);
    layout->addWidget(m_searchLine);

    setFocusProxy(m_searchLine);

    auto* vbox = new QVBoxLayout(this);
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addLayout(layout);
    vbox->addWidget(m_tree);
    setLayout(vbox);

    setWhatsThis(i18nc("@info:whatsthis", "Class Browser"));
}

ClassWidget::~ClassWidget()
{
    delete m_model;
}

#include "moc_classwidget.cpp"
