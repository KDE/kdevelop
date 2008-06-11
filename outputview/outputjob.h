/* This file is part of KDevelop
Copyright 2007-2008 Hamish Rodda <rodda@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef OUTPUTJOB_H
#define OUTPUTJOB_H

#include <QPointer>

#include "interfaces/irun.h"
#include "interfaces/irunprovider.h"
#include "interfaces/iruncontroller.h"

#include "outputview/ioutputview.h"
#include "outputview/outputviewexport.h"

class QStandardItemModel;
class QItemDelegate;

namespace KDevelop
{

class KDEVPLATFORMOUTPUTVIEW_EXPORT OutputJob : public KJob
{
    Q_OBJECT

public:
    OutputJob(QObject* parent = 0);

    void startOutput();

    void setRaiseOnCreation(bool raise);

protected:
    void setStandardToolView(IOutputView::StandardToolView standard);
    void setToolTitle(const QString& title);
    void setToolIcon(const KIcon& icon);
    /// Set the \a title for this job's output tab.  If not set, will default to the job's objectName().
    void setTitle(const QString& title);
    void setViewType(IOutputView::ViewType type);
    void setBehaviours(IOutputView::Behaviours behaviours);
    void setKillJobOnOutputClose(bool killJobOnOutputClose);

    QStandardItemModel* model() const;
    void setModel(QStandardItemModel* model, IOutputView::Ownership takeOwnership = IOutputView::KeepOwnership);
    void setDelegate(QAbstractItemDelegate* delegate, IOutputView::Ownership takeOwnership = IOutputView::KeepOwnership);

private Q_SLOTS:
    void outputViewRemoved(int , int id);

private:
    int m_standardToolView;
    QString m_title, m_toolTitle;
    KIcon m_toolIcon;
    IOutputView::ViewType m_type;
    IOutputView::Behaviours m_behaviours;
    bool m_killJobOnOutputClose;
    bool m_raiseOnCreation;
    int m_outputId;
    QStandardItemModel* m_outputModel;
    IOutputView::Ownership m_modelOwnership;
    QAbstractItemDelegate* m_outputDelegate;
    IOutputView::Ownership m_delegateOwnership;
};

}

#endif
