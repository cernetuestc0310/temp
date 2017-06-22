/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 licence
 *
 * $Id: OptRapidCommit.cpp,v 1.3 2005-07-17 19:56:55 thomson Exp $
 *
 * $Log: OptRapidCommit.cpp,v $
 * Revision 1.3  2005-07-17 19:56:55  thomson
 * End-of-line problem solved.
 *
 * Revision 1.2  2004/03/29 18:53:08  thomson
 * Author/Licence/cvs log/cvs version headers added.
 *
 *
 */
#include <stdlib.h>
#ifdef LINUX
#include <netinet/in.h>
#endif
#ifdef WIN32
#include <winsock2.h>
#endif

#include "OptRapidCommit.h"

TOptRapidCommit::TOptRapidCommit( char * &buf,  int &n, TMsg* parent)
	:TOpt(OPTION_RAPID_COMMIT, parent)
{
	//Here are read common things for server and client
}

TOptRapidCommit::TOptRapidCommit(TMsg* parent)
	:TOpt(OPTION_RAPID_COMMIT, parent)
{

}

//##ModelId=3ECE0A2C03D3
 int TOptRapidCommit::getSize()
{
	return 4;
}

//##ModelId=3ECE0A2D0028
 char * TOptRapidCommit::storeSelf( char* buf)
{
    *(short*)buf = htons(OptType);
    buf+=2;
    *(short*)buf = htons(getSize()-4);
    return buf+2;
}
