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
#include <QTreeView>
#include <QApplication>
#include <QEvent>
#include <QStandardItemModel>
#include <QMouseEvent>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QSignalMapper>
#include <QDesktopWidget>
#include <KTextEditor/View>
#include <KLocale>
#include <QPainter>

#include "variablecollection.h"
#include "../breakpoint/breakpointmodel.h"
#include "../interfaces/ivariablecontroller.h"
#include "../../util/activetooltip.h"
#include "../../interfaces/icore.h"
#include "../../interfaces/idebugcontroller.h"

namespace KDevelop {

class SizeGrip : public QWidget
{
public:
    SizeGrip(QWidget* parent) : QWidget(parent) {
        m_parent = parent;
    }
protected:
    virtual void paintEvent(QPaintEvent *)
    {
      QPainter painter(this);
      QStyleOptionSizeGrip opt;
      opt.init(this);
      opt.corner = Qt::BottomRightCorner;
      style()->drawControl(QStyle::CE_SizeGrip, &opt, &painter, this);
    }

    virtual void mousePressEvent(QMouseEvent* e)
    {
        if (e->button() == Qt::LeftButton) {
            m_pos = e->globalPos();
            m_startSize = m_parent->size();
            e->ignore();
        }
    }
    virtual void mouseReleaseEvent(QMouseEvent*)
    {
        m_pos = QPoint();
    }
    virtual void mouseMoveEvent(QMouseEvent* e)
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

VariableToolTip::VariableToolTip(QWidget* parent, QPoint position, 
                                 const QString& identifier)
:  ActiveToolTip(parent, position)
{
    setPalette( QApplication::palette() );

    model_ = new TreeModel(QVector<QString>() << i18n("Name") << i18n("Value") << i18n("Type"),
                           this);

    TooltipRoot* tr = new TooltipRoot(model_);
    model_->setRootItem(tr);
    var_ = ICore::self()->debugController()->currentSession()->
        variableController()->createVariable(
            model_, tr, identifier);
    tr->init(var_);
    var_->attachMaybe(this, "variableCreated");

    QVBoxLayout* l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    view_ = new AsyncTreeView(model_, this);
    view_->header()->resizeSection(0, 150);
    view_->header()->resizeSection(1, 90);
    view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    view_->setSelectionMode(QAbstractItemView::SingleSelection);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    l->addWidget(view_);

    itemHeight_ = view_->indexRowSizeHint(model_->indexForItem(var_, 0));
    connect(view_->verticalScrollBar(),
            SIGNAL(rangeChanged(int,int)),
            this,
            SLOT(slotRangeChanged(int,int)));

    selection_ = view_->selectionModel();
    selection_->select(model_->indexForItem(var_, 0), 
                       QItemSelectionModel::Rows
                       | QItemSelectionModel::ClearAndSelect);

    QHBoxLayout* buttonBox = new QHBoxLayout();
    buttonBox->setContentsMargins(11, 0, 11, 6);
    QPushButton* watchThisButton = new QPushButton(i18n("Watch this"));
    buttonBox->addWidget(watchThisButton);
    QPushButton* stopOnChangeButton = new QPushButton(i18n("Stop on Change"));
    buttonBox->addWidget(stopOnChangeButton);

    QSignalMapper* mapper = new QSignalMapper(this);
    connect(watchThisButton, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(watchThisButton, "add_watch");
    connect(stopOnChangeButton, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(stopOnChangeButton, "add_watchpoint");
    connect(mapper, SIGNAL(mapped(QString)), this, SLOT(slotLinkActivated(QString)));

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
    view_->resizeColumns();
    if (hasValue) {
        ActiveToolTip::showToolTip(this, 0.0);
    } else {
        close();
    }
}

void VariableToolTip::slotLinkActivated(const QString& link)
{
    Variable* v = var_;
    QItemSelection s = selection_->selection();
    if (!s.empty())
    {
        QModelIndex index = s.front().topLeft();
        TreeItem *item = model_->itemForIndex(index);
        if (item)
        {
            Variable* v2 = dynamic_cast<Variable*>(item);
            if (v2)
                v = v2;
        }
    }

    IDebugSession *session = ICore::self()->debugController()->currentSession();
    if (session && session->state() != IDebugSession::NotStartedState && session->state() != IDebugSession::EndedState) {
        if (link == "add_watch") {
            session->variableController()->addWatch(v);
        } else if (link == "add_watchpoint") {
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
    if (pos().y() + height() + max*itemHeight_ < rect.bottom())
        resize(width(), height() + max*itemHeight_);    
    else
    {
        // Oh, well, I'm sorry, but here's the scrollbar you was
        // longing to see
        view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);        
    }
}

}

#include "variabletooltip.moc"
