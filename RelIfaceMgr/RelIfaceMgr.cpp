/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: RelIfaceMgr.cpp,v 1.18 2008-08-29 00:07:32 thomson Exp $
 *
 */

#include "RelCommon.h"
#include "Logger.h"
#include "IPv6Addr.h"
#include "Iface.h"
#include "SocketIPv6.h"
#include "RelIfaceMgr.h"
#include "RelCfgMgr.h"
#include "RelMsgGeneric.h"
#include "RelMsgRelayForw.h"
#include "RelMsgRelayRepl.h"
#include "RelOptInterfaceID.h"

/*
 * constructor. Do nothing particular, just invoke IfaceMgr constructor
 */
TRelIfaceMgr::TRelIfaceMgr(string xmlFile) 
    : TIfaceMgr(xmlFile, true) {
}

TRelIfaceMgr::~TRelIfaceMgr() {
    Log(Debug) << "RelIfaceMgr cleanup." << LogEnd;
}

void TRelIfaceMgr::dump()
{
    std::ofstream xmlDump;
    xmlDump.open( this->XmlFile.c_str() );
    xmlDump << *this;
    xmlDump.close();
}

/**
 * sends data to client. Uses multicast address as source
 * @param iface - interface ID
 * @param msg - buffer containing message ready to send
 * @param size - size of message
 * @param addr - destination address
 * returns true if message was send successfully
 */
bool TRelIfaceMgr::send(int ifindex, char *data, int dataLen, 
			SmartPtr<TIPv6Addr> addr, int port) {
    // find this interface
    SmartPtr<TIfaceIface> iface = this->getIfaceByID(ifindex);
    if (!iface) {
	Log(Error)  << "Send failed: No such interface id=" << ifindex << LogEnd;
	return false;
    }

    // find this socket
    SmartPtr<TIfaceSocket> ptrSocket;
    iface->firstSocket();
    ptrSocket = iface->getSocket();
    if (!ptrSocket) {
	Log(Error) << "Send failed: interface " << iface->getName() 
		   << "/" << iface->getID() << " has no open sockets." << LogEnd;
	return false;
    }

    // send it!
    if (ptrSocket->send(data, dataLen, addr, port) < 0)
	return false;
    return true;
}

/**
 * reads messages from all interfaces
 * it's wrapper around IfaceMgr::select(...) method
 * @param timeout - how long can we wait for packets?
 * returns SmartPtr to message object
 */
SmartPtr<TRelMsg> TRelIfaceMgr::select(unsigned long timeout) {
    
    // static buffer speeds things up
    static char data[2048];
    int dataLen=2048;

    SmartPtr<TIPv6Addr> peer (new TIPv6Addr());
    int sockid;
    int msgtype;

    // read data
    sockid = TIfaceMgr::select(timeout, data, dataLen, peer);
    if (sockid>0) {
	if (dataLen<4) {
	    Log(Warning) << "Received message is truncated (" << dataLen << " bytes)." << LogEnd;
	    return 0; //NULL
	}
	
	// check message type
	msgtype = data[0];
	SmartPtr<TMsg> ptr;
	SmartPtr<TIfaceIface> iface;
	SmartPtr<TIfaceSocket> sock;

	// get interface
	iface = this->getIfaceBySocket(sockid);

	sock = iface->getSocketByFD(sockid);

	Log(Debug) << "Received " << dataLen << " bytes on the " << iface->getName() << "/" 
		   << iface->getID() << " interface (socket=" << sockid << ", addr=" << peer->getPlain() 
		   << ", port=" << sock->getPort() << ")." << LogEnd;
	
	if (sock->getPort()!=DHCPSERVER_PORT) {
	    Log(Error) << "Message was received on invalid (" << sock->getPort() << ") port." << LogEnd;
	    return 0;
	}

	return this->decodeMsg(iface, peer, data, dataLen);
    } 
    return 0;
}

