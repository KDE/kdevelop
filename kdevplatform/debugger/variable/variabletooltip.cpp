/*
 * KDevelop Debugger Support
 *
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "variabletooltip.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QApplication>
#include <QMouseEvent>
#include <QHeaderView>
#include <QPushButton>
#include <QScrollBar>
#include <QSignalMapper>
#include <QDesktopWidget>
#include <KTextEditor/View>
#include <KLocalizedString>
#include <QPainter>
#include <QSortFilterProxyModel>

#include "variablecollection.h"
#include "../util/treeview.h"
#include "../interfaces/ivariablecontroller.h"
#include "../../util/activetooltip.h"
#include "../../interfaces/icore.h"
#include "../../interfaces/idebugcontroller.h"

namespace KDevelop {

class SizeGrip : public QWidget
{
    Q_OBJECT
public:
    explicit SizeGrip(QWidget* parent) : QWidget(parent) {
        m_parent = parent;
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
      QPainter painter(this);
      QStyleOptionSizeGrip opt;
      opt.init(this);
      opt.corner = Qt::BottomRightCorner;
      style()->drawControl(QStyle::CE_SizeGrip, &opt, &painter, this);
    }

    void mousePressEvent(QMouseEvent* e) override
    {
        if (e->button() == Qt::LeftButton) {
            m_pos = e->globalPos();
            m_startSize = m_parent->size();
            e->ignore();
        }
    }
    void mouseReleaseEvent(QMouseEvent*) override
    {
        m_pos = QPoint();
    }
    void mouseMoveEvent(QMouseEvent* e) override
    {
        if (!m_pos.isNull()) {
            m_parent->resize(
                m_startSize.width() + (e->globalPos().x() - m_pos.x()),
                m_startSize.height() + (e->globalPos().y() - m_pos.y())
            );
        }
    }
private:
    QWidget *m_parent;
    QSize m_startSize;
    QPoint m_pos;
};

VariableToolTip::VariableToolTip(QWidget* parent, const QPoint& position,
                                 const QString& identifier)
:  ActiveToolTip(parent, position)
{
    setPalette( QApplication::palette() );

    m_model = new TreeModel(QVector<QString>() << i18n("Name") << i18n("Value") << i18n("Type"),
                           this);

    TooltipRoot* tr = new TooltipRoot(m_model);
    m_model->setRootItem(tr);
    m_var = ICore::self()->debugController()->currentSession()->
        variableController()->createVariable(
               m_model, tr, identifier);
    tr->init(m_var);
    m_var->attachMaybe(this, "variableCreated");

    QVBoxLayout* l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    // setup proxy model
    m_proxy = new QSortFilterProxyModel;
    m_view = new AsyncTreeView(m_model, m_proxy, this);
    m_proxy->setSourceModel(m_model);
    m_view->setModel(m_proxy);
    m_view->header()->resizeSection(0, 150);
    m_view->header()->resizeSection(1, 90);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    l->addWidget(m_view);

    QModelIndex varIndex = m_proxy->mapFromSource(m_model->indexForItem(m_var, 0));
    m_itemHeight = m_view->indexRowSizeHint(varIndex);
    connect(m_view->verticalScrollBar(),
            &QScrollBar::rangeChanged,
            this,
            &VariableToolTip::slotRangeChanged);

    m_selection = m_view->selectionModel();
    m_selection->select(varIndex,
                        QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);

    QHBoxLayout* buttonBox = new QHBoxLayout();
    buttonBox->setContentsMargins(11, 0, 11, 6);
    QPushButton* watchThisButton = new QPushButton(i18n("Watch This"));
    buttonBox->addWidget(watchThisButton);
    QPushButton* stopOnChangeButton = new QPushButton(i18n("Stop on Change"));
    buttonBox->addWidget(stopOnChangeButton);

    QSignalMapper* mapper = new QSignalMapper(this);
    connect(watchThisButton, &QPushButton::clicked, mapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
    mapper->setMapping(watchThisButton, QStringLiteral("add_watch"));
    connect(stopOnChangeButton, &QPushButton::clicked, mapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
    mapper->setMapping(stopOnChangeButton, QStringLiteral("add_watchpoint"));
    connect(mapper, static_cast<void(QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped), this, &VariableToolTip::slotLinkActivated);

    QHBoxLayout* inner = new QHBoxLayout();
    l->addLayout(inner);
    inner->setContentsMargins(0, 0, 0, 0);
    inner->addLayout(buttonBox);
    inner->addStretch();

    SizeGrip* g = new SizeGrip(this);
    g->setFixedSize(16, 16);
    inner->addWidget(g, 0, (Qt::Alignment)(Qt::AlignRight | Qt::AlignBottom));

    move(position);
}

void VariableToolTip::variableCreated(bool hasValue)
{
    m_view->resizeColumns();
    if (hasValue) {
        ActiveToolTip::showToolTip(this, 0.0);
    } else {
        close();
    }
}

void VariableToolTip::slotLinkActivated(const QString& link)
{
    Variable* v = m_var;
    QItemSelection s = m_selection->selection();
    if (!s.empty())
    {
        QModelIndex index = s.front().topLeft();
        const auto sourceIndex = m_proxy->mapToSource(index);
        TreeItem *item = m_model->itemForIndex(sourceIndex);
        if (item)
        {
            Variable* v2 = qobject_cast<Variable*>(item);
            if (v2)
                v = v2;
        }
    }

    IDebugSession *session = ICore::self()->debugController()->currentSession();
    if (session && session->state() != IDebugSession::NotStartedState && session->state() != IDebugSession::EndedState) {
        if (link == QLatin1String("add_watch")) {
            session->variableController()->addWatch(v);
        } else if (link == QLatin1String("add_watchpoint")) {
            session->variableController()->addWatchpoint(v);
        }
    }
    close();
}

void VariableToolTip::slotRangeChanged(int min, int max)
{
    Q_ASSERT(min == 0);
    Q_UNUSED(min);
    QRect rect = QApplication::desktop()->screenGeometry(this);
    if (pos().y() + height() + max*m_itemHeight < rect.bottom())
        resize(width(), height() + max*m_itemHeight);
    else
    {
        // Oh, well, I'm sorry, but here's the scrollbar you was
        // longing to see
        m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }
}

}

#include "variabletooltip.moc"
