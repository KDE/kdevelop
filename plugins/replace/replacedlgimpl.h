
#ifndef REPLACEDLGIMPL_H
#define REPLACEDLGIMPL_H

#include "replacedlg.h"

class QDialog;

class ReplaceDlgImpl : public ReplaceDlg
{
    Q_OBJECT

public:
    ReplaceDlgImpl(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~ReplaceDlgImpl();

    QRegExp expressionPattern();
    QString replacementString();

public slots:
    void show( QString const & path );

protected:

protected slots:
    void showRegExpEditor();
    void validateExpression( const QString & );
    void validateFind( const QString & );
    void toggleExpression( bool );
    void saveComboHistories();

private:
    QDialog * _regexp_dialog;

};

#endif

