#include <stdlib.h>
#include "facebook/chatmessage.h"

namespace Facebook {

ChatMessage::ChatMessage( const QString &messageId )
    : _id(messageId)
{
}
 
ChatMessage::ChatMessage()
{
    _id = QString::number(qrand());
}    
   
ChatMessage::~ChatMessage()
{
}

QString ChatMessage::messageId() const
{
    return _id;
}
    
bool ChatMessage::readVariant( const QVariant &variant )
{
    // { "type":"msg",
    //   "msg":{"text":"i'm sending u a message",
    //   "time":1209548203464, "clientTime":1209548202312,
    //   "msgID":"3409501070"},"from":800753867,"to":596176850,"from_name":"Eion Robb","to_name":"Jeremy Lawson","from_first_name":"Eion","to_first_name":"Jeremy"}

    if ( ! variant.canConvert(QVariant::Map) )
        return false;    
     
    QVariantMap message = variant.toMap();
    QVariantMap data = message["msg"].toMap();
    
    // may be we should also check that type is msg
    _id = data["msgID"].toString();

    if (_id.isEmpty())
        return false;

    setText(data["text"].toString());
    setFrom(message["from"].toString());
    setTo(message["to"].toString());
    setFromName(message["from_name"].toString());
    setToName(message["to_name"].toString());
    setFromFirstName(message["from_first_name"].toString());
    setToFirstName(message["to_first_name"].toString());
    setToFirstName(message["to_first_name"].toString());
    setTime(QDateTime::fromTime_t(data["time"].toInt()));
    setClientTime(QDateTime::fromTime_t(data["client_time"].toInt()));
    return true;
}

QString ChatMessage::text() const
{                                
    return _text;                
}                                

void ChatMessage::setText( const QString &text )
{                                               
    _text = text;                               
}                                               


QDateTime ChatMessage::time() const
{                                  
    return _time;                  
}                                  

void ChatMessage::setTime( const QDateTime &time )
{                                                 
    _time = time;                                 
}                                                 


QDateTime ChatMessage::clientTime() const
{                                        
    return _clientTime;                  
}                                        

void ChatMessage::setClientTime( const QDateTime &clientTime )
{                                                             
    _clientTime = clientTime;                                 
}                                                             


QString ChatMessage::from() const
{                                
    return _from;                
}                                

void ChatMessage::setFrom( const QString &from )
{                                               
    _from = from;
}


QString ChatMessage::to() const
{
    return _to;
}

void ChatMessage::setTo( const QString &to )
{
    _to = to;
}


QString ChatMessage::toName() const
{
    return _toName;
}

void ChatMessage::setToName( const QString &toName )
{
    _toName = toName;
}


QString ChatMessage::toFirstName() const
{
    return _toFirstName;
}

void ChatMessage::setToFirstName( const QString &toFirstName )
{
    _toFirstName = toFirstName;
}


QString ChatMessage::fromName() const
{
    return _fromName;
}

void ChatMessage::setFromName( const QString &fromName )
{
    _fromName = fromName;
}


QString ChatMessage::fromFirstName() const
{
    return _fromFirstName;
}

void ChatMessage::setFromFirstName( const QString &fromFirstName )
{
    _fromFirstName = fromFirstName;
}

} //ns

QDataStream & operator<< ( QDataStream & stream, const Facebook::ChatMessage &message )
{
    stream << "msg: " << message.messageId() << " | " << message.from() << ":" << message.to() << " | " << message.fromName() << " -> " << message.toName();
    
    return stream;
}

QDebug operator<<(QDebug dbg, const Facebook::ChatMessage &message)
{
    dbg.nospace() << "msg: " << message.messageId() << " | " << message.from() << ":" << message.to() << " | " << message.fromName() << " -> " << message.toName();
    ;
    return dbg.space();
}
