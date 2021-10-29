/*
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_BREAKPOINTDETAILS_H
#define KDEVPLATFORM_BREAKPOINTDETAILS_H

#include <QWidget>
#include <debugger/debuggerexport.h>

namespace KDevelop {
class Breakpoint;
class BreakpointDetailsPrivate;

class KDEVPLATFORMDEBUGGER_EXPORT BreakpointDetails : public QWidget
{
    Q_OBJECT
public:
    explicit BreakpointDetails(QWidget *parent = nullptr);
    ~BreakpointDetails() override;

    void setItem(Breakpoint *breakpoint);

private Q_SLOTS:
    void showExplanation(const QString& link);

    void setIgnoreHits(int ignoreHits);

private:
    const QScopedPointer<class BreakpointDetailsPrivate> d_ptr;
    Q_DECLARE_PRIVATE(BreakpointDetails)
};


}

#endif // KDEVPLATFORM_BREAKPOINTDETAILS_H
