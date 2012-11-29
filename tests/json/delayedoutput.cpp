/* This file is part of KDevelop
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QDebug>

#include "delayedoutput.h"

namespace KDevelop
{
typedef QPair<QString, int> DepthedOutput;

class DelayedOutputPrivate
{
public:
  void flushOutput()
  {
    while (output.size())
    {
        DepthedOutput curOutput = output.pop();
        qDebug().nospace() << qPrintable(QString(curOutput.second -1, ' ')) << curOutput.first.toUtf8().data();
    }
  }
  QStack<DepthedOutput> output;
  int delayDepth;
};
DelayedOutput::Delay::Delay(DelayedOutput* output)
{
  m_output = output;
  ++m_output->d->delayDepth;
}
DelayedOutput::Delay::~Delay()
{
  --m_output->d->delayDepth;
  if (!m_output->d->delayDepth)
    m_output->d->flushOutput();
}

DelayedOutput::DelayedOutput()
: d(new DelayedOutputPrivate())
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
  d->output.push(DepthedOutput(output, d->delayDepth));
}

}
