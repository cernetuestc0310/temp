/*                                                                           
 * Dibbler - a portable DHCPv6                                               
 *                                                                           
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>                           
 *          Marek Senderski <msend@o2.pl>                                    
 *                                                                           
 * released under GNU GPL v2 only licence                                
 *                                                                           
 * $Id: SrvMsgDecline.h,v 1.5 2008-08-29 00:07:34 thomson Exp $
 *
 * $Log: SrvMsgDecline.h,v $
 * Revision 1.5  2008-08-29 00:07:34  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.4  2006-08-21 21:33:20  thomson
 * prepareFQDN() moved from SrvMgrReply to SrvMsg,
 * unnecessary constructors removed.
 *
 * Revision 1.3  2005-01-08 16:52:04  thomson
 * Relay support implemented.
 *
 * Revision 1.2  2004/06/20 17:25:07  thomson
 * getName() method implemented, clean up
 *
 *                                                                           
 */

class TSrvMsgDecline;
#ifndef SRVMSGDECLINE_H
#define SRVMSGDECLINE_H
#include "SrvMsg.h"

class TSrvMsgDecline : public TSrvMsg
{
  public:
    TSrvMsgDecline(SmartPtr<TSrvIfaceMgr> IfaceMgr,
		   SmartPtr<TSrvTransMgr> TransMgr,
		   SmartPtr<TSrvCfgMgr> CfgMgr,
		   SmartPtr<TSrvAddrMgr> AddrMgr,
		   int iface, SmartPtr<TIPv6Addr> addr,
		   char* buf, int bufSize);
    
    bool  check();
    string getName();
    void  doDuties();
    unsigned long  getTimeout();
    ~TSrvMsgDecline();
};

#endif /* SRVMSGDECLINE_H */
