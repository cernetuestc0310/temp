/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 licence
 *
 * $Id: OptPreference.cpp,v 1.3 2004-10-25 20:45:53 thomson Exp $
 *
 * $Log: OptPreference.cpp,v $
 * Revision 1.3  2004-10-25 20:45:53  thomson
 * Option support, parsers rewritten. ClntIfaceMgr now handles options.
 *
 * Revision 1.2  2004/03/29 18:53:08  thomson
 * Author/Licence/cvs log/cvs version headers added.
 *
 *
 */
#ifdef WIN32
#include <winsock2.h>
#endif
#ifdef LINUX
#include <netinet/in.h>
#endif 
#include "DHCPConst.h"
#include "OptPreference.h"

TOptPreference::TOptPreference( char * &buf,  int &n, TMsg* parent)
	:TOpt(OPTION_PREFERENCE, parent)
{
	this->Pref=buf[0];
	buf++; n--;
}

TOptPreference::TOptPreference( char pref, TMsg* parent)
	:TOpt(OPTION_PREFERENCE, parent)
{
    Pref = pref;
}

 int TOptPreference::getSize()
{
    return 5;
}


 char * TOptPreference::storeSelf( char* buf)
{
    *(uint16_t*)buf = htons(OptType);
    buf+=2;
    *(uint16_t*)buf = htons(getSize() - 4);
    buf+=2;
    *buf = Pref;
    return buf+1;
}

int TOptPreference::getPreference(void)
{
	return this->Pref;
}