SmartPtr<TRelMsg> TRelIfaceMgr::decodeRelayForw(SmartPtr<TIfaceIface> iface, 
						SmartPtr<TIPv6Addr> peer, 
						char * data, int dataLen) {
    int ifindex = iface->getID();
    SmartPtr<TRelMsg> msg = new TRelMsgRelayForw(this->Ctx, ifindex, peer, data, dataLen);
    return msg;
}

SmartPtr<TRelMsg> TRelIfaceMgr::decodeGeneric(SmartPtr<TIfaceIface> iface, 
					      SmartPtr<TIPv6Addr> peer, 
					      char * buf, int bufsize) {
    int ifindex = iface->getID();
    return new TRelMsgGeneric(this->Ctx, ifindex, peer, buf, bufsize);
}

SmartPtr<TRelMsg> TRelIfaceMgr::decodeRelayRepl(SmartPtr<TIfaceIface> iface, 
						SmartPtr<TIPv6Addr> peer, 
						char * buf, int bufsize) {
    SmartPtr<TIPv6Addr> linkAddrTbl[HOP_COUNT_LIMIT];
    SmartPtr<TIPv6Addr> peerAddrTbl[HOP_COUNT_LIMIT];
    SmartPtr<TRelOptInterfaceID> interfaceIDTbl[HOP_COUNT_LIMIT];
    int hopTbl[HOP_COUNT_LIMIT];
    SmartPtr<TRelOptInterfaceID> ptrIfaceID;
    char * relayBuf=0;
    int relayLen = 0;
    int relays=0;

    /* decode RELAY_FORW message */
    if (bufsize < 34) {
	Log(Warning) << "Truncated RELAY_REPL message received." << LogEnd;
	return 0;
    }

    // char type = buf[0]; // ignore it
    int hopCount = buf[1]; // this one is not currently needed either
    SmartPtr<TIPv6Addr> linkAddr = new TIPv6Addr(buf+2,false);
    SmartPtr<TIPv6Addr> peerAddr = new TIPv6Addr(buf+18, false);
    buf+=34;
    bufsize-=34;
    
    // options: only INTERFACEID and RELAY_MSG are allowed
    while (bufsize>=4) {

	unsigned short code = ntohs( *((unsigned short*) (buf)));
	unsigned short len  = ntohs( *((unsigned short*) (buf+2)));
	if (len>bufsize) {
	    Log(Error) << "Message RELAY-REPL truncated. There are " << (bufsize-len) 
		       << " bytes left to parse. Message dropped." << LogEnd;
	    return 0;
	}
	buf     += 4;
	bufsize -= 4;
	switch (code) {
	case OPTION_INTERFACE_ID:
	    if (bufsize<4) {
		Log(Warning) << "Truncated INTERFACE_ID option in RELAY_REPL message. Message dropped." << LogEnd;
		return 0;
	    }
	    if (len!=4) {
		Log(Warning) << "Invalid INTERFACE_ID option, expected length " << 4
			     << ", actual length " << len << "." << LogEnd;
		return 0;
	    }

	    ptrIfaceID = new TRelOptInterfaceID(buf, bufsize, 0);
	    buf     += ptrIfaceID->getSize()-4;
	    bufsize -= ptrIfaceID->getSize()-4;
	    break;
	case OPTION_RELAY_MSG:
	    relayBuf = buf;
	    relayLen = len;
	    buf     += relayLen;
	    bufsize -= relayLen;
	    break;
	default:
	    SPtr<TRelOptEcho> echo = Ctx->CfgMgr->getEcho();
	    if (echo && echo->isOption(code)) {
		Log(Notice) << "Received echoed back option " << code << "." << LogEnd;
	    } else {
		Log(Warning) << "Invalid option " << code << " in RELAY_REPL message. Option ignored." << LogEnd;
	    }
	    buf     += len;
	    bufsize -= len;
	    continue;
	}
    }
    
    Log(Info) << "RELAY_REPL was decapsulated: link=" << linkAddr->getPlain() << ", peer=" << peerAddr->getPlain();
    if (ptrIfaceID)
	Log(Cont) << ", interfaceID=" << ptrIfaceID->getValue();
    Log(Cont) << LogEnd;

    if (!ptrIfaceID) {
	if (!Ctx->CfgMgr->guessMode()) {
	    /* guessMode disabled */
	    Log(Warning) << "InterfaceID option is missing, guessMode disabled, unable to forward. Packet dropped." << LogEnd;
	    return 0;
	}

	/* guess mode enabled, let's find any interface */
	SmartPtr<TRelCfgIface> tmp;
	Ctx->CfgMgr->firstIface();
	while (tmp = Ctx->CfgMgr->getIface()) {
	    if (tmp->getID() == iface->getID()) 
		continue; // skip the interface we've received the data on
	    break;
	}
	if (!tmp) {
	    Log(Error) << "Guess-mode failed. Unable to find any interface the message can be relayed on. Please send interface-id option in server replies." << LogEnd;
	    return 0;
	}
	Log(Notice) << "Guess-mode successful. Using " << tmp->getFullName() << " as destination interface." << LogEnd;
	ptrIfaceID = new TRelOptInterfaceID(tmp->getInterfaceID(), 0);
    }

    linkAddrTbl[relays] = linkAddr;
    peerAddrTbl[relays] = peerAddr;
    interfaceIDTbl[relays] = ptrIfaceID;
    hopTbl[relays] = hopCount;
    relays++;

    SmartPtr<TRelMsg> msg;
    switch (relayBuf[0]) {
    case RELAY_REPL_MSG:
	//msg = this->decodeRelayRepl(iface, peer, buf, bufsize);
	msg = new TRelMsgRelayRepl(this->Ctx, iface->getID(), peer, relayBuf, relayLen);
	break;
    case RELAY_FORW_MSG:
	Log(Error) << "RELAY_REPL contains RELAY_FORW message." << LogEnd;
	return 0;
    default:
	msg = this->decodeGeneric(iface, peer, relayBuf, relayLen);
    };
    // inform that this message should be sent to the peerAddr address on the ptrIface interface.
    msg->setDestination(ptrIfaceID->getValue(), peerAddr);
    return (Ptr*)msg;
}

