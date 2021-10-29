/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_PROBLEMNAVIGATIONCONTEXT_H
#define KDEVPLATFORM_PROBLEMNAVIGATIONCONTEXT_H

#include <interfaces/iproblem.h>
#include <interfaces/iassistant.h>

#include <language/duchain/navigation/abstractnavigationcontext.h>
#include <language/languageexport.h>

#include <QPointer>

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT ProblemNavigationContext
    : public AbstractNavigationContext
{
    Q_OBJECT

public:
    enum Flag {
        NoFlag = 0,
        ShowLocation = 1 << 0,
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    explicit ProblemNavigationContext(const QVector<IProblem::Ptr>& problems, const Flags flags = {});
    ~ProblemNavigationContext() override;

    QString name() const override;
    QString html(bool shorten = false) override;
    QWidget* widget() const override;
    bool isWidgetMaximized() const override;

    NavigationContextPointer executeKeyAction(const QString& key) override;

public Q_SLOTS:
    void executeAction(int index); // TODO: Add API in base class?

private:
    void html(IProblem::Ptr problem);

    /**
     * Return HTML-ized text. Used for processing problem's description and explanation.
     * Some plugins (kdev-cppcheck for example) return already HTML-ized strings,
     * therefore we should make check for this case.
     */
    QString escapedHtml(const QString& text) const;

    QVector<IProblem::Ptr> m_problems;
    Flags m_flags;

    QPointer<QWidget> m_widget;
    QVector<IAssistantAction::Ptr> m_assistantsActions;
};
}

#endif // KDEVPLATFORM_PROBLEMNAVIGATIONCONTEXT_H
