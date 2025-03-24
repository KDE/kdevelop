/*
    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_OUTPUTJOB_H
#define KDEVPLATFORM_OUTPUTJOB_H

#include <outputview/ioutputview.h>
#include <outputview/outputviewexport.h>

#include <KJob>

class QIcon;

namespace KDevelop
{
class OutputJobPrivate;

class KDEVPLATFORMOUTPUTVIEW_EXPORT OutputJob : public KJob
{
    Q_OBJECT

public:
    enum
    {
        FailedShownError = UserDefinedError + 100 //job failed and failure is shown in OutputView
    };
    enum OutputJobVerbosity { Silent, Verbose };

    explicit OutputJob(QObject* parent = nullptr, OutputJobVerbosity verbosity = OutputJob::Verbose);
    ~OutputJob() override;

    void startOutput();

    OutputJobVerbosity verbosity() const;

    void setVerbosity(OutputJobVerbosity verbosity);

    QAbstractItemModel* model() const;

    /**
     * @return the title for this job's output tab
     */
    [[nodiscard]] QString title() const;
    /// Set the \a title for this job's output tab.  If not set, will default to the job's objectName().
    void setTitle(const QString& title);

protected:
    /// NOTE: if a standard tool view is not set, a new unshared and unconfigurable tool view is created for the job.
    void setStandardToolView(IOutputView::StandardToolView standard);
    void setToolTitle(const QString& title);
    void setToolIcon(const QIcon& icon);
    void setViewType(IOutputView::ViewType type);
    void setOptions(IOutputView::Options options);
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
    const QScopedPointer<class OutputJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(OutputJob)
};

}

#endif
