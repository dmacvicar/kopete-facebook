
#include <QtCore/QCoreApplication>

#include "facebook/chatservice.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    
    Facebook::ChatService *chat = new Facebook::ChatService(&app);
    chat->setLoginInformation( argv[1], argv[2] );
    chat->loginToService();
   
    return app.exec();
}
