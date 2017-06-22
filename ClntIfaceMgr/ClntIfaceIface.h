/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntIfaceIface.h,v 1.10 2008-08-29 00:07:27 thomson Exp $
 *
 * $Log: ClntIfaceIface.h,v $
 * Revision 1.10  2008-08-29 00:07:27  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.9  2008-08-21 00:25:08  thomson
 * Notify mechanism implemented.
 *
 * Revision 1.8  2008-08-19 23:27:33  thomson
 * TunnelMode additional support.
 *
 *
 */

#ifndef CLNTIFACEIFACE_H
#define CLNTIFACEIFACE_H

#include "Iface.h"
#include "SmartPtr.h"
#include "DUID.h"
#include "ClntIfaceMgr.h"
#include "OptFQDN.h"

class TClntIfaceIface: public TIfaceIface {
 public:
    friend ostream & operator <<(ostream & strum, TClntIfaceIface &x);
    TClntIfaceIface(char * name, int id, unsigned int flags, char* mac, 
		    int maclen, char* llAddr, int llAddrCnt, char * globalAddr, 
		    int globalAddrCnt, int hwType);
    ~TClntIfaceIface();

    bool setDNSServerLst(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, List(TIPv6Addr) addrs);
    bool setDomainLst(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, List(string) domains);
    bool setNTPServerLst(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, List(TIPv6Addr) addrs);
    bool setTimezone(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, string timezone);
    bool setSIPServerLst(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, List(TIPv6Addr) addrs);
    bool setSIPDomainLst(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, List(string) domains);
    bool setFQDN(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, string fqdn);
    bool setNISServerLst(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, List(TIPv6Addr) addrs);
    bool setNISDomain(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, string domain);
    bool setNISPServerLst(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, List(TIPv6Addr) addrs);
    bool setNISPDomain(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, string domain);
    bool setLifetime(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> srv, unsigned int life);

    bool setTunnelMode(int mode, SPtr<TIPv6Addr> remoteEndpoint);
    int  getTunnelMode();
    SPtr<TIPv6Addr> getTunnelEndpoint();

    void removeAllOpts();
    unsigned int getTimeout();

    //Getters for FQDN
    string getFQDN();
    List(TIPv6Addr) getDNSServerLst();

 private:

    void addString(const char * filename, const char * str);
    void delString(const char * filename, const char * str);
    void setString(const char * filename, const char * str);

    List(TIPv6Addr)     DNSServerLst;
    SmartPtr<TIPv6Addr> DNSServerLstAddr;
    SmartPtr<TDUID>     DNSServerLstDUID;

    List(string)        DomainLst;
    SmartPtr<TIPv6Addr> DomainLstAddr;
    SmartPtr<TDUID>     DomainLstDUID;

    List(TIPv6Addr)     NTPServerLst;
    SmartPtr<TIPv6Addr> NTPServerLstAddr;
    SmartPtr<TDUID>     NTPServerLstDUID;

    string Timezone;
    SmartPtr<TIPv6Addr> TimezoneAddr;
    SmartPtr<TDUID>     TimezoneDUID;

    string FQDN;
    SmartPtr<TIPv6Addr> FQDNAddr;
    SmartPtr<TDUID>     FQDNDUID;

    List(TIPv6Addr)     SIPServerLst;
    SmartPtr<TIPv6Addr> SIPServerLstAddr;
    SmartPtr<TDUID>     SIPServerLstDUID;

    List(string)        SIPDomainLst;
    SmartPtr<TIPv6Addr> SIPDomainLstAddr;
    SmartPtr<TDUID>     SIPDomainLstDUID;

    List(TIPv6Addr)     NISServerLst;
    SmartPtr<TIPv6Addr> NISServerLstAddr;
    SmartPtr<TDUID>     NISServerLstDUID;

    string              NISDomain;
    SmartPtr<TIPv6Addr> NISDomainAddr;
    SmartPtr<TDUID>     NISDomainDUID;

    List(TIPv6Addr)     NISPServerLst;
    SmartPtr<TIPv6Addr> NISPServerLstAddr;
    SmartPtr<TDUID>     NISPServerLstDUID;

    string              NISPDomain;
    SmartPtr<TIPv6Addr> NISPDomainAddr;
    SmartPtr<TDUID>     NISPDomainDUID;

    int TunnelMode;
    SPtr<TIPv6Addr> TunnelEndpoint;
    
    unsigned int LifetimeTimeout;
    unsigned int LifetimeTimestamp;

};

#endif
