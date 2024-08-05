/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_LANGUAGECONTROLLERTESTBASE_H
#define KDEVPLATFORM_LANGUAGECONTROLLERTESTBASE_H

#include <QObject>
#include <QString>
#include <QUrl>

#include <vector>

namespace KDevelop {
class ILanguageController;
}

class LanguageControllerTestBase : public QObject
{
    Q_OBJECT

protected:
    explicit LanguageControllerTestBase(QObject* parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

protected:
    void fillLanguageControllerMimeTypeCache() const;
    void matchingLanguagesForUrlInBackgroundThreadTestData() const;
    void matchingLanguagesForUrlTestData() const;
    static void nonmatchingLanguagesForUrlTestData();

    KDevelop::ILanguageController* m_subject = nullptr;

private:
    void newOptionalRow(const char* dataTag, const QUrl& url, const QString& languageName) const;

    struct UrlEntry
    {
        QUrl url;
        QString languageName;
    };
    std::vector<UrlEntry> m_differentLanguagesUrls;
};

#endif // KDEVPLATFORM_LANGUAGECONTROLLERTESTBASE_H
