/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntOptServerIdentifier.cpp,v 1.7 2008-08-29 00:07:29 thomson Exp $
 *
 * $Log: ClntOptServerIdentifier.cpp,v $
 * Revision 1.7  2008-08-29 00:07:29  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.6  2006-03-05 21:38:20  thomson
 * TA support merged.
 *
 * Revision 1.5.2.1  2006/02/05 23:38:07  thomson
 * Devel branch with Temporary addresses support added.
 *
 * Revision 1.5  2006/01/29 10:48:31  thomson
 * Base class changed.
 *
 * Revision 1.4  2004/10/25 20:45:53  thomson
 * Option support, parsers rewritten. ClntIfaceMgr now handles options.
 *
 * Revision 1.3  2004/03/29 19:10:06  thomson
 * Author/Licence/cvs log/cvs version headers added.
 *
 * Revision 1.2  2004/03/29 18:53:08  thomson
 * Author/Licence/cvs log/cvs version headers added.
 */

#include <stdlib.h>
#include "DHCPConst.h"
#include "ClntOptServerIdentifier.h"

TClntOptServerIdentifier::TClntOptServerIdentifier( char * duid,  int n, TMsg* parent)
	:TOptDUID(OPTION_SERVERID, duid,n, parent) {

}

TClntOptServerIdentifier::TClntOptServerIdentifier(SmartPtr<TDUID> duid, TMsg* parent)
    :TOptDUID(OPTION_SERVERID, duid, parent) {

}

bool TClntOptServerIdentifier::doDuties() {
    return false;
}
