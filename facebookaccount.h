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

#ifndef FACEBOOKACCOUNT_H
#define FACEBOOKACCOUNT_H

#include "kopetepasswordedaccount.h"
#include "facebook/chatservice.h"

class KActionMenu;
namespace Kopete 
{ 
	class Contact;
	class MetaContact;
	class StatusMessage;
}

class FacebookProtocol;
class FacebookContact;

using Facebook::ChatMessage;
using Facebook::BuddyInfo;

/**
 * A Facebook account with its own session
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
*/
class FacebookAccount : public Kopete::PasswordedAccount
{
    Q_OBJECT
public:
    FacebookAccount( FacebookProtocol *parent, const QString& accountID );
    ~FacebookAccount();

    /**
     * return contact with id
     */
    FacebookContact *contact( const QString &id );

    /**
     * Construct the context menu used for the status bar icon
     */
    virtual void fillActionMenu( KActionMenu *actionMenu );
    
    /**
     * Creates a protocol specific Kopete::Contact subclass and adds it to the supplie
     * Kopete::MetaContact
     */
    virtual bool createContact(const QString& contactId, Kopete::MetaContact* parentContact);
    /**
     * Called when Kopete is set globally away
     */
    virtual void setAway(bool away, const QString& reason);
    /**
     * Called when Kopete status is changed globally
     */
    virtual void setOnlineStatus(const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
				 const OnlineStatusOptions& options = None);

    virtual void setStatusMessage(const Kopete::StatusMessage& statusMessage);

    /**
     * Connect to the server
     */
    virtual void connectWithPassword (const QString & password);

    /**
     * Disconnect from the server.  Only sets myself() offline.
     */
    virtual void disconnect();

    /**
     * Facebook service session
     */
    Facebook::ChatService *service();
    
public slots:

    /**
     * Called by the server when it has a message for us.
     * This identifies the sending Kopete::Contact and passes it a Kopete::Message
     */
    void receivedMessage( const QString &message );
    
protected:
    Facebook::ChatService *m_service;

protected slots:

    /**
     * login to server completed
     */
    void slotLoginToServiceFinished();

    /**
     * login to server failed
     */
    void slotLoginToServiceError();

    /**
     * login to server completed
     */
    void slotLogoutFromServiceFinished();

    /**
     * login to server failed
     */
    void slotLogoutFromServiceError();

    /**
     * a message is available
     */
    void slotMessageAvailable( const ChatMessage &message );

    /**
     * buddy on available list
     */
    void slotBuddyAvailable( const BuddyInfo &buddy, bool idle );

    /**
     * buddy on available list
     */
    void slotBuddyNotAvailable( const BuddyInfo &buddy );

    /**
     * a buddy is known
     */
    void slotBuddyInformation( const BuddyInfo &buddy );

    /**
     * picture available
     */
    void slotBuddyThumbAvailable( const QString &buddyid, const QImage &image );

    /**
     * Change the account's status.  Called by KActions and internally.
     */
    void slotGoOnline();
    /**
     * Change the account's status.  Called by KActions and internally.
	 */
    void slotGoAway();
    /**
     * Change the account's status.  Called by KActions and internally.
     */
    void slotGoOffline();

    /**
     * someone is typing
     */
    void slotTypingEventAvailable( const QString &from, const QString &to );

    /**
     * something bad happened
     */
    void slotError( const QString &error );

};

#endif
