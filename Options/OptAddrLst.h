/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: OptAddrLst.h,v 1.2 2008-08-29 00:07:30 thomson Exp $
 *
 * $Log: OptAddrLst.h,v $
 * Revision 1.2  2008-08-29 00:07:30  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.1  2004-11-02 01:23:13  thomson
 * Initial revision
 *
 *
 */

#ifndef OPTDNSSERVERS_H
#define OPTDNSSERVERS_H
#include "IPv6Addr.h"
#include "Container.h"
#include "SmartPtr.h"
#include "Opt.h"

class TOptAddrLst : public TOpt
{
public:
    TOptAddrLst(int type, List(TIPv6Addr) lst, TMsg* parent);
    TOptAddrLst(int type, char *&buf, int &bufsize, TMsg* parent);
    char * storeSelf( char* buf);
    int getSize();
    void firstAddr();
    SmartPtr<TIPv6Addr> getAddr();
    int countAddr();
    bool isValid();
protected:
    List(TIPv6Addr) AddrLst;
    bool Valid;

};
#endif
