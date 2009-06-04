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

// Useful resources when implementing the Facebook chat protocol
// 
// http://code.google.com/p/pidgin-facebookchat/source/browse/trunk
// http://coderrr.wordpress.com/2008/05/06/facebook-chat-api/
// http://snipplr.com/view/6246/facebook-chat/
// http://imfreedom.org/wiki/Facebook
// https://www.limewire.org/fisheye/browse/limecvs/components/facebook/src/main/java/org/limewire/facebook/service/ChatClient.java?r=facebook-branch-2

#include <QObject>
#include <QDebug>
#include <QUrl>
#include <QStack>
#include <QBuffer>
#include <QTimer>

#include <QMap>
#include <QStringList>
#include <QString>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookieJar>

#include <qjson/json_driver.hh>

#include "facebook/chatservice.h"
#include "facebook/chatmessage.h"
#include "facebook/buddyinfo.h"

namespace Facebook
{

#define FACEBOOK_URL "http://www.facebook.com"
#define FACEBOOK_LOGIN_URL "https://login.facebook.com/login.php"
#define FACEBOOK_BUDDYLIST_URL "http://apps.facebook.com/ajax/presence/update.php"
#define FACEBOOK_PAGE_URL "http://www.facebook.com/presence/popout.php"
#define FACEBOOK_VISIBILITY_URL "http://apps.facebook.com/ajax/chat/settings.php"
#define FACEBOOK_STATUS_URL "http://www.facebook.com/updatestatus.php"
#define FACEBOOK_MESSAGE_URL "http://www.facebook.com/ajax/chat/send.php"
#define FACEBOOK_ACK_MESSAGE_URL "http://www.facebook.com/ajax/chat/settings.php?_ecdc=false"

// only wait one second as the rest is done by the comet style connection
#define FACEBOOK_MESSAGE_POLL_INTERVAL 1000
// poll buddylist every 3 min
#define FACEBOOK_BUDDYLIST_POLL_INTERVAL 180000

/**
 * encodes a collection of parameters and values in POST form
 */
static QString encodePostParams( QMap<QString, QString> params )
{
    QString data;
    QString formData;
    QMapIterator<QString, QString> i(params);
    QStringList paramList;
    while (i.hasNext())
    {
        i.next();
        
        QStringList bothParams;
        bothParams << i.key();
        bothParams << QUrl::toPercentEncoding(i.value().toAscii());
        paramList << bothParams.join("=");
    }
    
    data = paramList.join("&");
    return data;
    //return QString("Content-Type: application/x-www-form-urlencoded\nContent-Length: %1\n\n%2\n").arg(data.length()).arg(data);
}

/**
 * Looks for a value in HTML page, looking for the start text
 * for example '<foo name="bar" id="' until the end marker (by default a quote)
 * returns empty if not found.
 */
static QString scrapValue( const QString &page, const QString &startText, const QString &endText = "\"" )
{
    int index = page.indexOf(startText);
    if ( index == -1 )
    {
        qDebug() << "Could not find start for " << startText;
        return QString();
    }
    qDebug() << "found start text at position " << index;

    // look for the next ", or endtext
    int index2 = page.indexOf(endText, index + startText.count());
    if ( index2 == -1 )
    {
        qDebug() << "Could not find end text: " << endText;
        return QString();
    }
    qDebug() << "found end text at position " << index2;

    // move to the first quote
    index = index + startText.count();
    return page.mid(index, index2 - index);
}

ChatService::ChatService( QObject *parent )
    : QObject(parent)
    , _network(new QNetworkAccessManager(this))
    , _loggedin(false)
    , _seq(-1)
    , _message_poll_timer(new QTimer(this))
    , _buddylist_poll_timer(new QTimer(this))
{
    QObject::connect(_network, SIGNAL(sslErrors( QNetworkReply *, const QList<QSslError> &)), this, SLOT(slotSslErrors( QNetworkReply *, const QList<QSslError> & )));

    // timer for the buddylist, but we dont start it until we get the form_id
    // what we do is, once the form_id is received, if it is the form_id we get after
    // login in, then we schedule it inmediately. Otherwise we wait the interval time.
    // after the first buddy list request, after decoding we set start the timer, which
    // we stop when starting the request.
    QObject::connect(_buddylist_poll_timer, SIGNAL(timeout()), this, SLOT(startRetrieveBuddyListRequest()));
}

ChatService::~ChatService()
{

}

QString ChatService::userId() const
{
    return _user_id;    
}

void ChatService::setLoginInformation( const QString &login, const QString &pass )
{
    _login = login;
    _password = pass;
}

/**
 * The chain is as following:
 * first login request
 * when finished, page request, and get form_id
 * when finished, buddy list request
 */
void ChatService::loginToService()
{
    startLoginRequest();
    qDebug() << ">>>> login request sent";
}

void ChatService::disconnect()
{
    // clear cookies
    _network->setCookieJar(new QNetworkCookieJar());
    _loggedin = false;
    _seq = -1;
    _form_id = "";
    _availableBuddies.clear();
}

void ChatService::logoutFromService()
{
    disconnect();    
    emit logoutFromServiceFinished();
}
  
bool ChatService::isLoggedIn() const
{
    return _loggedin;
}

void ChatService::setVisibility(bool visible)
{
    qDebug() << "Setting visibility to " << visible;
    startUpdateVisibilityRequest(visible);
}

void ChatService::setStatusMessage(const QString &status)
{
    qDebug() << "Setting status to " << status;
    startUpdateStatusRequest(status);
}

void ChatService::sendMessage(const ChatMessage &message )
{
    qDebug() << "sendMessage() Sending message to " << message.to();
    startMessageSendRequest(message);
    qDebug() << "sendMessage() DONE Sending message to " << message.to();
}

void ChatService::startLoginRequest()
{
    _buddylist_poll_timer->stop();
    
    qDebug() << _network->cookieJar()->cookiesForUrl(QUrl(FACEBOOK_URL)).count() << " cookies";
    QList<QNetworkCookie> cookies;
    
    QMap<QString, QString> params;
    QUrl url(FACEBOOK_LOGIN_URL);

    QNetworkCookie cookie;
    cookie.setDomain(".facebook.com");
    cookie.setPath("/");

    // facebook uses this cookie to test for cookie support
    // if we dont set it it thinks we don't support cookies.
    cookie.setName("test_cookie");
    cookie.setValue("1");
    cookies << cookie;

    cookie.setName("isfbe");
    cookie.setValue("false");
    cookies << cookie;

    _network->cookieJar()->setCookiesFromUrl(cookies, QUrl(FACEBOOK_URL));

    qDebug() << _network->cookieJar()->cookiesForUrl(QUrl(FACEBOOK_URL)).count() << " cookies";

    // it seems those are not really needed, until I figure what
    // are they for
    //params.insert("md5pass", "0");
    //params.insert("noerror", "1");

    params.insert("email", _login);
    params.insert("pass", _password);
    params.insert("persistent", "1");
    params.insert("login", "Login");
    params.insert("charset_test", "€,´,€,´,水,Д,Є");

    QString data = encodePostParams(params);
    qDebug() << data;

    QNetworkReply *reply = _network->post(QNetworkRequest(url), data.toAscii());
    reply->setParent(this);
    
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(slotLoginRequestFinished()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotLoginRequestError(QNetworkReply::NetworkError)));
}

