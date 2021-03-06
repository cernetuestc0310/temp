/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntOptSIPServer.h,v 1.2 2008-08-29 00:07:29 thomson Exp $
 *
 * $Log: ClntOptSIPServer.h,v $
 * Revision 1.2  2008-08-29 00:07:29  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.1  2004-10-25 20:45:53  thomson
 * Option support, parsers rewritten. ClntIfaceMgr now handles options.
 *
 */

#ifndef CLNTOPTSIPSERVERS_H
#define CLNTOPTSIPSERVERS_H

#include "OptAddrLst.h"
#include "DUID.h"

class TClntOptSIPServers : public TOptAddrLst
{
 public:
    TClntOptSIPServers(List(TIPv6Addr) * lst, TMsg* parent);
    TClntOptSIPServers(char* buf, int size, TMsg* parent);
    bool doDuties();
    void setSrvDuid(SmartPtr<TDUID> duid);
 private:
    SmartPtr<TDUID> SrvDUID;
};
#endif
