/**
 * The interface to compiler options configuration
 */

#ifndef _KDEVCOMPILEROPTIONS_H_
#define _KDEVCOMPILEROPTIONS_H_

#include <qobject.h>


class KDevCompilerOptions : public QObject
{
    Q_OBJECT

public:
    KDevCompilerOptions( QObject *parent=0, const char *name=0 );
    ~KDevCompilerOptions();

    /**
     * Opens a dialog which allows the user to configure the
     * compiler options. The initial settings in the dialog
     * will be set from the flags argument of this method.
     * After the dialog is accepted, the new settings will
     * be returned as a string. If the dialog was cancelled,
     * QString::null is returned.
     */
    virtual QString exec(QWidget *parent, const QString &flags) = 0;
};

#endif