void ChatService::slotLoginRequestFinished()
{
    qDebug() << "connected to facebook";
    qDebug() << _network->cookieJar()->cookiesForUrl(QUrl(FACEBOOK_URL)).count() << " cookies";

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if ( !reply )
        return;

    qDebug() << reply->readAll();
    qDebug() << reply->errorString();

    // parse new cookies
    QList<QNetworkCookie> cookies;
    QStringList rawcookies = QString(reply->rawHeader("Set-Cookie")).split("\n");
    foreach (QString rawcookie, rawcookies)
    {
        qDebug() << "cookie header: " << rawcookie;
        cookies << QNetworkCookie::parseCookies(rawcookie.toAscii());
    }
    qDebug() << "Received " << cookies.count() << " cookies";
    
    _network->cookieJar()->setCookiesFromUrl(cookies, QUrl(FACEBOOK_URL));
        

    QListIterator<QByteArray> header_it(reply->rawHeaderList());
    while (header_it.hasNext())
    {
        QByteArray header = header_it.next();
        qDebug() << header << " : " << reply->rawHeader(header);
    }

    QListIterator<QNetworkCookie> i(cookies);
    while ( i.hasNext() )
    {
        QNetworkCookie cookie(i.next());
        //qDebug() << "cookie: " << cookie.name() << " : " << cookie.value();
        if (cookie.name() == "c_user")
            _user_id = cookie.value();
    }
    
    if ( _user_id.isEmpty() )
    {        
        qDebug() << "No user set";
        _loggedin = false;
        emit loginToServiceError();
        return;
    }    
    else
        qDebug() << "c_user: " << _user_id;

    qDebug() << _network->cookieJar()->cookiesForUrl(QUrl(FACEBOOK_URL)).count() << " cookies";

    // queue the job to grab the form_id
    QTimer::singleShot(0, this, SLOT(startRetrievePageRequest())); 
}

