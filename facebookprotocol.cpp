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

#include <QList>
#include <kgenericfactory.h>
#include <kdebug.h>

#include "kopeteaccountmanager.h"
#include "facebookaccount.h"
#include "facebookcontact.h"
#include "facebookprotocol.h"
#include "facebookaddcontactpage.h"
#include "facebookeditaccountwidget.h"

K_PLUGIN_FACTORY( FacebookProtocolFactory, registerPlugin<FacebookProtocol>(); )
K_EXPORT_PLUGIN( FacebookProtocolFactory( "kopete_facebook" ) )

FacebookProtocol *FacebookProtocol::s_protocol = 0L;

FacebookProtocol::FacebookProtocol( QObject* parent, const QVariantList &/*args*/ )
    : Kopete::Protocol( FacebookProtocolFactory::componentData(), parent )
    , facebookOnline(  Kopete::OnlineStatus::Online, 25, this, 0,  QStringList(), i18n( "Online" ), i18n( "O&nline" ), Kopete::OnlineStatusManager::Online )
    , facebookAway(  Kopete::OnlineStatus::Away, 25, this, 1, QStringList(QLatin1String("msn_away")), i18n( "Away" ), i18n( "&Away" ), Kopete::OnlineStatusManager::Away )
    , facebookOffline(  Kopete::OnlineStatus::Offline, 25, this, 2,  QStringList(QString()), i18n( "Offline" ),   i18n( "O&ffline" ), Kopete::OnlineStatusManager::Offline )
    , facebookConnecting(  Kopete::OnlineStatus::Connecting, 25, this, 2,  QStringList("facebook_connecting"), i18n( "Connecting" ), i18n( "C&onnecting" ), 0, Kopete::OnlineStatusManager::HideFromMenu )
      
{
    kDebug( 14210 ) ;
    
    s_protocol = this;
}

FacebookProtocol::~FacebookProtocol()
{
}

Kopete::Contact *FacebookProtocol::deserializeContact(
    Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData,
    const QMap<QString, QString> &/* addressBookData */)
{
    QString contactId = serializedData[ "contactId" ];
    QString accountId = serializedData[ "accountId" ];
    QString displayName = serializedData[ "displayName" ];
    QString type = serializedData[ "contactType" ];
    
    FacebookContact::Type tbcType;
    if ( type == QLatin1String( "group" ) )
        tbcType = FacebookContact::Group;
    else if ( type == QLatin1String( "echo" ) )
        tbcType = FacebookContact::Echo;
    else if ( type == QLatin1String( "null" ) )
        tbcType = FacebookContact::Null;
    else
        tbcType = FacebookContact::Null;
    
    QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts( this );
    Kopete::Account* account = 0;
    foreach( Kopete::Account* acct, accounts )
    {
        if ( acct->accountId() == accountId )
            account = acct;
    }
    
    if ( !account )
    {
        kDebug(14210) << "Account doesn't exist, skipping";
        return 0;
    }
    
    FacebookContact * contact = new FacebookContact(account, contactId, displayName, metaContact);
    contact->setType( tbcType );
    return contact;
}

AddContactPage * FacebookProtocol::createAddContactWidget( QWidget *parent, Kopete::Account * /* account */ )
{
    kDebug( 14210 ) << "Creating Add Contact Page";
    return new FacebookAddContactPage( parent );
}

KopeteEditAccountWidget * FacebookProtocol::createEditAccountWidget( Kopete::Account *account, QWidget *parent )
{
    kDebug(14210) << "Creating Edit Account Page";
    return new FacebookEditAccountWidget( parent, account );
}

Kopete::Account *FacebookProtocol::createNewAccount( const QString &accountId )
{
    return new FacebookAccount( this, accountId );
}

FacebookProtocol *FacebookProtocol::protocol()
{
    return s_protocol;
}

#include "facebookprotocol.moc"
