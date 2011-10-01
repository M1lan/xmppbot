#include "XmppBot.h"

XmppBot::XmppBot()
{
    std::cout << "Initializing..." << std::endl;

    opt::options_description desc("Options");
    desc.add_options()
        ("server.user","User name")
        ("server.password","Password")
        ("server.address","The server to connect to")
        ("server.resource","The resource to connect as")
        ("muc.name","The name to use in the MUC room")
        ("muc.room","The MUC room to connect to")
        ("muc.server","The MUC server to connect to")
        ;

    std::ifstream ifs("bot.cfg");
    opt::store(opt::parse_config_file(ifs, desc), vm);
    opt::notify(vm);

    std::cout << "Loaded \"bot.cfg\". Parsing... ";

    std::string username;
    std::string password;
    std::string server;
    std::string resource;
    std::string muc_name;
    std::string muc_room;
    std::string muc_server;

    if(vm.count("server.user"))
        username = vm["server.user"].as<std::string>();
    if(vm.count("server.password"))
        password = vm["server.password"].as<std::string>();
    if(vm.count("server.address"))
        server = vm["server.address"].as<std::string>();
    if(vm.count("server.resource"))
        resource = vm["server.resource"].as<std::string>();
    if(vm.count("muc.name"))
        muc_name = vm["muc.name"].as<std::string>();
    if(vm.count("muc.room"))
        muc_room = vm["muc.room"].as<std::string>();
    if(vm.count("muc.server"))
        muc_server = vm["muc.server"].as<std::string>();

    std::cout << "Done." << std::endl;

    JID nick;
    nick.setUsername(username);
    nick.setResource(resource);
    nick.setServer(server);

    m_Client = new Client( nick, password);

    m_RosterManager = m_Client->rosterManager();
    m_RosterManager->registerRosterListener(this);

    JID muc_nick;
    muc_nick.setUsername(muc_room);
    muc_nick.setResource(muc_name);
    muc_nick.setServer(muc_server);

    m_Room = new MUCRoom(m_Client, muc_nick, this, 0);

    m_Adhoc = new Adhoc(m_Client);
    m_Adhoc->registerAdhocCommandProvider(this, "name", "friendly_name");

    m_Client->registerMessageHandler(this);
    m_Client->registerConnectionListener(this);

    this->m_CommandMgr = new BotCommandManager();
    this->m_CommandMgr->registerCommand("test", new TestBotCommand());

    m_Client->connect();
}

XmppBot::~XmppBot()
{
    if(m_Client!=0)
    {
        if(m_Client->state()==StateConnected || m_Client->state()==StateConnecting)
        {
            m_Client->disconnect();
        }
    }
    delete m_Client;
    m_Client=0;
}

void XmppBot::handleMessage( const Message& stanza, MessageSession* session)
{
    std::cout << "Message from: "<< stanza.from().username() << ": " << stanza.body() << std::endl;

    std::string jid = stanza.from().bareJID().full();
    std::string msg = stanza.body();

    bool success = false;
    this->m_CommandMgr->tryInvokeFromString(msg, stanza.from(),&success);

    if(!success)
        std::cout <<  "invoke command failed" << std::endl;

    Message m(Message::Chat, stanza.from(), "Hi, "+stanza.from().username()+" you wrote: \""+stanza.body()+"\".");
    m_Client->send(m);
}

void XmppBot::onConnect()
{
    std::cout << "Connected" << std::endl;
    m_Client->setPresence(Presence::Available, 0);

    m_Room->join();
    m_Room->getRoomItems();
}

void XmppBot::onDisconnect(ConnectionError e)
{

}

bool XmppBot::onTLSConnect(const CertInfo& info)
{
    std::cout << "Certificate accepted!" << std::endl;
    return true;
}

void XmppBot::handleItemAdded( const JID& jid )
{
    std::cout << "Added: " << jid.full() << std::endl;
}

