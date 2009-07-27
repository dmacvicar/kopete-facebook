/*
    Kopete Facebook Protocol implementation
    This code is not associated with Facebook in any way.

    Copyright (c) 2009 Duncan Mac-Vicar P. <duncan@kde.org>
    Kopete    (c) 2002-2009 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef FACEBOOKADDCONTACTPAGE_H
#define FACEBOOKADDCONTACTPAGE_H

#include <addcontactpage.h>
#include "ui_facebookadd.h"

namespace Kopete { class Account; }
namespace Kopete { class MetaContact; }
namespace Ui { class FacebookAddUI; }

/**
 * A page in the Add Contact Wizard
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
*/
class FacebookAddContactPage : public AddContactPage
{
	Q_OBJECT
public:
  FacebookAddContactPage( QWidget* parent = 0 );
  ~FacebookAddContactPage();
	
    /**
     * Make a contact out of the entered data
     */
  virtual bool apply(Kopete::Account* a, Kopete::MetaContact* m);
  /**
   * Is the data correct?
   */
  virtual bool validateData();

protected:
  Ui::FacebookAddUI m_facebookAddUI;
};

#endif
