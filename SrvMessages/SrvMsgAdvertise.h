/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: SrvMsgAdvertise.h,v 1.5 2008-08-29 00:07:34 thomson Exp $
 *
 * $Log: SrvMsgAdvertise.h,v $
 * Revision 1.5  2008-08-29 00:07:34  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.4  2005-01-08 16:52:04  thomson
 * Relay support implemented.
 *
 * Revision 1.3  2004/09/05 15:27:49  thomson
 * Data receive switched from recvfrom to recvmsg, unicast partially supported.
 *
 * Revision 1.2  2004/06/20 17:25:07  thomson
 * getName() method implemented, clean up
 *
 *
 */

#ifndef SRVMSGADVERTISE_H
#define SRVMSGADVERTISE_H

#include "SrvMsg.h"
#include "SrvMsgSolicit.h"
class TSrvMsgAdvertise : public TSrvMsg
{
  public:
    // creates object based on a buffer
    TSrvMsgAdvertise(SmartPtr<TSrvIfaceMgr> IfaceMgr,
		     SmartPtr<TSrvTransMgr> TransMgr,
		     SmartPtr<TSrvCfgMgr> CfgMgr,
		     SmartPtr<TSrvAddrMgr> AddrMgr,
		     int iface, SmartPtr<TIPv6Addr> addr);
    
    TSrvMsgAdvertise(SmartPtr<TSrvIfaceMgr> IfaceMgr,
		     SmartPtr<TSrvTransMgr> TransMgr,
		     SmartPtr<TSrvCfgMgr> CfgMgr,
		     SmartPtr<TSrvAddrMgr> AddrMgr,
		     SmartPtr<TSrvMsgSolicit> question);
    
	TSrvMsgAdvertise(SmartPtr<TSrvIfaceMgr> IfaceMgr,
		SmartPtr<TSrvTransMgr> TransMgr,
		SmartPtr<TSrvCfgMgr> CfgMgr,
		SmartPtr<TSrvAddrMgr> AddrMgr,
		unsigned int iface, SmartPtr<TIPv6Addr> addr,
		unsigned char* buf, unsigned int bufSize);

    bool check();
    bool answer(SmartPtr<TSrvMsgSolicit> solicit);
    void doDuties();
    unsigned long getTimeout();
    string getName();
    ~TSrvMsgAdvertise();
};

#endif /* SRVMSGADVERTISE_H */
