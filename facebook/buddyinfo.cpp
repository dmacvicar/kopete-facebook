
#include "facebook/buddyinfo.h"

namespace Facebook
{
 
BuddyInfo::BuddyInfo()
    : _enableVC(false)
{
}

BuddyInfo::~BuddyInfo()
{
}

bool BuddyInfo::readVariant( const QVariant &variant )
{
    if ( ! variant.canConvert(QVariant::Map) )
        return false;
    
    QVariantMap info = variant.toMap();
    setFirstName(info["firstName"].toString());
    setName(info["name"].toString());
    setStatus(info["status"].toString());
    setThumbSrc(info["thumbSrc"].toString());
    return true;
}
   
QString BuddyInfo::buddyId() const
{
    return _buddyId;
}

void BuddyInfo::setBuddyId( const QString &id )
{ 
    _buddyId = id;
}

bool BuddyInfo::enableVC() const
{
    return _enableVC;   
}

void BuddyInfo::setEnableVC( bool enabled )
{
    _enableVC = enabled;
}


QString BuddyInfo::firstName() const
{
    return _firstName;
}

void BuddyInfo::setFirstName( const QString &firstName )
{
    _firstName = firstName;
}

QString BuddyInfo::name() const
{
    return _name;
}

void BuddyInfo::setName( const QString &name )
{
    _name = name;
}

    
QString BuddyInfo::status() const
{
    return _status;
}

void BuddyInfo::setStatus( const QString &status )
{
   _status = status;
}
    
QDateTime BuddyInfo::statusTime() const
{
    return _statusTime;
}

void BuddyInfo::setStatusTime( const QDateTime &statusTime )
{
    _statusTime = statusTime;
}

QString BuddyInfo::statusTimeRel() const
{
    return _statusTimeRel;
}

void BuddyInfo::setStatusTimeRel( const QString &statusTimeRel )
{
    _statusTimeRel = statusTimeRel;
}

QString BuddyInfo::thumbSrc() const
{
    return _thumbSrc;
}

void BuddyInfo::setThumbSrc( const QString &thumbSrc )
{
    _thumbSrc = thumbSrc;
}

} // ns facebook

QDebug operator<<(QDebug dbg, const Facebook::BuddyInfo &info)
{
    dbg.nospace() << info.buddyId() << " : " << info.name();
    return dbg.space();
}