SmartPtr<TRelMsg> TRelIfaceMgr::decodeMsg(SmartPtr<TIfaceIface> iface, 
					  SmartPtr<TIPv6Addr> peer, 
					  char * data, int dataLen) {
    if (dataLen <= 0) 
	return 0;
    
    // create specific message object
    switch (data[0]) {
    case SOLICIT_MSG:
    case REQUEST_MSG:
    case CONFIRM_MSG:
    case RENEW_MSG:
    case REBIND_MSG:
    case RELEASE_MSG:
    case DECLINE_MSG:
    case INFORMATION_REQUEST_MSG:
    case ADVERTISE_MSG:
    case REPLY_MSG:
	return this->decodeGeneric(iface, peer, data, dataLen);
    case RELAY_FORW_MSG: 
	return this->decodeRelayForw(iface, peer, data, dataLen);
    case RELAY_REPL_MSG:
	return this->decodeRelayRepl(iface, peer, data, dataLen);
    case RECONFIGURE_MSG:
    default:
	Log(Warning) << "Message type " << (int)(data[0]) << " is not supported." << LogEnd;
	return 0;
    }
}

void TRelIfaceMgr::setContext(TCtx *ctx)
{
    this->Ctx = ctx;
}

ostream & operator <<(ostream & strum, TRelIfaceMgr &x) {
    strum << "<RelIfaceMgr>" << std::endl;
    SmartPtr<TIfaceIface> ptr;
    x.IfaceLst.first();
    while ( ptr= (Ptr*) x.IfaceLst.get() ) {
	strum << *ptr;
    }
    strum << "</RelIfaceMgr>" << std::endl;
    return strum;
}
