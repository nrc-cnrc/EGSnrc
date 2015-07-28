/*
###############################################################################
#
#  EGSnrc egs_inprz about form
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Blake Walters, 2015
#
#  Contributors:
#
###############################################################################
*/


//qt3to4 -- BW
//entire new include file to implement AboutForm container class for AboutForm form!
#include "aboutform_rzImpl.h"

#include <qvariant.h>
/*
 *  Constructs a AboutForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
AboutForm::AboutForm(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : QDialog(parent)
{
    setupUi(this);

}

/*
 *  Destroys the object and frees any allocated resources
 */
AboutForm::~AboutForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AboutForm::languageChange()
{
    retranslateUi(this);
}

