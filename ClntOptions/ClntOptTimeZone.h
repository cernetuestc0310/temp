/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntOptTimeZone.h,v 1.4 2008-08-29 00:07:29 thomson Exp $
 *
 * $Log: ClntOptTimeZone.h,v $
 * Revision 1.4  2008-08-29 00:07:29  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.3  2004-10-25 20:45:53  thomson
 * Option support, parsers rewritten. ClntIfaceMgr now handles options.
 *
 *
 */

#ifndef CLNTOPTTIMEZONE_H
#define CLNTOPTTIMEZONE_H

#include "OptString.h"
#include "DUID.h"

class TClntOptTimeZone : public TOptString
{
 public:
    TClntOptTimeZone(string domain, TMsg* parent);
    TClntOptTimeZone(char *buf, int bufsize, TMsg* parent);
    bool doDuties();
    void setSrvDuid(SmartPtr<TDUID> duid);
    bool isValid();
 private:
    SmartPtr<TDUID> SrvDUID;
};
#endif
