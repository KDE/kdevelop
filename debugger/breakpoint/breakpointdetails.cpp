/*
 * This file is part of KDevelop
 *
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
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

#include "breakpointdetails.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QWhatsThis>
#include <QSpinBox>

#include <KLocalizedString>

#include "../breakpoint/breakpoint.h"
#include "../interfaces/idebugsession.h"
#include "../../interfaces/icore.h"
#include "../interfaces/ibreakpointcontroller.h"
#include "../../interfaces/idebugcontroller.h"

using namespace KDevelop;

BreakpointDetails::BreakpointDetails(QWidget *parent)
    : QWidget(parent), m_currentBreakpoint(0)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(11, 0, 0, 11);

    m_status = new QLabel(this);
    connect(m_status, &QLabel::linkActivated,
            this, &BreakpointDetails::showExplanation);
    layout->addWidget(m_status);

    QGridLayout* hitsLayout = new QGridLayout();
    layout->addLayout(hitsLayout);

    hitsLayout->setContentsMargins(0, 0, 0, 0);

    m_hits = new QLabel(i18n("Not hit yet"), this);
    m_hits->setWordWrap(true);
    hitsLayout->addWidget(m_hits, 0, 0, 1, 3);

    QFrame* frame = new QFrame(this);
    frame->setFrameShape(QFrame::HLine);
    hitsLayout->addWidget(frame, 1, 0, 1, 3);

    QLabel *l2 = new QLabel(i18n("Ignore"), this);
    hitsLayout->addWidget(l2, 2, 0);

    m_ignore = new QSpinBox(this);
    hitsLayout->addWidget(m_ignore, 2, 1);
    m_ignore->setRange(0, 99999);
    connect(m_ignore, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &BreakpointDetails::setIgnoreHits);

    QLabel *l3 = new QLabel(i18n("next hits"), this);
    hitsLayout->addWidget(l3, 2, 2);

    layout->addStretch();

    setItem(0); //initialize with no breakpoint active
}

void KDevelop::BreakpointDetails::setIgnoreHits(int ignoreHits)
{
    if (!m_currentBreakpoint)
        return;
    m_currentBreakpoint->setIgnoreHits(ignoreHits);
}


void BreakpointDetails::setItem(Breakpoint *b)
{
    m_currentBreakpoint = b;

    if (!b) {
        m_status->hide();
        m_hits->hide();
        m_ignore->setEnabled(false);
        return;
    }

    m_ignore->setValue(b->ignoreHits());

    if (b->state() == Breakpoint::NotStartedState) {
        m_status->hide();
        m_hits->hide();
        m_ignore->setEnabled(true);
        return;
    }

    m_status->show();
    m_hits->show();
    m_ignore->setEnabled(true);

    if (b->errors().isEmpty()) {
        switch (b->state()) {
            case Breakpoint::NotStartedState:
                Q_ASSERT(0);
                break;
            case Breakpoint::PendingState:
                m_status->setText(i18n("Breakpoint is <a href=\"pending\">pending</a>"));
                break;
            case Breakpoint::DirtyState:
                m_status->setText(i18n("Breakpoint is <a href=\"dirty\">dirty</a>"));
                break;
            case Breakpoint::CleanState:
                m_status->setText(i18n("Breakpoint is active"));
                break;
        }

        if (b->hitCount() == -1)
            m_hits->setText("");
        else if (b->hitCount())
            m_hits->setText(i18np("Hit %1 time", "Hit %1 times", b->hitCount()));
        else
            m_hits->setText(i18n("Not hit yet"));
    } else {
        m_status->setText(i18n("Breakpoint has errors"));
        m_hits->setText(b->errorText());
    }
}

void BreakpointDetails::showExplanation(const QString& link)
{
    QPoint pos = m_status->mapToGlobal(m_status->geometry().topLeft());
    if (link == "pending")
    {
        QWhatsThis::showText(pos,
                                i18n("<b>Breakpoint is pending</b>"
                                "<p>Pending breakpoints are those that have "
                                "been passed to GDB, but which are not yet "
                                "installed in the target, because GDB cannot "
                                "find the function or file to which the breakpoint "
                                "refers. The most common case is a breakpoint "
                                "in a shared library: GDB will insert this "
                                "breakpoint only when the library is loaded.</p>"),
                                m_status);
    }
    else if (link == "dirty")
    {
        QWhatsThis::showText(pos,
                                i18n("<b>Breakpoint is dirty</b>"
                                "<p>The breakpoint has not yet been passed "
                                "to the debugger.</p>"),
                                m_status);
    }
}