void XmppBot::handleItemSubscribed( const JID& jid )
{
    std::cout << "Subscribed to: " << jid.full() << std::endl;
}

void XmppBot::handleItemRemoved( const JID& jid )
{
    std::cout << "Removed: " << jid.full() << std::endl;
}

void XmppBot::handleItemUpdated( const JID& jid )
{
    std::cout << "Updated: " << jid.full() << std::endl;
}

void XmppBot::handleItemUnsubscribed( const JID& jid )
{
    std::cout << "Unsubscribed from: " << jid.full() << std::endl;
}

void XmppBot::handleRoster( const Roster& roster )
{
    std::cout << "My roster:" << std::endl;
    for(Roster::const_iterator it=roster.begin(); it!=roster.end(); it++)
        std::cout << ((RosterItem*)(*it).second)->jid() << std::endl;
    std::cout << "End of roster." << std::endl;
}

void XmppBot::handleRosterPresence( const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string& msg )
{
    std::cout << "Roster presence update: " << item.jid() << ": " << msg << std::endl;
}

void XmppBot::handleSelfPresence( const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string& msg )
{

}

bool XmppBot::handleSubscriptionRequest( const JID& jid, const std::string& msg )
{
    std::cout << "Got a subscription request from: " << jid.full() << std::endl;
    m_RosterManager->ackSubscriptionRequest(jid, true);
    m_RosterManager->subscribe(jid,"SVN-Bot",StringList(),"Hi I am the SVN-Bot from shin-project.org");
    m_RosterManager->synchronize();
    return true;
}

bool XmppBot::handleUnsubscriptionRequest( const JID& jid, const std::string& msg )
{
    std::cout << "Got an unsubscribe request from: " << jid.full() << std::endl;
    m_RosterManager->unsubscribe(jid);
    m_RosterManager->synchronize();
    return true;
}

void XmppBot::handleNonrosterPresence( const Presence& presence )
{
    std::cout << " Got a non-roster presence: " << presence.from().full() << ": " << presence.status() << std::endl;
}

void XmppBot::handleRosterError( const IQ& iq )
{
    std::cerr << "Roster error: " << iq.error() << std::endl;
}

void XmppBot::handleMUCParticipantPresence( MUCRoom* room, const MUCRoomParticipant participant, const Presence& presence )
{

}

void XmppBot::handleMUCMessage( MUCRoom* room, const Message& stanza, bool priv )
{
    if(!priv)
        return;

    std::string msg = stanza.body();
    bool success = false;

    this->m_CommandMgr->tryInvokeFromString(msg, stanza.from(),&success);

    if(!success)
        std::cout <<  "invoke command failed" << std::endl;
}

bool XmppBot::handleMUCRoomCreation( MUCRoom* room )
{
    return true;
}

void XmppBot::handleMUCSubject( MUCRoom* room, const std::string& nick, const std::string& subject )
{

}

void XmppBot::handleMUCInviteDecline( MUCRoom* room, const JID& invitee, const std::string& reason )
{

}

void XmppBot::handleMUCError( MUCRoom* room, StanzaError error )
{

}

void XmppBot::handleMUCInfo( MUCRoom* room, int features, const std::string& name, const DataForm* infoForm )
{

}

void XmppBot::handleMUCItems( MUCRoom* room, const Disco::ItemList& items )
{
    std::cout << "Users in " << room->name() << ":" << std::endl;

    for(Disco::ItemList::const_iterator it = items.begin(); it!=items.end(); it++)
    {
        std::cout << ((Disco::Item*)*it)->name()<<", " << ((Disco::Item*)(*it))->node() << " (" << ((Disco::Item*)(*it))->jid().resource() << ")" << std::endl;
    }

}

void XmppBot::handleAdhocCommand( const JID& from, const Adhoc::Command& command, const std::string& sessionID )
{

}

bool XmppBot::handleAdhocAccessRequest( const JID& from, const std::string& command )
{
    // everyone can see all commands
    return true;
}
