/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SNIPPETVARIABLESUBST_H__
#define __SNIPPETVARIABLESUBST_H__

#include "ui_snippetvariablesubst.h"

class SnippetVariableModel;

/**
 * A dialog that allows to enter the values for the variables
 * that have been added to the SnippetVariableModel @p model
 * @see snippetvariables.ui
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class SnippetVariables: public QDialog, public Ui::SnippetVariableSubstBase {
Q_OBJECT
public:
    explicit SnippetVariables(SnippetVariableModel* model, QWidget *parent = 0);
};

#endif
