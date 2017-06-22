/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 licence
 *
 * $Id: OptAddr.h,v 1.1 2004-10-26 22:36:57 thomson Exp $
 *
 * $Log: OptAddr.h,v $
 * Revision 1.1  2004-10-26 22:36:57  thomson
 * *** empty log message ***
 *
 * Revision 1.2  2004/09/05 15:27:49  thomson
 * Data receive switched from recvfrom to recvmsg, unicast partially supported.
 *
 *
 */

#ifndef OPTSERVERUNICAST_H
#define OPTSERVERUNICAST_H

#include "Opt.h"
#include "DHCPConst.h"
#include "SmartPtr.h"
#include "IPv6Addr.h"

class TOptAddr : public TOpt
{
 public:
    TOptAddr(int type, char * &buf,  int &n, TMsg* parent);
    TOptAddr(int type, SmartPtr<TIPv6Addr> addr, TMsg * parent);
    int getSize();
    char * storeSelf( char* buf);
    SmartPtr<TIPv6Addr> getAddr();
 protected:
    SmartPtr<TIPv6Addr> Addr;
};

#endif
