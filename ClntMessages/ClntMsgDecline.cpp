/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 * changes: Michal Kowalczuk <michal@kowalczuk.eu>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntMsgDecline.cpp,v 1.7 2008-11-11 22:20:42 thomson Exp $
 *
 * $Log: ClntMsgDecline.cpp,v $
 * Revision 1.7  2008-11-11 22:20:42  thomson
 * *** empty log message ***
 *
 * Revision 1.6  2008-08-29 00:07:28  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.5  2008-06-26 21:44:17  thomson
 * *** empty log message ***
 *
 * Revision 1.4  2008-02-25 17:49:07  thomson
 * Authentication added. Megapatch by Michal Kowalczuk.
 * (small changes by Tomasz Mrugalski)
 *
 * Revision 1.3  2005-01-08 16:52:03  thomson
 * Relay support implemented.
 *
 * Revision 1.2  2004/06/20 17:51:48  thomson
 * getName() method implemented, comment cleanup
 *
 *
 */

#include "ClntMsgDecline.h"
#include "ClntOptClientIdentifier.h"
#include "ClntOptServerIdentifier.h"
#include "AddrIA.h"
#include "ClntOptIA_NA.h"

TClntMsgDecline::TClntMsgDecline(SmartPtr<TClntIfaceMgr> IfaceMgr,
				 SmartPtr<TClntTransMgr> TransMgr,
				 SmartPtr<TClntCfgMgr> CfgMgr,
				 SmartPtr<TClntAddrMgr> AddrMgr,
				 int iface, SmartPtr<TIPv6Addr> addr,
				 TContainer< SmartPtr< TAddrIA> > IALst)
    :TClntMsg(IfaceMgr,TransMgr,CfgMgr,AddrMgr,iface,addr,DECLINE_MSG)
{
    IRT=DEC_TIMEOUT;
    MRT=0;
    //these both below mean there is no ending condition and transactions
    //lasts till receiving answer
    MRC=DEC_MAX_RC;
    MRD=0;
    RT=0;

    // include our ClientIdentifier
    SmartPtr<TOpt> ptr;
    ptr = new TClntOptClientIdentifier( CfgMgr->getDUID(), this );
    Options.append( ptr );
    
    // include server's DUID
    ptr = new TClntOptServerIdentifier(IALst.getFirst()->getDUID(),this);
    Options.append( ptr );
    
    // create IAs
    SmartPtr<TAddrIA> ptrIA;
    IALst.first();
    while ( ptrIA = IALst.get() ) {
	Options.append( new TClntOptIA_NA(ptrIA,this));
    }

    appendElapsedOption();
    appendAuthenticationOption(AddrMgr);

    pkt = new char[getSize()];
    this->IsDone = false;
    // this->send();
}

TClntMsgDecline::TClntMsgDecline(SmartPtr<TClntIfaceMgr> IfaceMgr,
				 SmartPtr<TClntTransMgr> TransMgr,
				 SmartPtr<TClntCfgMgr> CfgMgr,
				 SmartPtr<TClntAddrMgr> AddrMgr,
				 int iface, SmartPtr<TIPv6Addr> addr, char* buf, int bufSize)
    :TClntMsg(IfaceMgr,TransMgr,CfgMgr,AddrMgr,iface,addr,buf,bufSize)
{
	pkt=NULL;
}

void TClntMsgDecline::answer(SmartPtr<TClntMsg> rep)
{
    //FIXME: Is UseMulticast option included?

    SmartPtr<TClntOptServerIdentifier> repSrvID= (Ptr*)  rep->getOption(OPTION_SERVERID);
    SmartPtr<TClntOptServerIdentifier> msgSrvID= (Ptr*)  this->getOption(OPTION_SERVERID);
    if ((!repSrvID)||
        (!(*msgSrvID->getDUID()==*repSrvID->getDUID())))
       return;
    IsDone = true;
}

void TClntMsgDecline::doDuties()
{
    if (RC!=MRC)
       	send();
    else
	IsDone=true;
    return;
}

bool TClntMsgDecline::check()
{
	return false;
}

string TClntMsgDecline::getName() {
    return "DECLINE";
}


TClntMsgDecline::~TClntMsgDecline()
{
}
