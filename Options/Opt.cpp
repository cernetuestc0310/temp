/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 licence
 *
 * $Id: Opt.cpp,v 1.10 2008-08-17 22:41:43 thomson Exp $
 *
 * $Log: Opt.cpp,v $
 * Revision 1.10  2008-08-17 22:41:43  thomson
 * CONFIRM improvements (patch sent by Liu Ming)
 *
 * Revision 1.9  2008-06-01 18:29:03  thomson
 * Server now supports out of addrs scenario (bug #177)
 *
 * Revision 1.8  2007-03-28 00:39:46  thomson
 * Clnt: REQUEST now contains addresses offered in ADVERTISE (bug #152)
 *
 * Revision 1.7  2006-11-17 00:37:16  thomson
 * Partial AUTH support by Sammael, fixes by thomson
 *
 * Revision 1.6  2004-10-27 22:07:56  thomson
 * Signed/unsigned issues fixed, Lifetime option implemented, INFORMATION-REQUEST
 * message is now sent properly. Valid lifetime granted by server fixed.
 *
 * Revision 1.5  2004/05/23 16:40:12  thomson
 * *** empty log message ***
 *
 * Revision 1.4  2004/05/23 16:38:18  thomson
 * CR-LF issues.
 *
 * Revision 1.3  2004/03/29 19:11:56  thomson
 * Author/Licence/cvs log/cvs version headers added.
 *
 */

#include "Opt.h"

int TOpt::getOptType()
{
    return OptType;
}

TOpt::~TOpt() {

}

TOpt::TOpt( int optType, TMsg *parent){	
    OptType=optType;    
    Parent=parent;
}
int TOpt::getSubOptSize() {
    int size = 0;    
    SubOptions.first();    
    SmartPtr<TOpt> ptr;    
    while (ptr = SubOptions.get()) 		
	size += ptr->getSize();    
    return size;
}
char* TOpt::storeSubOpt( char* buf){
    SmartPtr<TOpt> ptr;	
    SubOptions.first();    
    while ( ptr = SubOptions.get() ) {
	ptr->storeSelf(buf);
	buf += ptr->getSize();    
    }	
    return buf;
}

void TOpt::firstOption() {
    SubOptions.first();
}

SmartPtr<TOpt> TOpt::getOption() {
    return SubOptions.get();
}

SmartPtr<TOpt> TOpt::getOption(int optType) {
    firstOption();
    SmartPtr<TOpt> opt = 0;
    while(opt=getOption()) {
	if (opt->getOptType()==optType)
	    return opt;
    }
    return 0;
}

void TOpt::addOption(SmartPtr<TOpt> opt)
{
    SubOptions.append(opt);
}

int TOpt::countOption()
{
    return SubOptions.count();
}

void TOpt::setParent(TMsg* Parent)
{
    this->Parent=Parent;
}

void TOpt::delAllOptions()
{
    SubOptions.clear();
}

bool TOpt::isValid()
{   
    //for most of options it's enough
    return true;
}

SmartPtr<TDUID> TOpt::getDUID() {
    return this->DUID;
}
void TOpt::setDUID(SmartPtr<TDUID> duid) {
    this->DUID = duid;
}
