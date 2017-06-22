/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: RelParsGlobalOpt.cpp,v 1.5 2008-08-29 00:07:32 thomson Exp $
 *
 * $Log: RelParsGlobalOpt.cpp,v $
 * Revision 1.5  2008-08-29 00:07:32  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.4  2008-03-02 19:35:26  thomson
 * RemoteID, EchoRequest options implemented.
 *
 * Revision 1.3  2007-05-01 12:03:13  thomson
 * Support for interface-id location added.
 *
 * Revision 1.2  2007-03-07 02:37:11  thomson
 * Experimental support for guess-mode.
 *
 * Revision 1.1  2005-01-11 22:53:35  thomson
 * Relay skeleton implemented.
 *
 */

#include "RelParsGlobalOpt.h"
#include "Portable.h"

TRelParsGlobalOpt::TRelParsGlobalOpt(void) {
    this->WorkDir   = WORKDIR;
    this->GuessMode = false;
    this->InterfaceIDOrder = REL_IFACE_ID_ORDER_BEFORE;
}

TRelParsGlobalOpt::~TRelParsGlobalOpt(void) {
}

string TRelParsGlobalOpt::getWorkDir() {
    return this->WorkDir;
}

void TRelParsGlobalOpt::setWorkDir(string dir) {
    this->WorkDir=dir;
}


void TRelParsGlobalOpt::setGuessMode(bool guess) {
    GuessMode = guess;
}

bool TRelParsGlobalOpt::getGuessMode() {
    return GuessMode;
}

void TRelParsGlobalOpt::setInterfaceIDOrder(ERelIfaceIdOrder order) {
    InterfaceIDOrder = order;
}

ERelIfaceIdOrder TRelParsGlobalOpt::getInterfaceIDOrder() {
    return InterfaceIDOrder;
}

void TRelParsGlobalOpt::setRemoteID(SPtr<TRelOptRemoteID> remoteID)
{
    RemoteID = remoteID;
}

SPtr<TRelOptRemoteID> TRelParsGlobalOpt::getRemoteID()
{
    return RemoteID;
}

void TRelParsGlobalOpt::setEcho(SPtr<TRelOptEcho> echo)
{
    Echo = echo;
}

SPtr<TRelOptEcho> TRelParsGlobalOpt::getEcho()
{
    return Echo;
}
