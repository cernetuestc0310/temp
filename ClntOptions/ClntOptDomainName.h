/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntOptDomainName.h,v 1.4 2008-08-29 00:07:28 thomson Exp $
 *
 * $Log: ClntOptDomainName.h,v $
 * Revision 1.4  2008-08-29 00:07:28  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.3  2004-10-25 20:45:53  thomson
 * Option support, parsers rewritten. ClntIfaceMgr now handles options.
 *
 *
 */

#ifndef CLNTOPTDOMAINNAME_H
#define CLNTOPTDOMAINNAME_H

#include <iostream>
#include "OptStringLst.h"
#include "DUID.h"
#include "SmartPtr.h"

class TClntOptDomainName : public TOptStringLst
{
 public:
    TClntOptDomainName(List(string) * domains, TMsg* parent);
    TClntOptDomainName(char *buf, int bufsize, TMsg* parent);
    bool doDuties();
    void setSrvDuid(SmartPtr<TDUID> duid);
private:
    SmartPtr<TDUID> SrvDUID;
};
#endif
