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

#include "facebookaddcontactpage.h"

#include <qlayout.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <qlineedit.h>
#include <kdebug.h>

#include "kopeteaccount.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"

#include "facebookprotocol.h"
#include "facebookcontact.h"

FacebookAddContactPage::FacebookAddContactPage( QWidget* parent )
    : AddContactPage(parent)
{
    kDebug(FBDBG) ;
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget();
    m_facebookAddUI.setupUi( w );
    l->addWidget( w );
    m_facebookAddUI.m_uniqueName->setFocus();
}

FacebookAddContactPage::~FacebookAddContactPage()
{
}

bool FacebookAddContactPage::apply( Kopete::Account* a, Kopete::MetaContact* m )
{
    if ( validateData() )
    {
	QString name = m_facebookAddUI.m_uniqueName->text();
	
	if ( a->addContact(name, m, Kopete::Account::ChangeKABC ) )
	{
	    FacebookContact * newContact = qobject_cast<FacebookContact*>( Kopete::ContactList::self()->findContact( a->protocol()->pluginId(), a->accountId(), name ) );
	    if ( newContact )
	    {
		newContact->setType( m_facebookAddUI.m_rbEcho->isChecked() ? FacebookContact::Echo : FacebookContact::Group );
		return true;
	    }
	}
	else
	    return false;
    }
    return false;
}

bool FacebookAddContactPage::validateData()
{
    return true;
}


#include "facebookaddcontactpage.moc"
