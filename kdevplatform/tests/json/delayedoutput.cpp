/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "delayedoutput.h"

#include <QDebug>
#include <QStack>

namespace KDevelop {
using DepthedOutput = QPair<QString, int>;

class DelayedOutputPrivate
{
public:
    void flushOutput()
    {
        while (!output.isEmpty()) {
            DepthedOutput curOutput = output.pop();
            qDebug().nospace() <<
                qPrintable(QString(curOutput.second - 1, QLatin1Char(' '))) << curOutput.first.toUtf8().data();
        }
    }
    QStack<DepthedOutput> output;
    int delayDepth;
};
DelayedOutput::Delay::Delay(DelayedOutput* output)
{
    m_output = output;
    ++m_output->d_func()->delayDepth;
}
DelayedOutput::Delay::~Delay()
{
    --m_output->d_func()->delayDepth;
    if (!m_output->d_func()->delayDepth)
        m_output->d_func()->flushOutput();
}

DelayedOutput::DelayedOutput()
    : d_ptr(new DelayedOutputPrivate())
{
}
DelayedOutput::~DelayedOutput()
{
}
DelayedOutput& DelayedOutput::self()
{
    static DelayedOutput _inst;
    return _inst;
}
void DelayedOutput::push(const QString& output)
{
    Q_D(DelayedOutput);

    d->output.push(DepthedOutput(output, d->delayDepth));
}
}
