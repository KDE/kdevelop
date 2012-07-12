/*
 *
 */

#ifndef CREATETESTASSISTANT_H
#define CREATETESTASSISTANT_H

#include "kassistantdialog.h"

class CreateTestAssistantPrivate;

class CreateTestAssistant : public KAssistantDialog
{
    Q_OBJECT

public:
    explicit CreateTestAssistant (const KUrl& baseUrl, QWidget* parent = 0, Qt::WFlags flags = 0);
    virtual ~CreateTestAssistant();

    virtual void accept();
    virtual void next();

public slots:
    void templateValid(bool valid);

private:
    class CreateTestAssistantPrivate* const d;

};

#endif // CREATETESTASSISTANT_H