/*
 * Dibbler - a portable DHCPv6
 *
 * author: Michal Kowalczuk <michal@kowalczuk.eu>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntOptAAAAuthentication.cpp,v 1.2 2008-08-29 00:07:28 thomson Exp $
 *
 * $Log: ClntOptAAAAuthentication.cpp,v $
 * Revision 1.2  2008-08-29 00:07:28  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.1  2008-02-25 20:42:45  thomson
 * Missing new AUTH files added.
 *
 *
 */
#include "ClntOptAAAAuthentication.h"

TClntOptAAAAuthentication::TClntOptAAAAuthentication( char * buf,  int n, TMsg* parent)
	:TOptAAAAuthentication(buf, n, parent)
{
}

TClntOptAAAAuthentication::TClntOptAAAAuthentication(TClntMsg* parent)
    :TOptAAAAuthentication(parent)
{
    this->setAAASPI(parent->getClntCfgMgr()->getAAASPI());
}

bool TClntOptAAAAuthentication::doDuties()
{
    return false;
}
