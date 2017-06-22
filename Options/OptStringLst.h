/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 licence
 *
 * $Id: OptStringLst.h,v 1.1 2004-11-02 01:23:13 thomson Exp $
 *
 * $Log: OptStringLst.h,v $
 * Revision 1.1  2004-11-02 01:23:13  thomson
 * Initial revision
 *
 */

#ifndef OPTSTRINGLST_H
#define OPTSTRINGLST_H
#include "Opt.h"
#include <iostream>
#include <string>
using namespace std;

class TOptStringLst : public TOpt
{
public:
    TOptStringLst(int type, List(string) strLst, TMsg* parent);
    TOptStringLst(int type, char *&buf, int &bufsize, TMsg* parent);
    char * storeSelf( char* buf);
    int getSize();
    void firstString();
    string getString();
    int countString();

protected:
    List(string) StringLst;
};

#endif
