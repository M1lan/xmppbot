#include "StateBotCommand.h"

StateBotCommand::StateBotCommand(MUCRoom *room)
{
    this->_room = room;
    this->_afkMap = new boost::unordered_map<std::string, std::pair<boost::posix_time::ptime,std::string>>();
}

bool StateBotCommand::invoke(const JID& user, const std::string& args, std::string *response) const
{
    std::string state, reason;
    bool success = this->parseArgs(args, &state, &reason);
    if(!success)
    {
        *response = "expecting 2 arguments";
        return false;
    }

    if(state != "afk" && state != "re" && state != "of")
    {
        *response = "unknown state";
        return false;
    }

    boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());
    if(state == "afk")
    {
        std::string reason_str;
        if(reason.length() > 0)
            reason_str = " ( "+reason+" )";

        (*(this->_afkMap))[user.resource()] = std::pair<boost::posix_time::ptime,std::string>(t,reason_str);

        this->_room->send(user.resource() + " is afk" + reason_str + "!");
    }
    else if(state == "re")
    {
        if(!this->_afkMap->count(user.resource()))
        {
            *response = "You are not afk...";
            return false;
        }

        boost::posix_time::time_period period((*(this->_afkMap))[user.resource()].first, t);
        std::string r = (*(this->_afkMap))[user.resource()].second;

        this->_afkMap->erase(user.resource());
        this->_room->send(user.resource() + " is back from " +r+" (" + boost::posix_time::to_simple_string(period.length()) + ")!");
    }
    else //state == "of"
    {
        std::string msg;
        if(!this->_afkMap->count(reason))
        {
            msg = "If \"" + reason + "\" exists, he/she is not afk";
        }
        else
        {
            boost::posix_time::time_period period((*(this->_afkMap))[reason].first, t);
            std::string r = (*(this->_afkMap))[reason].second;
            msg = reason + " is afk ( " + r + ", " + boost::posix_time::to_simple_string(period.length()) + " )!";
        }

        *response = msg;
        this->_room->send(msg);
    }

    return true;
}

std::string StateBotCommand::getHelp() const
{
    return std::string("<state> [<reason>] - State can be \"afk\", \"re\" or \"of\". If the state is afk you can specify an optional message for the room. With of you can query the state of a user");
}

bool StateBotCommand::showHelp() const
{
    return true;
}

bool StateBotCommand::parseArgs(const std::string& args, std::string *state, std::string *reason) const
{
    std::string cpy = boost::trim_copy(args);

    std::size_t splitpos = cpy.find(" ");

    if(std::string::npos == splitpos)
    {
        *state = cpy;
        return true;
    }

    *state = cpy.substr(0,splitpos);
    *reason = cpy.substr(splitpos + 1);

    return true;
}

void StateBotCommand::removeUserState(const JID& user)
{
    this->removeUserState(user.resource());
}

void StateBotCommand::removeUserState(const std::string& nick)
{
    if(this->_afkMap->count(nick))
        this->_afkMap->erase(nick);
}
