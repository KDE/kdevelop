/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROJECTSOURCEPAGE_H
#define KDEVPLATFORM_PROJECTSOURCEPAGE_H

#include <QWidget>

class KJob;
class QUrl;
namespace Ui { class ProjectSourcePage; }
namespace KDevelop
{
    class VcsJob;
    class IProjectProvider;
    class IProjectProviderWidget;
    class IPlugin;
    class IBasicVersionControl;
    class VcsLocationWidget;

class ProjectSourcePage : public QWidget
{
    Q_OBJECT
    public:
        explicit ProjectSourcePage(const QUrl& initial, const QUrl& repoUrl, IPlugin* preSelectPlugin,
                                   QWidget* parent = nullptr);
        ~ProjectSourcePage() override;

        QUrl workingDir() const;
        
    private Q_SLOTS:
        void setSourceIndex(int index);
        void checkoutVcsProject();
        void projectReceived(KJob* job);
        void reevaluateCorrection();
        void progressChanged(KJob*, unsigned long);
        void infoMessage(KJob*, const QString& text);
        void locationChanged();
        void projectChanged(const QString& name);
        
    Q_SIGNALS:
        void isCorrect(bool);
        
    private:
        void setSourceWidget(int index, const QUrl& repoUrl);
        void setStatus(const QString& message);
        void clearStatus();
        
        KDevelop::IBasicVersionControl* vcsPerIndex(int index);
        KDevelop::IProjectProvider* providerPerIndex(int index);
        KDevelop::VcsJob* jobPerCurrent();
        
        Ui::ProjectSourcePage* m_ui;
        QList<KDevelop::IPlugin*> m_plugins;
        KDevelop::VcsLocationWidget* m_locationWidget;
        KDevelop::IProjectProviderWidget* m_providerWidget;
};

}
#endif // KDEVPLATFORM_PROJECTSOURCEPAGE_H
