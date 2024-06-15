/*
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "variabletooltip.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QApplication>
#include <QMouseEvent>
#include <QHeaderView>
#include <QPushButton>
#include <QScreen>
#include <QScrollBar>
#include <KLocalizedString>
#include <QPainter>

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
      opt.initFrom(this);
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
            m_parent->resize(m_startSize.width() + (e->globalPosition().x() - m_pos.x()),
                             m_startSize.height() + (e->globalPosition().y() - m_pos.y()));
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

    m_model = new TreeModel(QVector<QString>{i18n("Name"), i18n("Value"), i18n("Type")}, this);

    auto* tr = new TooltipRoot(m_model);
    m_model->setRootItem(tr);
    m_var = ICore::self()->debugController()->currentSession()->
        variableController()->createVariable(
               m_model, tr, identifier);
    tr->init(m_var);
    m_var->attachMaybe(this, "variableCreated");

    auto* l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);

    m_view = new AsyncTreeView(*m_model, this);
    m_view->setModel(m_model);
    m_view->header()->resizeSection(0, 150);
    m_view->header()->resizeSection(1, 90);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    l->addWidget(m_view);

    const QModelIndex varIndex = m_model->indexForItem(m_var, 0);
    m_itemHeight = m_view->indexRowSizeHint(varIndex);
    connect(m_view->verticalScrollBar(),
            &QScrollBar::rangeChanged,
            this,
            &VariableToolTip::slotRangeChanged);

    m_selection = m_view->selectionModel();
    m_selection->select(varIndex,
                        QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);

    auto* buttonBox = new QHBoxLayout();
    buttonBox->setContentsMargins(11, 0, 11, 6);
    auto* watchThisButton = new QPushButton(i18n("Watch This"));
    buttonBox->addWidget(watchThisButton);
    auto* stopOnChangeButton = new QPushButton(i18n("Stop on Change"));
    buttonBox->addWidget(stopOnChangeButton);

    connect(watchThisButton, &QPushButton::clicked,
            this, [this](){ slotLinkActivated(QStringLiteral("add_watch")); });
    connect(stopOnChangeButton, &QPushButton::clicked,
            this, [this](){ slotLinkActivated(QStringLiteral("add_watchpoint")); });

    auto* inner = new QHBoxLayout();
    l->addLayout(inner);
    inner->setContentsMargins(0, 0, 0, 0);
    inner->addLayout(buttonBox);
    inner->addStretch();

    auto* g = new SizeGrip(this);
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
        TreeItem* const item = m_model->itemForIndex(s.front().topLeft());
        if (item)
        {
            auto* v2 = qobject_cast<Variable*>(item);
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
    const auto rect = screen()->geometry();
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
#include "moc_variabletooltip.cpp"
