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

#ifndef KOPETEFACEBOOKPROTOCOL_H
#define KOPETEFACEBOOKPROTOCOL_H

#include <kopeteprotocol.h>

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class FacebookProtocol : public Kopete::Protocol
{
    Q_OBJECT
public:
    FacebookProtocol(QObject *parent, const QVariantList &args);
    ~FacebookProtocol();
    /**
     * Convert the serialised data back into a FacebookContact and add this
     * to its Kopete::MetaContact
     */
    virtual Kopete::Contact *deserializeContact( Kopete::MetaContact *metaContact,
                                                 const QMap< QString, QString > & serializedData,
                                                 const QMap< QString, QString > & addressBookData
                                                 );
    /**
     * Generate the widget needed to add FacebookContacts
     */
    virtual AddContactPage * createAddContactWidget( QWidget *parent, Kopete::Account *account );
    /**
     * Generate the widget needed to add/edit accounts for this protocol
     */
    virtual KopeteEditAccountWidget * createEditAccountWidget( Kopete::Account *account, QWidget *parent );
    /**
     * Generate a FacebookAccount
     */
    virtual Kopete::Account * createNewAccount( const QString &accountId );
    /**
     * Access the instance of this protocol
     */
    static FacebookProtocol *protocol();
    /**
     * Represents contacts that are Online
     */
    const Kopete::OnlineStatus facebookOnline;
    /**
     * Represents contacts that are Away
     */
    const Kopete::OnlineStatus facebookAway;
    /**
     * Represents contacts that are Offline
     */
    const Kopete::OnlineStatus facebookOffline;
    /**
     * Represents when we are connecting
     */
    const Kopete::OnlineStatus facebookConnecting;

 protected:
    static FacebookProtocol *s_protocol;
};

#endif
