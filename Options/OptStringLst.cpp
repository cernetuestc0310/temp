/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 licence
 *
 * $Id: OptStringLst.cpp,v 1.11 2007-12-25 08:11:57 thomson Exp $
 *
 * $Log: OptStringLst.cpp,v $
 * Revision 1.11  2007-12-25 08:11:57  thomson
 * Several fixes after 2nd bakeoff meeting.
 *
 * Revision 1.10  2007-08-26 10:26:19  thomson
 * Possible segfault in REBIND processing,
 * gcc 4.3.0 conformance
 *
 * Revision 1.9  2007-05-01 12:06:50  thomson
 * Fix for possible misalignment in strings.
 *
 * Revision 1.8  2007-01-21 21:29:45  thomson
 * Portability fixes (MIPSEL and 64bits)
 *
 * Revision 1.7  2005-07-17 21:09:52  thomson
 * Minor improvements for 0.4.1 release.
 *
 * Revision 1.6  2005/05/10 00:01:46  thomson
 * Minor compilation bug fixed.
 *
 * Revision 1.5  2005/03/15 23:02:31  thomson
 * 0.4.0 release.
 *
 * Revision 1.4  2005/03/07 23:48:16  thomson
 * Typo fix.
 *
 * Revision 1.3  2005/03/07 23:36:14  thomson
 * Minor changes.
 *
 * Revision 1.2  2005/03/07 22:44:22  thomson
 * DomainList is now stored and parsed approprietly (bug #104)
 *
 * Revision 1.1  2004/11/02 01:23:13  thomson
 * Initial revision
 *
 * Revision 1.3  2004/04/11 18:10:56  thomson
 * CRLF fixed.
 *
 * Revision 1.2  2004/03/29 18:53:08  thomson
 * Author/Licence/cvs log/cvs version headers added.
 *
 *
 */

#include <stdlib.h>
#include "Portable.h"
#include "OptStringLst.h"
#include "DHCPConst.h"
#include "Logger.h"

using namespace std;

TOptStringLst::TOptStringLst(int type, List(string) strLst, TMsg* parent)
    :TOpt(type, parent)
{
    this->StringLst = strLst;
}

TOptStringLst::TOptStringLst(int type, char *&buf, int &bufsize, TMsg* parent)
    :TOpt(type, parent) {
    int len;
    string domain = "";
    char* str=new char[bufsize+1];
    str[bufsize]=0;
    while (bufsize) {
	    len = (char)*buf;
	    if (len>bufsize) {
	        Log(Debug) << "Option parsing failed. String length is specified as " << len
		           << ", but remaining buffer is only " << bufsize << " bytes long." << LogEnd;
	        StringLst.clear();
	        return;
	    }
        if (len==0) {
            // end of domain
            if (domain.length()) {
        	    SmartPtr<string> x = new string(domain);
                this->StringLst.append(x);
            }
            bufsize -= len+1;
            buf     += len+1;
            domain   = "";
            continue;
        }
	    memcpy(str,buf+1,len);
	    str[len]=0;

        if (domain.length())
            domain += string(".");
        domain = domain + string(str);

	    bufsize -= len+1;
	    buf     += len+1;
    }
    delete [] str;
}

char * TOptStringLst::storeSelf(char* buf)
{
    SmartPtr<string> x;
    *(short*)buf = htons(OptType);
    buf+=2;
    *(short*)buf = htons(getSize()-4);
    buf+=2;
    int len = 0;
    std::string::size_type dotpos;
   
    StringLst.first();
    while (x = StringLst.get() ) {
        string cp(*x);
        len = (int)(x->length());

        dotpos = string::npos;
        while (cp.find(".")!=string::npos) {
	    if (*cp.c_str()=='.')
		break;
            dotpos = cp.find(".");
            if (dotpos!=string::npos) {
                *buf = dotpos;
                buf++;
                memcpy(buf, cp.c_str(), dotpos);
                buf+=dotpos;
                cp = cp.substr(dotpos+1);
            }
        }

        *buf = cp.length(); // length of the string
        buf++;
        memcpy(buf, cp.c_str(), cp.length());
	buf+= cp.length();
	*buf=0;
	buf++;
    }
    return buf;
}

int TOptStringLst::getSize() {
    int len = 0;
    int tmplen = 0;
    SmartPtr<string> x;
    StringLst.first();
    while ( x = StringLst.get() ) {
	const char * c = x->c_str();
	tmplen = x->length();
	if (c[tmplen]=='.')
	    len++;
	len += x->length()+2;
    }
    return len+4; // 5=4(std.option header) + 1 (final 0)
}
string TOptStringLst::getString() {
    return *(this->StringLst.get());
}
