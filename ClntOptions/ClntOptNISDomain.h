/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntOptNISDomain.h,v 1.2 2008-08-29 00:07:29 thomson Exp $
 *
 * $Log: ClntOptNISDomain.h,v $
 * Revision 1.2  2008-08-29 00:07:29  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.1  2004-10-25 20:45:53  thomson
 * Option support, parsers rewritten. ClntIfaceMgr now handles options.
 *
 *
 */

#ifndef CLNTOPTNISDOMAIN_H
#define CLNTOPTNISDOMAIN_H

#include "OptString.h"
#include "DUID.h"
#include "SmartPtr.h"

class TClntOptNISDomain : public TOptString
{
 public:
    TClntOptNISDomain(string domains, TMsg* parent);
    TClntOptNISDomain(char *buf, int bufsize, TMsg* parent);
    bool doDuties();
    void setSrvDuid(SmartPtr<TDUID> duid);
private:
    SmartPtr<TDUID> SrvDUID;
};
#endif