void ChatService::slotLoginRequestError(QNetworkReply::NetworkError code)
{
    qDebug() << "error on connect: " << code;
    emit loginToServiceError();
}

void ChatService::startRetrieveBuddyListRequest()
{
    // avoids two request in parallel
    _buddylist_poll_timer->stop();
   
    QMap<QString, QString> params;
    QUrl url(FACEBOOK_BUDDYLIST_URL);
    params.insert("user", _user_id);
    params.insert("notifications", "1");
    params.insert("popped_out", "false");
    params.insert("force_render", "true");
    params.insert("buddy_list", "1");

    QString data = encodePostParams(params);
    qDebug() << data;
    
    QNetworkReply *reply = _network->post(QNetworkRequest(url), data.toAscii());
    reply->setParent(this);
    
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(slotRetrieveBuddyListRequestFinished()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotRetrieveBuddyListRequestError(QNetworkReply::NetworkError)));
}

void ChatService::slotRetrieveBuddyListRequestFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if ( !reply )
        return;

    qDebug() << "got buddy list";
    decodeBuddyListResponse(reply);

    // set the next one
    _buddylist_poll_timer->start(FACEBOOK_BUDDYLIST_POLL_INTERVAL);
}

void ChatService::slotRetrieveBuddyListRequestError(QNetworkReply::NetworkError code)
{
    qDebug() << "error on retrieve buddy list: " << code;
    // set the next one
    _buddylist_poll_timer->start(FACEBOOK_BUDDYLIST_POLL_INTERVAL);
}

void ChatService::startRetrievePageRequest()
{
    QMap<QString, QString> params;
    QUrl url(FACEBOOK_PAGE_URL);
    
    QNetworkReply *reply = _network->get(QNetworkRequest(url));
    reply->setParent(this);
    
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(slotRetrievePageRequestFinished()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotRetrievePageRequestError(QNetworkReply::NetworkError)));
}

