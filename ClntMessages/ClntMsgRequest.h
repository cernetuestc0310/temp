/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 * changes: Krzysztof Wnuk <keczi@poczta.onet.pl>
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntMsgRequest.h,v 1.8 2008-08-29 00:07:28 thomson Exp $
 *
 * $Log: ClntMsgRequest.h,v $
 * Revision 1.8  2008-08-29 00:07:28  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.7  2007-03-28 00:39:46  thomson
 * Clnt: REQUEST now contains addresses offered in ADVERTISE (bug #152)
 *
 * Revision 1.6  2007-03-28 00:13:53  thomson
 * No longer gets confused if there is no reply to REQUEST message (bug #147)
 *
 * Revision 1.5  2007-01-27 17:12:24  thomson
 * Huge cleanup, prefix delegation support improved.
 *
 * Revision 1.4  2006-10-06 00:43:28  thomson
 * Initial PD support.
 *
 * Revision 1.3  2005-01-08 16:52:03  thomson
 * Relay support implemented.
 *
 * Revision 1.2  2004/06/20 17:51:48  thomson
 * getName() method implemented, comment cleanup
 *
 *
 */

class TClntIfaceMgr;
#ifndef CLNTMSGREQUEST_H
#define CLNTMSGREQUEST_H

#include "SmartPtr.h"
#include "ClntMsg.h"
#include "ClntAddrMgr.h"
#include "ClntCfgMgr.h"
#include "ClntIfaceMgr.h"
#include "ClntCfgMgr.h"

class TClntMsgRequest : public TClntMsg
{
  public:
    TClntMsgRequest(SmartPtr<TClntIfaceMgr> IfMgr, 
		    SmartPtr<TClntTransMgr> TransMgr,
		    SmartPtr<TClntCfgMgr>   ConfMgr, 
		    SmartPtr<TClntAddrMgr>  AddrMgr, 
		    TContainer< SmartPtr<TOpt> > opts, 
		    int iface);
    TClntMsgRequest(SmartPtr<TClntIfaceMgr> IfaceMgr, 
				 SmartPtr<TClntTransMgr> TransMgr,
				 SmartPtr<TClntCfgMgr>   CfgMgr, 
				 SmartPtr<TClntAddrMgr> AddrMgr, 
                 TContainer<SmartPtr<TAddrIA> > requestIALst,
                 SmartPtr<TDUID> srvDUID,
				 int iface);

    void answer(SmartPtr<TClntMsg> msg);
    void doDuties();
    bool check();
    string getName();
    ~TClntMsgRequest();
  private:
    void setState(List(TOpt) opts, EState state);
    void copyAddrsFromAdvertise(SPtr<TClntMsg> adv);
    SmartPtr<TClntAddrMgr> AddrMgr;
};

#endif /* CLNTMSGREQUEST_H */
