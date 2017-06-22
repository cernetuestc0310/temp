/*
 * Dibbler - a portable DHCPv6
 *
 * author: Michal Kowalczuk <michal@kowalczuk.eu>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntOptKeyGeneration.cpp,v 1.2 2008-08-29 00:07:29 thomson Exp $
 *
 * $Log: ClntOptKeyGeneration.cpp,v $
 * Revision 1.2  2008-08-29 00:07:29  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.1  2008-02-25 20:42:45  thomson
 * Missing new AUTH files added.
 *
 *
 */
#include "ClntOptKeyGeneration.h"
#include "ClntMsg.h"
#include "Logger.h"

TClntOptKeyGeneration::TClntOptKeyGeneration( char * buf,  int n, TClntMsg* parent)
	:TOptKeyGeneration(buf, n, parent)
{
    parent->getClntCfgMgr()->setDigest((DigestTypes)this->getAlgorithmId());
}

bool TClntOptKeyGeneration::doDuties()
{
    int ifindex = this->Parent->getIface();
    TClntMsg * msg = (TClntMsg*)(this->Parent);
    SmartPtr<TClntCfgMgr> cfgMgr = msg->getClntCfgMgr();
    SmartPtr<TClntCfgIface> cfgIface = cfgMgr->getIface(ifindex);
    cfgIface->setKeyGenerationState(STATE_CONFIGURED);

    return true;
}
