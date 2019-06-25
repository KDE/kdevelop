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

using namespace KDevelop;

class KDevelop::BreakpointDetailsPrivate
{
public:
    QLabel* status;
    QLabel* hits;
    QSpinBox* ignore;
    Breakpoint* currentBreakpoint = nullptr;
};

BreakpointDetails::BreakpointDetails(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new BreakpointDetailsPrivate)
{
    Q_D(BreakpointDetails);

    auto* layout = new QVBoxLayout(this);

    d->status = new QLabel(this);
    connect(d->status, &QLabel::linkActivated,
            this, &BreakpointDetails::showExplanation);
    layout->addWidget(d->status);

    auto* hitsLayout = new QGridLayout();
    layout->addLayout(hitsLayout);

    hitsLayout->setContentsMargins(0, 0, 0, 0);

    d->hits = new QLabel(i18n("Not hit yet"), this);
    d->hits->setWordWrap(true);
    hitsLayout->addWidget(d->hits, 0, 0, 1, 3);

    QLabel *l2 = new QLabel(i18n("Ignore"), this);
    hitsLayout->addWidget(l2, 2, 0);

    d->ignore = new QSpinBox(this);
    hitsLayout->addWidget(d->ignore, 2, 1);
    d->ignore->setRange(0, 99999);
    connect(d->ignore, QOverload<int>::of(&QSpinBox::valueChanged), this, &BreakpointDetails::setIgnoreHits);

    QLabel *l3 = new QLabel(i18n("next hits"), this);
    hitsLayout->addWidget(l3, 2, 2);

    layout->addStretch();

    setItem(nullptr); //initialize with no breakpoint active
}

BreakpointDetails::~BreakpointDetails() = default;

void KDevelop::BreakpointDetails::setIgnoreHits(int ignoreHits)
{
    Q_D(BreakpointDetails);

    if (!d->currentBreakpoint)
        return;
    d->currentBreakpoint->setIgnoreHits(ignoreHits);
}


void BreakpointDetails::setItem(Breakpoint *breakpoint)
{
    Q_D(BreakpointDetails);

    d->currentBreakpoint = breakpoint;

    if (!breakpoint) {
        d->status->hide();
        d->hits->hide();
        d->ignore->setEnabled(false);
        return;
    }

    d->ignore->setValue(breakpoint->ignoreHits());

    if (breakpoint->state() == Breakpoint::NotStartedState) {
        d->status->hide();
        d->hits->hide();
        d->ignore->setEnabled(true);
        return;
    }

    d->status->show();
    d->hits->show();
    d->ignore->setEnabled(true);

    if (breakpoint->errorText().isEmpty()) {
        switch (breakpoint->state()) {
            case Breakpoint::NotStartedState:
                Q_ASSERT(0);
                break;
            case Breakpoint::PendingState:
                d->status->setText(i18n("Breakpoint is <a href=\"pending\">pending</a>"));
                break;
            case Breakpoint::DirtyState:
                d->status->setText(i18n("Breakpoint is <a href=\"dirty\">dirty</a>"));
                break;
            case Breakpoint::CleanState:
                d->status->setText(i18n("Breakpoint is active"));
                break;
        }

        if (breakpoint->hitCount() == -1)
            d->hits->clear();
        else if (breakpoint->hitCount())
            d->hits->setText(i18np("Hit %1 time", "Hit %1 times", breakpoint->hitCount()));
        else
            d->hits->setText(i18n("Not hit yet"));
    } else {
        d->status->setText(i18n("Breakpoint has errors"));
        d->hits->setText(breakpoint->errorText());
    }
}

void BreakpointDetails::showExplanation(const QString& link)
{
    Q_D(BreakpointDetails);

    QPoint pos = d->status->mapToGlobal(d->status->geometry().topLeft());
    if (link == QLatin1String("pending"))
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
                                d->status);
    }
    else if (link == QLatin1String("dirty"))
    {
        QWhatsThis::showText(pos,
                                i18n("<b>Breakpoint is dirty</b>"
                                "<p>The breakpoint has not yet been passed "
                                "to the debugger.</p>"),
                                d->status);
    }
}