void ChatService::slotRetrievePageRequestFinished()
{
    qDebug() << "got facebook page, looking for form_id";
    
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if ( !reply )
        return;

    QString page(reply->readAll());
    
    _form_id = scrapValue(page, "id=\"post_form_id\" name=\"post_form_id\" value=\"");
    
    if ( _form_id.isEmpty() )
    {
        qDebug() << "Could not find form_id" << _form_id;
        return;
    }
    
    qDebug() << "Found form_id: " << _form_id;

    qDebug() << "looking for channel id";
    _channel = scrapValue(page, "\", \"channel");
    
    if ( _form_id.isEmpty() )
    {
        qDebug() << "Could not find form_id" << _form_id;
        _loggedin = false;
        emit loginToServiceError();
        return;
    }

    qDebug() << "Found channel: " << _channel;

    // also ask for the buddylist. inmediatelly if we are loggin in
    if ( ! _loggedin )
        QTimer::singleShot(0, this, SLOT(startRetrieveBuddyListRequest()));
    else
        QTimer::singleShot(FACEBOOK_BUDDYLIST_POLL_INTERVAL, this, SLOT(startRetrieveBuddyListRequest()));

    // now that we have the channel and form_id, start polling for messages
    QTimer::singleShot(FACEBOOK_MESSAGE_POLL_INTERVAL, this, SLOT(startGetMessagesRequest()));

    // now we have the form id
    if ( ! _loggedin )
    {
        qDebug() << "Finished login!";
        _loggedin = true;
        emit loginToServiceFinished();
    }
}

void ChatService::slotRetrievePageRequestError(QNetworkReply::NetworkError code)
{
    qDebug() << "error on retrieve form_id page: " << code;
}

void ChatService::slotSslErrors( QNetworkReply * reply, const QList<QSslError> & errors )
{
    Q_UNUSED(reply);
    Q_UNUSED(errors);
    
    qDebug() << "ssl error";
}

void ChatService::startUpdateVisibilityRequest(bool visible)
{
    QMap<QString, QString> params;
    QUrl url(FACEBOOK_VISIBILITY_URL);
    // visibility=true&post_form_id=1234
    params.insert("visibility", visible ? "true" : "false" );
    params.insert("post_form_id", _form_id );

    QString data = encodePostParams(params);
    QNetworkReply *reply = _network->post(QNetworkRequest(url), data.toAscii());
    reply->setParent(this);
    
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(slotUpdateVisibilityRequestFinished()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotUpdateVisibilityRequestError(QNetworkReply::NetworkError)));
}

void ChatService::slotUpdateVisibilityRequestFinished()
{
    qDebug() << "visibility updated";
}

void ChatService::slotUpdateVisibilityRequestError(QNetworkReply::NetworkError code)
{
    qDebug() << "error on when setting visibility" << code;
}

void ChatService::startUpdateStatusRequest(const QString &status)
{
    qDebug() << "starting update status request...";    
    QMap<QString, QString> params;
    QUrl url(FACEBOOK_STATUS_URL);
    // visibility=true&post_form_id=1234
    params.insert("status", status );

    QString data = encodePostParams(params);
    QNetworkReply *reply = _network->post(QNetworkRequest(url), data.toAscii());
    reply->setParent(this);
    
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(slotUpdateStatusRequestFinished()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotUpdateStatusRequestError(QNetworkReply::NetworkError)));
}


void ChatService::slotUpdateStatusRequestFinished()
{
    qDebug() << "update status request completed";
}

void ChatService::slotUpdateStatusRequestError(QNetworkReply::NetworkError code)
{
    qDebug() << "error on update status request" << code;
}

void ChatService::startMessageSendRequest(const ChatMessage &message)
{
    QMap<QString, QString> params;
    QUrl url(FACEBOOK_MESSAGE_URL);
    // msg_text={message}&msg_id=3409501070&client_time={timestamp}&to={uid}&popped_out=true&num_tabs=1&post_form_id=1234
    params.insert("msg_text", message.text() );

    qDebug() << "sent request for message to " << message.to() << " with message id " << message.messageId();
    params.insert("msg_id", message.messageId() );
    params.insert("client_time", QString::number(QDateTime::currentDateTime().toTime_t()));
    params.insert("to", message.to());
    params.insert("popped_out", "true");
    params.insert("num_tabs", "1");
    params.insert("post_form_id", _form_id);

    _messageQueue[message.messageId()] = message;
    
    QString data = encodePostParams(params);

    // pass the message id as an attribute
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, message.messageId());

    qDebug() << "startMessageSendRequest() posting to " << url;
    QNetworkReply *reply = _network->post(request, data.toAscii());
    qDebug() << "startMessageSendRequest() going to set parent";    
    reply->setParent(this);   
    qDebug() << "startMessageSendRequest() connecting signals";
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(slotMessageSendRequestFinished()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotMessageSendRequestError(QNetworkReply::NetworkError)));
    qDebug() << "startMessageSendRequest() done";
}


