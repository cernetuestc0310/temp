/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: SrvOptDomainName.h,v 1.4 2008-08-29 00:07:36 thomson Exp $
 *
 * $Log: SrvOptDomainName.h,v $
 * Revision 1.4  2008-08-29 00:07:36  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.3  2004-10-25 20:45:54  thomson
 * Option support, parsers rewritten. ClntIfaceMgr now handles options.
 *
 */

#ifndef SRVOPTDOMAINNAME_H
#define SRVOPTDOMAINNAME_H

#include "OptStringLst.h"

class TSrvOptDomainName : public TOptStringLst
{
public:
    TSrvOptDomainName(List(string) domains, TMsg* parent);
    TSrvOptDomainName(char *buf, int bufsize, TMsg* parent);
    bool doDuties();
};
#endif
