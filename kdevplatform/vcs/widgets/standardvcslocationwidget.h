/*
    SPDX-FileCopyrightText: 2010 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_STANDARDVCSLOCATIONWIDGET_H
#define KDEVPLATFORM_STANDARDVCSLOCATIONWIDGET_H

#include <vcs/widgets/vcslocationwidget.h>
#include <vcs/vcsexport.h>

class QUrl;
class KUrlRequester;
namespace KDevelop
{

class KDEVPLATFORMVCS_EXPORT StandardVcsLocationWidget : public VcsLocationWidget
{
    Q_OBJECT
    public:
        explicit StandardVcsLocationWidget(QWidget* parent = nullptr, Qt::WindowFlags f = {});
        VcsLocation location() const override;
        bool isCorrect() const override;
        QUrl url() const;
        QString projectName() const override;
        void setLocation(const QUrl& remoteLocation) override;
        void setUrl(const QUrl& url);

    public Q_SLOTS:
        void textChanged(const QString& str);
        
    private:
        KUrlRequester* m_urlWidget;
};

}
#endif // KDEVPLATFORM_STANDARDVCSLOCATIONWIDGET_H