void ChatService::slotMessageSendRequestFinished()
{
    qDebug() << "request finished, message sent, emmiting ack";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if ( !reply )
        return;
    QString messageid = reply->request().attribute(QNetworkRequest::User).toString();
    if ( messageid.isEmpty() )
        return;

    if ( ! _messageQueue.contains(messageid) )
        return;
    
    emit messageSendFinished(_messageQueue[messageid]);
    _messageQueue.remove(messageid);
}

void ChatService::slotMessageSendRequestError(QNetworkReply::NetworkError code)
{
    qDebug() << "request finished with error , message not sent, emmiting error";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if ( !reply )
        return;
    QString messageid = reply->request().attribute(QNetworkRequest::User).toString();
    if ( messageid.isEmpty() )
        return;

    if ( ! _messageQueue.contains(messageid) )
        return;
    
    emit messageSendError(_messageQueue[messageid]);
    _messageQueue.remove(messageid);

}

QUrl ChatService::incomingMessagesUrl( int seq )
{
    return QUrl(QString("http://0.channel%1.facebook.com/x/0/false/p_%2=%3").arg(_channel).arg(_user_id).arg(seq));
}

void ChatService::startGetMessagesRequest()
{
    // get the url for the seq number.
     // the initial sq is -1 which is always invalid, but generates a reponse
    // which allows to get the real one
    QUrl url(incomingMessagesUrl(_seq));
    qDebug() << "may be facebook has messages for us, asking: " << url;

    QNetworkReply *reply = _network->get(QNetworkRequest(url));
    reply->setParent(this);
    
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(slotGetMessagesRequestFinished()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotGetMessagesRequestError(QNetworkReply::NetworkError)));
}
    
void ChatService::slotGetMessagesRequestFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if ( !reply )
        return;

    //qDebug() << reply->readAll();
    decodeGetMessagesResponse(reply);
}

void ChatService::slotGetMessagesRequestError(QNetworkReply::NetworkError code)
{
    qDebug() << "error on when sending message" << code;
}

