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

#ifndef FACEBOOKCHATSERVICE_H
#define FACEBOOKCHATSERVICE_H

#include <QObject>
#include <QList>
#include <QUrl>
#include <QtNetwork/QNetworkReply>
#include <QDataStream>
#include <QImage>

#include "facebook/buddyinfo.h"
#include "facebook/chatmessage.h"

class QNetworkAccessManager;
class QTimer;

namespace Facebook
{

/**
 * Interaction with Facebook server
 *
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class ChatService : public QObject
{
  Q_OBJECT
public:
  /**
   * Construct a object to talk to the facebook service
   */
  ChatService(QObject *parent);
  ~ChatService();

  enum ErrorType
  {
    ErrorUnknown = 0x0,
    ErrorDisconnected = 0x1
  };

  /**
   * Sets the login information. This is required for @ref connecToService
   * to succeed
   */
  void setLoginInformation( const QString &login, const QString &pass );

  /**
   * logs in to the service
   */
  void loginToService();

  /**
   * logout from service
   */
  void logoutFromService();

  /**
   * Returns true if we are logged in in the service
   */
  bool isLoggedIn() const;

  /**
   * the user id of the owner of the session
   * typically a number like 5454355
   */
  QString userId() const;

  /**
   * set the visibility status
   */
  void setVisibility(bool visible);

  /**
   * set the status message
   */
  void setStatusMessage(const QString &status);

  /**
   * sends a message
   */
  void sendMessage( const ChatMessage &message );

  /**
   * request picture
   */
  void requestPicture( const QString &buddyid );

protected slots:
  /**
   * cleanups cookies and reset internal structures
   */
  void disconnect();

  // The following functions are the actual requests to the
  // facebook server

  /**
   * Logs in to facebook
   */
  void startLoginRequest();

  /**
   * Initiates a request to the home page
   * in order to find the important
   * post_form_id parameter
   */
  void startRetrievePageRequest();

  /**
   * Initiates a buddy list request
   */
  void startRetrieveBuddyListRequest();

  /**
   * Initiates a buddy list request
   */
  void startUpdateVisibilityRequest(bool visible);

  /**
   * Initiates a message status update request
   */
  void startUpdateStatusRequest(const QString &status);

  /**
   * Initiates a message send request
   */
  void startMessageSendRequest( const ChatMessage &message );

  /**
   * Initiates a request to read incoming messages
   */
  void startGetMessagesRequest();

  /**
   * start retrieving the picture from a url
   */
  void startRetrievePictureRequest( const QString &buddyid );
 
protected slots:
  // the following slots handle the result of the requests
  // functions

  /**
   * Handles the response from a login request
   * when it is finished
   */
  void slotLoginRequestFinished();
  /**
   * Handles the response from a login request
   * when it has failed
   */
  void slotLoginRequestError(QNetworkReply::NetworkError code);
  /**
   * Handles the response from a buddy list request
   * when it is finished
   */
  void slotRetrieveBuddyListRequestFinished();
  /**
   * Handles the response from a buddy list request
   * when it has failed
   */
  void slotRetrieveBuddyListRequestError(QNetworkReply::NetworkError code);

  /**
   * Handles the response from the page request used to get the form_id
   * when it is finished
   */
  void slotRetrievePageRequestFinished();
  /**
   * Handles the response from the page request used to get the form_id
   * when it has failed
   */
  void slotRetrievePageRequestError(QNetworkReply::NetworkError code);

  /**
   * Handles the response from the page request to update visibility
   * when it is finished
   */
  void slotUpdateVisibilityRequestFinished();
  /**
   * Handles the response from the page request to update visibility
   * when it has failed
   */
  void slotUpdateVisibilityRequestError(QNetworkReply::NetworkError code);

  /**
   * Handles the response from the page request to update status
   * when it is finished
   */
  void slotUpdateStatusRequestFinished();
  /**
   * Handles the response from the page request to update status
   * when it has failed
   */
  void slotUpdateStatusRequestError(QNetworkReply::NetworkError code);

  /**
   * Handles the response from the page request to send a message
   * when it is finished
   */
  void slotMessageSendRequestFinished();
  /**
   * Handles the response from the page request to send a message
   * when it has failed
   */
  void slotMessageSendRequestError(QNetworkReply::NetworkError code);

  /**
   * Handles the response from the page request to get messages
   * when it is finished
   */
  void slotGetMessagesRequestFinished();
  /**
   * Handles the response from the page request to get messages
   * when it has failed
   */
  void slotGetMessagesRequestError(QNetworkReply::NetworkError code);

  /**
   * Handles the response from any connection when there are
   * SSL errors
   */
  void slotSslErrors ( QNetworkReply * reply, const QList<QSslError> & errors );

  /**
   * Decodes a buddylist response and generate data or error events
   */
  void decodeBuddyListResponse( QIODevice *input );

  /**
   * Decodes a read messages response and generate data or error events
   */
  void decodeGetMessagesResponse( QIODevice *input );

  /**
   * Error when retrieving picture
   */
  void slotRetrievePictureRequestFinished();

  /**
   * error when retrieving picture. 
   */
  void slotRetrievePictureRequestError(QNetworkReply::NetworkError code);

  /**
   * Handles the response from the message ack request
   * when it is finished
   */
  void slotMessageAckRequestFinished();
  /**
   * Handles the response from the message ack request
   * when it has failed
   */
  void slotMessageAckRequestError(QNetworkReply::NetworkError code);


protected:
  /**
   * returns the incoming message url for the given sequence number
   */
  QUrl incomingMessagesUrl( int seq );

signals:
  void loginToServiceFinished();
  void loginToServiceError();
  void logoutFromServiceFinished();
  void logoutFromServiceError();

  /**
   * emitted when there is information available about a buddy
   */
  void buddyInformation( const BuddyInfo &info );

  /**
   * emitted when a buddy is available (may be idle though)
   */
  void buddyAvailable( const BuddyInfo &info , bool idle);

  /**
   * emitted when a buddy is available (may be idle though)
   */
  void buddyNotAvailable( const BuddyInfo &info );
  
  /**
   * emitted when there is a message available for processing.
   * note that messages that we send also come here
   *
   * You need to decide if your ack is when the send request
   * is complete, or when the server sends the message back to
   * you
   */
  void messageAvailable( const ChatMessage &message );

  /**
   * message request completed
   */
  void messageSendFinished( const ChatMessage &message );

  /**
   * message request ended with error
   */
  void messageSendError( const ChatMessage &message );

  /**
   * emitted when a photo has been downloaded
   */
  void buddyThumbAvailable( const QString &buddyid, const QImage &image );

  /**
   * emmited when one user sends a typing event to the other
   */
  void typingEventAvailable( const QString &from, const QString &to );

  /**
   * emitted when something bad happens
   */
  void error( ErrorType error, const QString &description );

private:
  QNetworkAccessManager *_network;
  bool _loggedin;

  // user id
  QString _user_id;
  QString _login;
  QString _password;
  QString _form_id;
  QString _channel;

  int _seq;

  QTimer *_message_poll_timer;
  QTimer *_buddylist_poll_timer;

  QMap<QString, BuddyInfo> _buddyInfos;
  QMap<QString, bool> _availableBuddies;
  QMap<QString, ChatMessage> _messageQueue;
};

}

#endif


