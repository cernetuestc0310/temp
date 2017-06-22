/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: RelMsgGeneric.h,v 1.4 2008-08-29 00:07:32 thomson Exp $
 *
 * $Log: RelMsgGeneric.h,v $
 * Revision 1.4  2008-08-29 00:07:32  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.3  2005-04-28 21:20:52  thomson
 * Support for multiple relays added.
 *
 * Revision 1.2  2005/04/25 00:19:20  thomson
 * Changes in progress.
 *
 * Revision 1.1  2005/01/12 00:00:38  thomson
 * *** empty log message ***
 *
 * Revision 1.1  2005/01/11 22:53:35  thomson
 * Relay skeleton implemented.
 *
 *
 */

#ifndef RELMSGGENERIC_H
#define RELMSGGENERIC_H

#include "RelMsg.h"
#include "RelCommon.h"

using namespace std;

class TRelMsgGeneric: public TRelMsg {

 public:
    TRelMsgGeneric(TCtx * ctx, int iface, SmartPtr<TIPv6Addr> addr, char * data, int dataLen);
    string getName();
    bool check();
    int getSize();
};

#endif
