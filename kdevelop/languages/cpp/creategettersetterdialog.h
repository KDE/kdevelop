//
// C++ Interface: $MODULE$
//
// Description:
//
//
// Author: Jonas Jacobi <j.jacobi@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CREATEACCESSMETHODSDIALOG_H
#define CREATEACCESSMETHODSDIALOG_H

#include "creategettersetter.h"
#include "codemodel.h"

class CppSupportPart;

/**
 * Dialog which is shown, when a user wants to create get/set methods for a class attribute.
 * @author Jonas Jacobi <j.jacobi@gmx.de>
 */
class CreateGetterSetterDialog: public CreateGetterSetterDialogBase
{
	Q_OBJECT
public:
	CreateGetterSetterDialog(CppSupportPart* part, ClassDom aClass, VariableDom var, QWidget *parent = 0, const char *name = 0);

protected slots:
	void accept();
	void slotInlineChanged();
	
private:
	CppSupportPart* m_part;

	ClassDom m_class;
	VariableDom m_var;
};

#endif
