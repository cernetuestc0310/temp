/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: RelMsg.h,v 1.6 2008-08-29 00:07:32 thomson Exp $
 *
 * $Log: RelMsg.h,v $
 * Revision 1.6  2008-08-29 00:07:32  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.5  2005-04-28 21:20:52  thomson
 * Support for multiple relays added.
 *
 * Revision 1.4  2005/04/25 00:19:20  thomson
 * Changes in progress.
 *
 * Revision 1.3  2005/01/13 22:45:55  thomson
 * Relays implemented.
 *
 * Revision 1.2  2005/01/11 23:35:22  thomson
 * *** empty log message ***
 *
 * Revision 1.1  2005/01/11 22:53:35  thomson
 * Relay skeleton implemented.
 *
 */

class TRelMsg;
#ifndef RELMSG_H
#define RELMSG_H

#include "RelCommon.h"
#include "Msg.h"
#include "Iface.h"

class TRelMsg : public TMsg
{
public:
    TRelMsg(TCtx * ctx, int iface,  SmartPtr<TIPv6Addr> addr, char* data,  int dataLen);
    virtual bool check() = 0;
    virtual int getSize() = 0;
    void setDestination(int ifindex, SmartPtr<TIPv6Addr> dest);
    int getDestIface();
    SmartPtr<TIPv6Addr> getDestAddr();
    void decodeOpts(char * data, int dataLen);
    int getHopCount();
    
 protected:
    TCtx * Ctx;
    int DestIface;
    SmartPtr<TIPv6Addr>   DestAddr;

    int HopCount; // mormal messages =0, RELAY_FORW, RELAY_REPL = (0..32)
};

#endif
