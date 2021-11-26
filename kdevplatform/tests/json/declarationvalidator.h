/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DECLARATIONVALIDATOR_H
#define KDEVPLATFORM_DECLARATIONVALIDATOR_H

#include <tests/testsexport.h>

#include "language/duchain/ducontext.h"

#include <QMap>
#include <QVariant>

#include <functional>

namespace KDevelop {
class DeclarationValidatorPrivate;

class KDEVPLATFORMTESTS_EXPORT DeclarationValidator
    : public DUChainVisitor
{
public:
    using TestDataEditor = std::function<void(QVariantMap& testData)>;
    explicit DeclarationValidator(TestDataEditor testDataAdjuster = nullptr);
    ~DeclarationValidator() override;

    virtual bool testsPassed() const;
    void visit(DUContext*) override;
    void visit(Declaration* declaration) override;

private:
    Q_DISABLE_COPY(DeclarationValidator)
    const QScopedPointer<class DeclarationValidatorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DeclarationValidator)
};
}
#endif //KDEVPLATFORM_DECLARATIONVALIDATOR_H