void ChatService::decodeGetMessagesResponse( QIODevice *input )
{
    qDebug() << "looking for incoming messages or new seq";

    // read the useless for
    input->read(QString("for (;;);").count());
    QString json = input->readAll();
    
    JSonDriver parser;
    
    bool status = true;
    QVariant result = parser.parse(json, &status);
    
    qDebug() << json;

    if (!status)
    {
        // default keep old seq
        int newSeq = _seq;
        // may be we are asked for a refresh
        // "for (;;);{"t":"refresh", "seq":0}"

        if ( ! result.canConvert(QVariant::Map) )
        {
            // error
            qDebug() << "Server message reply is not a map";
            return;
        }
        
        QString tValue = result.toMap()["t"].toString();
        
        // in case we got a new seq,
        if ( tValue == "continue" )
        {
            // if not, parse the message and set a poll for later
        }
        else if ( tValue == "refresh" )
        {
            int oldSeq = _seq;
            newSeq = result.toMap()["seq"].toInt();
            _seq = newSeq;

            if ( oldSeq < 0 )
            {
                // we have the old seq -1, so we only got the initial seq
                // on the next message poll we will get messages
                qDebug() << "got initial seq: " << newSeq;
                // we can't return so we set the next message get
                // shot
            }
            else if ( oldSeq == 0 && newSeq == 0 )
            {
                // our old seq was zero, and now it got reseted, to zero
                // which may  be some problem.
                qDebug() << "bad: old seq and new seq are 0";                
                disconnect();
                emit error(ErrorDisconnected, "");
                return;                
            }
            else
            {
                // we had a normal seq, even zero, and it got resetted, this
                // means we have to read the channel value and form_id again
                qDebug() << "seq reset by server";                
                QTimer::singleShot(0, this, SLOT(startRetrievePageRequest()));
                return;                
            }                                
        }
        else if ( tValue == "msg" )
        {
            // We are interested in the ms entry in the map
            // "ms":[ {"type":"msg","msg":{"text":"i'm sending u a message","time":1209548203464,"clientTime":1209548202312,"msgID":"3409501070"},"from":800753867,"to":596176850,"from_name":"Eion Robb","to_name":"Jeremy Lawson","from_first_name":"Eion","to_first_name":"Jeremy"}]
            // check that ms is a list
            if ( result.toMap()["ms"].canConvert(QVariant::List) )
            {
                QVariantList mslist = result.toMap()["ms"].toList();
                // now every item in the ms list is a message
                foreach( QVariant ms, mslist )
                {
                    // every item in the ms array is either a type event
                    // or a message event
                    if ( ! ms.canConvert(QVariant::Map) )
                    {
                        qDebug() << "Error decoding message item";
                        continue;
                    }
                    
                    if ( ms.toMap()["type"].toString() == "typ" )
                    {
                        // typing event
                        ChatMessage message;
                        if ( message.readVariant(ms.toMap()) )
                        {
                            qDebug() << "typing from: " << message.from() << " to " << message.to();                            
                            emit typingEventAvailable(message.from(), message.to());
                        }
                        else
                        {
                            qDebug() << "Error decoding message";
                            continue;
                        }
                    }
                    else if ( ms.toMap()["type"].toString() == "msg" )
                    {
                        // message event, get the message key for decoding
                        ChatMessage message;
                        if ( message.readVariant(ms.toMap()) )
                        {
                            qDebug() << message;
                            emit messageAvailable(message);
                            // send back a ack simulation
                            // however we dont care about the result
                            // the ack only if the message is not set _to_ us
                            if ( message.from() != userId() )
                            {
                                qDebug() << "sending ack for message from " << message.fromName();                                
                                QMap<QString,QString> params;
                                params.insert("focus_chat", message.from() );
                                params.insert("windows_id", "12345" );                                                    params.insert("post_form_id", _form_id );
                                QString data = encodePostParams(params);
                                QUrl ackurl(FACEBOOK_ACK_MESSAGE_URL);
                                QNetworkReply *reply = _network->post(QNetworkRequest(ackurl), data.toAscii() );
                                reply->setParent(this);
                                QObject::connect(reply, SIGNAL(finished()), this, SLOT(slotMessageAckRequestFinished()));
                                QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotMessageAckRequestError(QNetworkReply::NetworkError)));                            
                            }
                        }                        
                        else
                        {
                            qDebug() << "Error decoding message";
                            continue;
                        }
                    }
                }
            }
            else
            {
                // complain
            }
            
            
            qDebug() << "We got a message!!!";
            // process it
            _seq++;
        }
    
        // setup next poll
        QTimer::singleShot(FACEBOOK_MESSAGE_POLL_INTERVAL, this, SLOT(startGetMessagesRequest()));
    }
    else
    {
        qDebug() << "invalid reply: " << parser.error();
        qDebug() << json;
        
    }
}

void ChatService::slotMessageAckRequestFinished()
{
    qDebug() << "message ack done";    
}


void ChatService::slotMessageAckRequestError(QNetworkReply::NetworkError code)
{
    Q_UNUSED(code);
}

