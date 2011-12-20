#ifndef JIDMESSAGEFILTER_H
#define JIDMESSAGEFILTER_H

#include "MessageFilter.h"
#include "gloox/client.h"
#include "LogHelper.h"

class JIDMessageFilter : public MessageFilter
{
public:
    JIDMessageFilter(const JID& jid, bool cmpBareOnly, bool room, bool priv);

    void handleMessage(const Message& stanza, bool room, bool priv, bool *handled);

private:
    JID *_jid;
    bool _cmpBareOnly;
    bool _room;
    bool _priv;
};

#endif
