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

#ifndef KDEVPLATFORM_OUTPUTJOB_H
#define KDEVPLATFORM_OUTPUTJOB_H

#include <QtCore/QPointer>

#include <kjob.h>

#include <outputview/ioutputview.h>
#include <outputview/outputviewexport.h>

class QStandardItemModel;
class QItemDelegate;

namespace KDevelop
{

class KDEVPLATFORMOUTPUTVIEW_EXPORT OutputJob : public KJob
{
    Q_OBJECT

public:
    enum
    {
        FailedShownError = UserDefinedError + 100 //job failed and failure is shown in OutputView
    };
    enum OutputJobVerbosity { Silent, Verbose };

    OutputJob(QObject* parent = 0, OutputJobVerbosity verbosity = OutputJob::Verbose);

    void startOutput();

    OutputJobVerbosity verbosity() const;

    void setVerbosity(OutputJobVerbosity verbosity);

    QAbstractItemModel* model() const;

    /// Set the \a title for this job's output tab.  If not set, will default to the job's objectName().
    void setTitle(const QString& title);

protected:
    void setStandardToolView(IOutputView::StandardToolView standard);
    void setToolTitle(const QString& title);
    void setToolIcon(const KIcon& icon);
    void setViewType(IOutputView::ViewType type);
    void setBehaviours(IOutputView::Behaviours behaviours);
    void setKillJobOnOutputClose(bool killJobOnOutputClose);

    /**
     * Sets the model for the view that shows this jobs output.
     *
     * The view takes ownership of the model, but it is safe to
     * use the model while the job is running.
     *
     * NOTE: Do not reuse the same model for different jobs.
     */
    void setModel(QAbstractItemModel* model);

    /**
     * Sets the delegate for the view that shows this jobs output.
     *
     * The view takes ownership of the delegate, but it is safe to
     * use the delegate while the job is running.
     *
     * NOTE: Do not reuse the same delegate for different jobs.
     */
    void setDelegate(QAbstractItemDelegate* delegate);

    int outputId() const;

private Q_SLOTS:
    void outputViewRemoved(int , int id);

private:
    int m_standardToolView;
    QString m_title, m_toolTitle;
    KIcon m_toolIcon;
    IOutputView::ViewType m_type;
    IOutputView::Behaviours m_behaviours;
    bool m_killJobOnOutputClose;
    OutputJobVerbosity m_verbosity;
    int m_outputId;
    QPointer<QAbstractItemModel> m_outputModel;
    QAbstractItemDelegate* m_outputDelegate;
};

}

#endif