void ChatService::decodeBuddyListResponse( QIODevice *responseInput )
{
    bool error = false;
    bool errorIsWarning = false;
    QString errorSummary;
    QString errorDesc;
    bool listChanged = true;
    
    // we could pass the input IO device
    // directly to the JSON reader but we need
    // first to strip the for{;;};" string

    // we do loser way and just skip 9
    // bytes from the original input and give
    // that to the json parser, but if facebook changes
    // the format we will have hard time figuring out
    responseInput->read(QString("for (;;);").count());
    QString json = responseInput->readAll();

    JSonDriver parser;    
    bool status = true;
    QVariant result = parser.parse(json, &status);
    
    if (!status)
    {
        // the whole reply is a map
        if ( ! result.canConvert(QVariant::Map) )
        {
            qDebug() << "invalid buddy list response";
            return;
        }
        error = result.toMap()["error"].toBool();
        errorSummary = result.toMap()["errorSummary"].toString();
        errorDesc = result.toMap()["errorDescription"].toString();
        
        QVariantMap payload = result.toMap()["payload"].toMap();
        QVariantMap buddy_list = payload["buddy_list"].toMap();
        listChanged = buddy_list["listChanged"].toBool();
        int availableCount = buddy_list["availableCount"].toInt();
        QVariantMap userInfos = buddy_list["userInfos"].toMap();

        if ( ! listChanged )
        {            
            qDebug() << "buddy list did not change. " << availableCount << " buddies available" ;
            return;            
        }
        
        
        foreach (QString userId, userInfos.keys())
        {
            BuddyInfo buddy;
            buddy.setBuddyId(userId);
            if ( ! buddy.readVariant(userInfos.value(userId)) )
            {
                qDebug() << "invalid buddy";
                qDebug() << buddy;
            }
            qDebug() << "got buddy: " << buddy;            
            _buddyInfos[userId] = buddy;
            emit buddyInformation(buddy);
        }
        QVariantMap nowAvailableList = buddy_list["nowAvailableList"].toMap();

        // look all contacts in our list that were not in the new
        // available list and inform them as offline
        foreach (QString userId, _availableBuddies.keys() )
        {
            if ( ! nowAvailableList.contains(userId) )
            {
                _availableBuddies.remove(userId);                
                if ( _buddyInfos.contains(userId) )
                    emit buddyNotAvailable(_buddyInfos.value(userId));
                else
                    qDebug() << "no info for buddy " << userId;
            }
            
        }

        // update available contacts
        foreach (QString userId, nowAvailableList.keys())
        {
            bool idle = nowAvailableList.value(userId).toMap()["i"].toBool();
            if ( _buddyInfos.contains(userId) )
            {
                // if the user id is already there with the same idle status, don't emit
                // anything
                if ( _availableBuddies.contains(userId) && 
                     ( _availableBuddies.value(userId) == idle ) )
                    continue;

                _availableBuddies[userId] = idle;                
                emit buddyAvailable(_buddyInfos.value(userId), _availableBuddies[userId]);
            }           
            else
            {                
                qDebug() << "no info for buddy " << userId;         
            }            
        }        
    }
    else
    {
        qDebug() << "invalid json for buddy list";
    }
}

void ChatService::requestPicture( const QString &buddyid )
{
    startRetrievePictureRequest(buddyid);
}

void ChatService::startRetrievePictureRequest( const QString &buddyid )
{
    if ( ! _buddyInfos.contains(buddyid) )
        return;
    
    QUrl url = QUrl(_buddyInfos.value(buddyid).thumbSrc());
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, buddyid);
    
    qDebug() << "requesting photo for " << buddyid << " at " << url;
    QNetworkReply *reply = _network->get(request);
    reply->setParent(this);
    
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(slotRetrievePictureRequestFinished()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotRetrievePictureRequestError(QNetworkReply::NetworkError)));
}
  
void ChatService::slotRetrievePictureRequestFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    if ( !reply )
        return;

    QString buddyid = reply->request().attribute(QNetworkRequest::User).toString();

    if ( buddyid.isEmpty() )
    {
        qDebug() << "photo for unknown buddy";
        return;
    }
    
    QImage image(QImage::fromData(reply->readAll()));
    if ( image.format() == QImage::Format_Invalid )
    {
        qDebug() << "photo for " << buddyid << " is invalid";
        return;
    }

    emit buddyThumbAvailable(buddyid, image);
}

void ChatService::slotRetrievePictureRequestError(QNetworkReply::NetworkError code)
{
    qDebug() << "error retrieving picture";
}

} //ns
 
