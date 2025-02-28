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
    /**
     * Place the job's output into a given standard output tool view.
     *
     * If this function is called, calling setToolViewId(), setToolTitle(), setToolIcon(), setViewType(),
     * and setOptions() is pointless because the standard tool view defines these values.
     */
    void setStandardToolView(IOutputView::StandardToolView standard);
    /**
     * Set the tool view ID of a nonstandard tool view to a given value.
     *
     * This function must be called if and only if setStandardToolView() is not called.
     *
     * @param toolViewId a nonempty ID that identifies and allows to share a nonstandard tool view
     */
    void setToolViewId(const QString& toolViewId);
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
    void outputViewRemoved(int id);

private:
    const QScopedPointer<class OutputJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(OutputJob)
};

}

#endif
