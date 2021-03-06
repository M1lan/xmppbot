#ifndef XMPPBOT_H
#define XMPPBOT_H

#include <list>

#include "JIDMap.h"

#include "gloox/client.h"
#include "gloox/message.h"
#include "gloox/presence.h"
#include "gloox/rosteritem.h"
#include "gloox/messagehandler.h"
#include "gloox/connectionlistener.h"
#include "gloox/rosterlistener.h"
#include "gloox/rostermanager.h"
#include "gloox/mucroom.h"
#include "gloox/mucroomhandler.h"
#include "gloox/mucroomconfighandler.h"

#include "BotCommandManager.h"
#include "StateBotCommand.h"

#include "LogHelper.h"
#include "StringFormat.h"

#include "MessageFilter.h"

#include "Configuration.h"

using namespace gloox;

class XmppBot: public MessageHandler, ConnectionListener, RosterListener, MUCRoomHandler, MUCRoomConfigHandler
{
public:
	enum ExitState
	{
		RELOAD_REQUESTED = -1,
		QUIT = 0,

		//define error states
		CONFIG_NOT_FOUND,
		UNABLE_TO_CONNECT

	};

    XmppBot(Configuration *config);
    virtual ~XmppBot();

    ExitState run();
    void quit(ExitState state = XmppBot::QUIT);
protected:
    // MessageListener
    void handleMessage(const Message& stanza, MessageSession* session = 0);

    // ConnectionListener
    void onConnect();
    void onDisconnect(ConnectionError e);
    bool onTLSConnect(const CertInfo &info);

    // RosterListener
    void handleItemAdded( const JID& jid );
    void handleItemSubscribed( const JID& jid );
    void handleItemRemoved( const JID& jid );
    void handleItemUpdated( const JID& jid );
    void handleItemUnsubscribed( const JID& jid );
    void handleRoster( const Roster& roster );
    void handleRosterPresence( const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string& msg );
    void handleSelfPresence( const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string& msg );
    bool handleSubscriptionRequest( const JID& jid, const std::string& msg );
    bool handleUnsubscriptionRequest( const JID& jid, const std::string& msg );
    void handleNonrosterPresence( const Presence& presence );
    void handleRosterError( const IQ& iq );

    // MUCRoomListener
    void handleMUCParticipantPresence( MUCRoom* room, const MUCRoomParticipant participant, const Presence& presence );
    void handleMUCMessage( MUCRoom* room, const Message& msg, bool priv );
    bool handleMUCRoomCreation( MUCRoom* room );
    void handleMUCSubject( MUCRoom* room, const std::string& nick, const std::string& subject );
    void handleMUCInviteDecline( MUCRoom* room, const JID& invitee, const std::string& reason );
    void handleMUCError( MUCRoom* room, StanzaError error );
    void handleMUCInfo( MUCRoom* room, int features, const std::string& name, const DataForm* infoForm );
    void handleMUCItems( MUCRoom* room, const Disco::ItemList& items );

    // MUCRoomConfigHandler
    void handleMUCConfigList( MUCRoom* room, const MUCListItemList& items, MUCOperation operation );
    void handleMUCConfigForm( MUCRoom* room, const DataForm& form );
    void handleMUCConfigResult( MUCRoom* room, bool success, MUCOperation operation );
    void handleMUCRequest( MUCRoom* room, const DataForm& form );

private:
    void init();
    void initXmpp();
    void initCommands();
    void initMessageFilter();

    void handleMessage(const Message& stanza, bool room, bool priv);

    ExitState m_ExitState;

    Client* m_Client;
    RosterManager* m_RosterManager;
    MUCRoom* m_Room;
    BotCommandManager* m_CommandMgr;
    Configuration* m_Config;
    JIDMap* m_UserNicknameMap;

    StateBotCommand* m_StateCommand;

    bool m_bePolite;
    std::string msg_join;
    std::string msg_leave;
    std::string msg_subscribe;

    std::list<MessageFilter*>* m_MessageFilter;
};

#endif // XMPPBOT_H
