/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 licence
 *
 * $Id: DHCPRelay.cpp,v 1.5 2007-03-10 01:42:32 thomson Exp $
 *
 * $Log: DHCPRelay.cpp,v $
 * Revision 1.5  2007-03-10 01:42:32  thomson
 * Guess-mode (and related segfault fix) added to relay.
 *
 * Revision 1.4  2007-02-03 17:50:43  thomson
 * Win32 fixes.
 *
 * Revision 1.3  2005/02/01 00:57:36  thomson
 * no message
 *
 * Revision 1.2  2005/01/13 22:45:55  thomson
 * Relays implemented.
 *
 * Revision 1.1  2005/01/11 22:53:35  thomson
 * Relay skeleton implemented.
 *
 */
      
#include "DHCPRelay.h"
#include "RelCommon.h"
#include "Logger.h"
#include "Portable.h"
#include "RelIfaceMgr.h"
#include "RelCfgMgr.h"
#include "RelTransMgr.h"
#include "RelMsg.h"

volatile int serviceShutdown;

TDHCPRelay::TDHCPRelay(string config)
{
    memset(&this->Ctx, 0, sizeof(TCtx));

    serviceShutdown = 0;
    srand(now());
    this->IsDone = false;

    this->Ctx.IfaceMgr = new TRelIfaceMgr(RELIFACEMGR_FILE);
    if ( this->Ctx.IfaceMgr->isDone() ) {
	Log(Crit) << "Fatal error during IfaceMgr initialization." << LogEnd;
	this->IsDone = true;
	return;
    }
    this->Ctx.IfaceMgr->setContext(&this->Ctx);
    this->Ctx.IfaceMgr->dump();

    this->Ctx.CfgMgr = new TRelCfgMgr(this->Ctx, config, RELCFGMGR_FILE);
    if ( this->Ctx.CfgMgr->isDone() ) {
	Log(Crit) << "Fatal error during CfgMgr initialization." << LogEnd;
	this->IsDone = true;
	return;
    }
    this->Ctx.CfgMgr->dump();

    this->Ctx.TransMgr = new TRelTransMgr(&this->Ctx, RELTRANSMGR_FILE);
    if ( this->Ctx.TransMgr->isDone() ) {
	Log(Crit) << "Fatal error during TransMgr initialization." << LogEnd;
	this->IsDone = true;
	return;
    }
    this->Ctx.IfaceMgr->dump();
    this->Ctx.TransMgr->dump();
}

void TDHCPRelay::run()
{
    bool silent = false;
    while ( (!this->isDone()) && (!this->Ctx.TransMgr->isDone()) ) {
    	if (serviceShutdown)
	    this->Ctx.TransMgr->shutdown();
	
	this->Ctx.TransMgr->doDuties();
	unsigned int timeout = DHCPV6_INFINITY/2;
	if (serviceShutdown)     timeout = 0;
	
	if (!silent)
	    Log(Debug) << "Accepting messages." << LogEnd;

#ifdef WIN32
	// There's no easy way to break select, so just don't sleep for too long.
	if (timeout>5) {
	    silent = true;
	    timeout = 5;
	}
#endif
	
	SmartPtr<TRelMsg> msg = this->Ctx.IfaceMgr->select(timeout);
	if (!msg) 
	    continue;
	silent = false;
	int iface = msg->getIface();
	SmartPtr<TIfaceIface> ptrIface;
	ptrIface = this->Ctx.IfaceMgr->getIfaceByID(iface);
	Log(Notice) << "Received " << msg->getName() << " on " << ptrIface->getName() 
		    << "/" << iface;
	if (msg->getType()!=RELAY_FORW_MSG && msg->getType()!=RELAY_REPL_MSG)
	    Log(Cont) << hex << ",TransID=0x" << msg->getTransID() << dec;
	Log(Cont) << ", " << msg->countOption() << " opts:";
	SmartPtr<TOpt> ptrOpt;
	msg->firstOption();
	while ( ptrOpt = msg->getOption() ) {
	    Log(Cont) << " " << ptrOpt->getOptType(); 
            // uncomment this to get detailed info about option lengths Log(Cont) << "/" << ptrOpt->getSize();
	}
	// Log(Cont) << ", " << msg->getRelayCount() << " relay(s).";
	Log(Cont) << LogEnd;
	this->Ctx.TransMgr->relayMsg(msg);
    }
    Log(Notice) << "Bye bye." << LogEnd;
}

bool TDHCPRelay::isDone() {
    return this->IsDone;
}

bool TDHCPRelay::checkPrivileges() {
    // FIXME: check privileges
    return true;
}

void TDHCPRelay::stop() {
    serviceShutdown = 1;
	Log(Crit) << "Service SHUTDOWN." << LogEnd;

}

void TDHCPRelay::setWorkdir(std::string workdir) {
    if (this->Ctx.CfgMgr) {
        this->Ctx.CfgMgr->setWorkdir(workdir);
        this->Ctx.CfgMgr->dump();
    }
}

TDHCPRelay::~TDHCPRelay() {
}

