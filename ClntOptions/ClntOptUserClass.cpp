/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 licence
 *
 * $Id: ClntOptUserClass.cpp,v 1.3 2005-07-17 19:56:54 thomson Exp $
 *
 * $Log: ClntOptUserClass.cpp,v $
 * Revision 1.3  2005-07-17 19:56:54  thomson
 * End-of-line problem solved.
 *
 * Revision 1.2  2004/03/29 18:53:08  thomson
 * Author/Licence/cvs log/cvs version headers added.
 *
 *
 */
#include "ClntOptUserClass.h"

TClntOptUserClass::TClntOptUserClass( char * buf,  int n, TMsg* parent)
	:TOptUserClass(buf,n, parent)
{

}
bool TClntOptUserClass::doDuties()
{
    return false;
}
