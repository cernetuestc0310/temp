/*                                                                           
 * Dibbler - a portable DHCPv6                                               
 *                                                                           
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>                           
 *          Marek Senderski <msend@o2.pl>                                    
 * changes: Krzysztof WNuk <keczi@poczta.onet.pl>                                                                         
 * released under GNU GPL v2 only licence                                
 *                                                                           
 * $Id: SrvAddrMgr.cpp,v 1.19 2008-08-29 00:07:33 thomson Exp $
 *
 */

#include "SrvAddrMgr.h"
#include "AddrClient.h"
#include "AddrIA.h"
#include "AddrAddr.h"
#include "Logger.h"
#include "SrvCfgAddrClass.h"
#include "Portable.h"

TSrvAddrMgr::TSrvAddrMgr(string xmlfile) 
    :TAddrMgr(xmlfile, false) {
    this->CacheMaxSize = 999999999;
    this->cacheRead();
}

TSrvAddrMgr::~TSrvAddrMgr() {
    Log(Debug) << "SrvAddrMgr cleanup." << LogEnd;
}

long TSrvAddrMgr::getTimeout() {
    return this->getValidTimeout();
}

/** 
 * add address for a client. If client's IA is missing, add it, too.
 * 
 * @param clntDuid 
 * @param clntAddr 
 * @param iface 
 * @param IAID 
 * @param T1 
 * @param T2 
 * @param addr 
 * @param pref 
 * @param valid 
 * @param quiet 
 * 
 * @return 
 */
bool TSrvAddrMgr::addClntAddr(SmartPtr<TDUID> clntDuid , SmartPtr<TIPv6Addr> clntAddr,
			      int iface, unsigned long IAID, unsigned long T1, unsigned long T2, 
			      SmartPtr<TIPv6Addr> addr, unsigned long pref, unsigned long valid,
			      bool quiet) {
    // find this client
    SmartPtr <TAddrClient> ptrClient;
    this->firstClient();
    while ( ptrClient = this->getClient() ) {
        if ( (*ptrClient->getDUID()) == (*clntDuid) ) 
            break;
    }

    // have we found this client? 
    if (!ptrClient) {
	if (!quiet) Log(Debug) << "Adding client (DUID=" << clntDuid->getPlain()
			       << ") to addrDB." << LogEnd;
	ptrClient = new TAddrClient(clntDuid);
	this->addClient(ptrClient);
    }

    // find this IA
    SmartPtr <TAddrIA> ptrIA;
    ptrClient->firstIA();
    while ( ptrIA = ptrClient->getIA() ) {
        if ( ptrIA->getIAID() == IAID)
            break;
    }

    // have we found this IA?
    if (!ptrIA) {
	ptrIA = new TAddrIA(iface, clntAddr, clntDuid, T1, T2, IAID);
	ptrClient->addIA(ptrIA);
	if (!quiet)
	    Log(Debug) << "Adding IA (IAID=" << IAID << ") to addrDB." << LogEnd;
    }

    SmartPtr <TAddrAddr> ptrAddr;
    ptrIA->firstAddr();
    while ( ptrAddr = ptrIA->getAddr() ) {
        if (*ptrAddr->get()==*addr)
            break;
    }

    // address already exists
    if (ptrAddr) {
        Log(Warning) << "Address " << *ptrAddr 
		             << " is already assigned to this IA." << LogEnd;
        return false;
    }

    // add address
    ptrAddr = new TAddrAddr(addr, pref, valid);
    ptrIA->addAddr(ptrAddr);
    if (!quiet)
	Log(Debug) << "Adding " << ptrAddr->get()->getPlain() 
		   << " to IA (IAID=" << IAID 
		   << ") to addrDB." << LogEnd;
    return true;
}

/*
 *  Frees address (also deletes IA and/or client, if this was last address)
 */
bool TSrvAddrMgr::delClntAddr(SmartPtr<TDUID> clntDuid,
			      unsigned long IAID, SmartPtr<TIPv6Addr> clntAddr,
			      bool quiet) {
    // find this client
    SmartPtr <TAddrClient> ptrClient;
    this->firstClient();
    while ( ptrClient = this->getClient() ) {
        if ( (*ptrClient->getDUID()) == (*clntDuid) ) 
            break;
    }

    // have we found this client? 
    if (!ptrClient) {
        Log(Warning) << "Client (DUID=" << clntDuid->getPlain() 
		     << ") not found in addrDB, cannot delete address and/or client." << LogEnd;
	return false;
    }

    // find this IA
    SmartPtr <TAddrIA> ptrIA;
    ptrClient->firstIA();
    while ( ptrIA = ptrClient->getIA() ) {
        if ( ptrIA->getIAID() == IAID)
            break;
    }

    // have we found this IA?
    if (!ptrIA) {
        Log(Warning) << "IA (IAID=" << IAID << ") not assigned to client, cannot delete address and/or IA."
		     << LogEnd;
        return false;
    }

    SmartPtr <TAddrAddr> ptrAddr;
    ptrIA->firstAddr();
    while ( ptrAddr = ptrIA->getAddr() ) {
        if (*ptrAddr->get()==*clntAddr)
            break;
    }

    // address already exists
    if (!ptrAddr) {
	Log(Warning) << "Address " << *clntAddr << " not assigned, cannot delete." << LogEnd;
	return false;
    }

    ptrIA->delAddr(clntAddr);
    this->addCachedAddr(clntDuid, clntAddr);
    if (!quiet)
	Log(Debug) << "Deleted address " << *clntAddr << " from addrDB." << LogEnd;
    
    if (!ptrIA->countAddr()) {
	if (!quiet)
	    Log(Debug) << "Deleted IA (IAID=" << IAID << ") from addrDB." << LogEnd;
	ptrClient->delIA(IAID);
    }

    if (!ptrClient->countIA() && !ptrClient->countTA() && !ptrClient->countPD()) {
	if (!quiet)
	    Log(Debug) << "Deleted client (DUID=" << clntDuid->getPlain()
		      << ") from addrDB." << LogEnd;
	this->delClient(clntDuid);
    }

    return true;
}

/** 
 * adds TA address
 * 
 * @param clntDuid 
 * @param clntAddr 
 * @param iface 
 * @param iaid 
 * @param addr 
 * @param pref 
 * @param valid 
 * 
 * @return 
 */
bool TSrvAddrMgr::addTAAddr(SmartPtr<TDUID> clntDuid , SmartPtr<TIPv6Addr> clntAddr,
			    int iface, unsigned long iaid, SmartPtr<TIPv6Addr> addr, 
			    unsigned long pref, unsigned long valid) {
    // find this client
    SmartPtr <TAddrClient> ptrClient;
    this->firstClient();
    while ( ptrClient = this->getClient() ) {
        if ( (*ptrClient->getDUID()) == (*clntDuid) ) 
            break;
    }

    // have we found this client? 
    if (!ptrClient) {
	Log(Debug) << "Adding client (DUID=" << clntDuid->getPlain() << ") to the addrDB." << LogEnd;
	ptrClient = new TAddrClient(clntDuid);
	this->addClient(ptrClient);
    }

    // find this TA
    SmartPtr <TAddrIA> ta;
    ptrClient->firstTA();
    while ( ta = ptrClient->getTA() ) {
        if ( ta->getIAID() == iaid)
            break;
    }

    // have we found this TA?
    if (!ta) {
	ta = new TAddrIA(iface, clntAddr, clntDuid, DHCPV6_INFINITY, DHCPV6_INFINITY, iaid);
	ptrClient->addTA(ta);
	Log(Debug) << "Adding TA (IAID=" << iaid << ") to the addrDB." << LogEnd;
    }

    SmartPtr <TAddrAddr> ptrAddr;
    ta->firstAddr();
    while ( ptrAddr = ta->getAddr() ) {
        if (*ptrAddr->get()==*addr)
            break;
    }

    // address already exists
    if (ptrAddr) {
        Log(Warning) << "Address " << *ptrAddr << " is already assigned to TA (iaid=" 
		     << iaid << ")." << LogEnd;
        return false;
    }

    // add address
    ptrAddr = new TAddrAddr(addr, pref, valid);
    ta->addAddr(ptrAddr);
    Log(Debug) << "Adding " << ptrAddr->get()->getPlain() << " to TA (IAID=" << iaid
	       << ") to addrDB." << LogEnd;
    return true;
}

/** 
 * Frees address (also deletes IA and/or client, if this was last address)
 * 
 * @param clntDuid 
 * @param iaid 
 * @param clntAddr 
 * 
 * @return 
 */
bool TSrvAddrMgr::delTAAddr(SmartPtr<TDUID> clntDuid, unsigned long iaid, 
			      SmartPtr<TIPv6Addr> clntAddr) {
    // find this client
    SmartPtr <TAddrClient> ptrClient;
    this->firstClient();
    while ( ptrClient = this->getClient() ) {
        if ( (*ptrClient->getDUID()) == (*clntDuid) ) 
            break;
    }

    // have we found this client? 
    if (!ptrClient) {
        Log(Warning) << "Client (DUID=" << clntDuid->getPlain() 
		     << ") not found in addrDB, cannot delete address and/or client." << LogEnd;
	return false;
    }

    // find this IA
    SmartPtr <TAddrIA> ta;
    ptrClient->firstTA();
    while ( ta = ptrClient->getTA() ) {
        if ( ta->getIAID() == iaid)
            break;
    }

    // have we found this TA?
    if (!ta) {
        Log(Warning) << "TA (IAID=" << iaid << ") not assigned to client, cannot delete address and/or IA."
		     << LogEnd;
        return false;
    }

    SmartPtr <TAddrAddr> ptrAddr;
    ta->firstAddr();
    while ( ptrAddr = ta->getAddr() ) {
        if (*ptrAddr->get()==*clntAddr)
            break;
    }

    // address already exists
    if (!ptrAddr) {
	Log(Warning) << "Temp. address " << *clntAddr << " not assigned, cannot delete." << LogEnd;
	return false;
    }

    ta->delAddr(clntAddr);
    Log(Debug) << "Deleted temp. address " << *clntAddr << " from addrDB." << LogEnd;
    
    if (!ta->countAddr()) {
	Log(Debug) << "Deleted TA (IAID=" << iaid << ") from addrDB." << LogEnd;
	ptrClient->delTA(iaid);
    }

    if (!ptrClient->countIA() && !ptrClient->countTA() && !ptrClient->countPD()) {
	Log(Debug) << "Deleted client (DUID=" << clntDuid->getPlain()
		   << ") from addrDB." << LogEnd;
	this->delClient(clntDuid);
    }
    
    return true;
}

/*
 * how many addresses does this client have?
 */
unsigned long TSrvAddrMgr::getAddrCount(SmartPtr<TDUID> duid)
{
    SmartPtr <TAddrClient> ptrClient;
    ClntsLst.first();
    while ( ptrClient = ClntsLst.get() ) {
        if ( (*ptrClient->getDUID()) == (*duid))
            break;
    }
    // Have we found this client? 
    if (!ptrClient) {
        return 0;
    }

    unsigned long count=0;
    
    // look at each of client's IAs
    SmartPtr <TAddrIA> ptrIA;
    ptrClient->firstIA();
    while ( ptrIA = ptrClient->getIA() ) {
        count += ptrIA->countAddr();
    }
    return count;
}

bool TSrvAddrMgr::addrIsFree(SmartPtr<TIPv6Addr> addr)
{
    // for each client...
    SmartPtr <TAddrClient> ptrClient;
    ClntsLst.first();
    while ( ptrClient = ClntsLst.get() ) 
    {

        // look at each client's IAs
        SmartPtr <TAddrIA> ptrIA;
        ptrClient->firstIA();
        while ( ptrIA = ptrClient->getIA() ) 
        {
            SmartPtr<TAddrAddr> ptrAddr;
            if (ptrAddr = ptrIA->getAddr(addr) )
                return false;
        }
    }
    return true;
}

/** 
 * Verifies if addr is unused
 * 
 * @param addr 
 * 
 * @return 
 */
bool TSrvAddrMgr::taAddrIsFree(SmartPtr<TIPv6Addr> addr)
{
    // for each client...
    SmartPtr <TAddrClient> ptrClient;
    ClntsLst.first();
    while ( ptrClient = ClntsLst.get() ) {
        // look at each client's TAs
        SmartPtr <TAddrIA> ta;
        ptrClient->firstTA();
        while ( ta = ptrClient->getTA() ) 
        {
            if (ta->getAddr(addr) )
                return false;
        }
    }
    return true;
}

void TSrvAddrMgr::getAddrsCount(
     SmartPtr<TContainer<SmartPtr<TSrvCfgAddrClass> > > classes,
     long    *clntCnt,
     long    *addrCnt,
     SmartPtr<TDUID> duid, 
     int iface)
{   
    memset(clntCnt,0,sizeof(long)*classes->count());
    memset(addrCnt,0,sizeof(long)*classes->count());
    int classNr=0;

    SmartPtr<TAddrClient> ptrClient;
    firstClient();
    while(ptrClient=getClient())
    {
        bool thisClient=(*(ptrClient->getDUID())==*duid);
        ptrClient->firstIA();
        SmartPtr<TAddrIA> ptrIA;
        while(ptrIA=ptrClient->getIA())
        {
            SmartPtr<TAddrAddr> ptrAddr;
            ptrIA->firstAddr();
            while(ptrAddr=ptrIA->getAddr())
            {
                if(ptrIA->getIface()==iface)
                {
                    SmartPtr<TSrvCfgAddrClass> ptrClass;
                    classes->first();
		    classNr=0;
                    while(ptrClass=classes->get())
                    {
                        if(ptrClass->addrInPool(ptrAddr->get()))
                        {
                            if(thisClient)
                                clntCnt[classNr]++;
                            addrCnt[classNr]++;
                        }
                        classNr++;
                    }
                }
            }
        }            
    }
}

SPtr<TIPv6Addr> TSrvAddrMgr::getFirstAddr(SPtr<TDUID> clntDuid)
{
    SmartPtr<TAddrClient> ptrAddrClient = this->getClient(clntDuid);	
    if (!ptrAddrClient) { 
	Log(Warning) << "Unable to find client in the addrDB."; 
	return 0;
    }
    ptrAddrClient->firstIA();
    SmartPtr<TAddrIA> ptrAddrIA = ptrAddrClient->getIA();
    if (!ptrAddrIA) { 
	Log(Warning) << "Client does not have any addresses assigned." << LogEnd; 
	return 0;
    }
    ptrAddrIA->firstAddr();
    SmartPtr<TAddrAddr> addr = ptrAddrIA->getAddr();
    if (!addr) {
	return 0;
    }
    return addr->get();
}



/* ******************************************************************************** */
/* *** ADDRESS CACHE ************************************************************** */
/* ******************************************************************************** */

/** 
 * remove outdated addresses 
 * 
 */
void TSrvAddrMgr::doDuties()
{
    SmartPtr<TAddrClient> ptrClient;
    SmartPtr<TAddrIA>     ptrIA;
    SmartPtr<TAddrAddr>   ptrAddr;
    bool anyDeleted=false;
    // for each client...
    this->firstClient();
    while (ptrClient = this->getClient() ) 
    {
        // ... which has outdated adresses
        if (!ptrClient->getValidTimeout()) 
        {
            ptrClient->firstIA();
            while ( ptrIA = ptrClient->getIA() ) 
            {
                // has this IA outdated addressess?
                if ( !ptrIA->getValidTimeout() ) 
                {
                    ptrIA->firstAddr();
                    while ( ptrAddr = ptrIA->getAddr() )  
                    {
                        if (!ptrAddr->getValidTimeout()) 
                        {
                            // delete this address
			    Log(Notice) << "Addr " << *(ptrAddr->get()) << " in IA (IAID="
					<< ptrIA->getIAID() << ") in client (DUID=\"";
                            if (ptrClient->getDUID()) 
                            {
                                Log(Cont) << *ptrClient->getDUID();
                            }
                            Log(Cont) << "\") has expired." << dec << LogEnd;
                            ptrIA->delAddr(ptrAddr->get());
                            anyDeleted=true;
                        }
                    }
                    if (!ptrIA->getAddrCount())
                    {
                        anyDeleted=true;
                        ptrClient->delIA(ptrIA->getIAID());
                    }
                }
            }
        }
        if (!(ptrClient->countIA()))
        {
            this->delClient(ptrClient->getDUID());
            anyDeleted=true;
        };
    }
    if (anyDeleted) 
        this->dump();
}

/** 
 * returns address cached for this client. 
 * 
 * @param clntDuid 
 * @param clntAddr 
 * 
 * @return cached address. 0 if cached address is not found.
 */
SPtr<TIPv6Addr> TSrvAddrMgr::getCachedAddr(SPtr<TDUID> clntDuid) {
    if (!this->CacheMaxSize)
	return 0;
    SPtr<TSrvCacheEntry> entry;
    this->Cache.first();

    while (entry = this->Cache.get()) {
	if (!entry->Duid)
	    continue; // something is wrong. VERY wrong. But shut up and continue.
	if (*entry->Duid == *clntDuid) {
	    Log(Debug) << "Cache: Cached address for client (DUID=" << clntDuid->getPlain() << ") found: " 
		       << entry->Addr->getPlain() << LogEnd;
	    return entry->Addr;
	}
    }

    Log(Debug) << "Cache: There are no cached entries for client (DUID=" << clntDuid->getPlain() << ")." << LogEnd;
    return 0;
}

/** 
 * this function deletes cache entry
 * 
 * @param addr 
 * 
 * @return 
 */
bool TSrvAddrMgr::delCachedAddr(SPtr<TIPv6Addr> addr) {
    if (!this->CacheMaxSize)
	return false;
    
    this->Cache.first();
    SPtr<TSrvCacheEntry> entry;
    while (entry = this->Cache.get()) {
	if (!entry->Addr)
	    continue; // something is wrong. VERY wrong. But shut up and continue.
	if ( *(entry->Addr) == *addr) {
	    this->Cache.del();
	    this->Cache.first();
	    Log(Debug) << "Cache: Address " << *addr << " was deleted." << LogEnd;
	    return true;
	}
    }
    Log(Debug) << "Cache: Attempt to delete " << *addr << " failed." << LogEnd;
    return false;
}

/** 
 * this function deletes cache entry
 * 
 * @param clntDuid 
 * 
 * @return 
 */
bool TSrvAddrMgr::delCachedAddr(SPtr<TDUID> clntDuid) {
    if (!this->CacheMaxSize)
	return false;
    
    this->Cache.first();
    SPtr<TSrvCacheEntry> entry;
    while (entry = this->Cache.get()) {
	if (!entry->Duid)
	    continue; // something is wrong. VERY wrong. But shut up and continue.
	if ( *(entry->Duid) == *clntDuid) {
	    this->Cache.del();
	    this->Cache.first();
	    Log(Debug) << "Cache: Entry for client (DUID=" << clntDuid->getPlain() << ") was deleted." << LogEnd;
	    return true;
	}
    }
    // delete attempt is done on multiple occasions as a safety precausion, so don't warn if it is missing
    // Log(Debug) << "Cache: Attempt to delete entry for client (DUID=" << clntDuid->getPlain() << ") failed." << LogEnd;
    return false;
}

/** 
 * this function adds an address to a cache. If there is entry for this client, updates it
 * 
 * @param clntDuid 
 * @param cachedAddr 
 */
void TSrvAddrMgr::addCachedAddr(SPtr<TDUID> clntDuid, SPtr<TIPv6Addr> cachedAddr) {
    if (!this->CacheMaxSize)
	return;
    SPtr<TSrvCacheEntry> entry;

    // is there an entry for this client, delete it. New entry will be added at the end
    this->delCachedAddr(clntDuid);
    
    entry = new TSrvCacheEntry();
    entry->Duid      = clntDuid;
    entry->Addr      = cachedAddr;
    Log(Debug) << "Cache: Address " << cachedAddr->getPlain() << " added for client (DUID=" << clntDuid->getPlain() << "). " << LogEnd; 
    this->Cache.append(entry);
    this->checkCacheSize();
}

void TSrvAddrMgr::setCacheSize(int bytes) {
    int entrySize = sizeof(TSrvCacheEntry) + sizeof(TIPv6Addr) + sizeof(TDUID);
    this->CacheMaxSize = bytes/entrySize;
    Log(Debug) << "Cache: size set to " << bytes << " bytes, 1 cache entry size is " << entrySize 
	       << " bytes, so maximum " << this->CacheMaxSize << " address-client pair(s) may be cached." << LogEnd;
    this->checkCacheSize();
}

/** 
 * this function checks if the cache size was not exceeded. If that is so, oldest entries are removed
 * 
 */
void TSrvAddrMgr::checkCacheSize() {
    if (this->Cache.count() <= this->CacheMaxSize)
	return;

    // there are too many cached elements, delete some
    while (this->Cache.count() > this->CacheMaxSize) {
	this->Cache.delFirst();
    }
}

void TSrvAddrMgr::print(ostream & out) {
    out << "  <cache size=\"" << this->Cache.count() << "\"/>" << endl;
}

void TSrvAddrMgr::dump() {
    TAddrMgr::dump(); // perform normal dump of the AddrMgr
    cacheDump();
}

/** 
 * dumps address cache into a file specified by SRVCACHE_FILE
 * 
 */
void TSrvAddrMgr::cacheDump() {
    std::ofstream f;
    f.open(SRVCACHE_FILE);
    if (!f.is_open()) {
	Log(Error) << "Cache: File " << SRVCACHE_FILE << " creation failed." << LogEnd;
	return;
    }
    f << "<cache size=\"" << this->Cache.count() << "\">" << endl;
    SPtr<TSrvCacheEntry> x;
    this->Cache.first();
    while (x=this->Cache.get()) {
	
	f << "  <entry duid=\"";
	if (x->Duid)
	    f << x->Duid->getPlain();
	f << "\">";
	if (x->Addr)
	    f << x->Addr->getPlain();
	f << "</entry>" << endl;
    }
    f << "</cache>" << endl;
    f.close();
}

/** 
 * dumps address cache into a file specified by SRVCACHE_FILE
 * 
 */
void TSrvAddrMgr::cacheRead() {

    this->Cache.clear();
    bool started = false;
    bool ended = false;
    bool parsed = false;
    int lineno = 0;
    int entries = 0;
    std::ifstream f;
    string s;
    f.open(SRVCACHE_FILE);
    if (!f.is_open()) {
	Log(Warning) << "Cache: Unable to open cache file " << SRVCACHE_FILE << "." << LogEnd;
	return;
    }

    while (!f.eof()) {
	parsed = false;
	getline(f,s);
	string::size_type pos=0;
	if ( (pos = s.find("<cache")!=string::npos) ) {
	    // parse beginning

	    started = true;
	    if ( (pos = s.find("size=\""))!=string::npos ) {
		s = s.substr(pos+6);
		entries = atoi(s.c_str());
		Log(Debug) << "Cache:" << SRVCACHE_FILE << " file: parsing started, expecting " << entries << " entries." << LogEnd;
	    } else {
		Log(Debug) << "Cache:" << SRVCACHE_FILE << " file:unable to find entries count. size=\"...\" missing in line " 
			   << lineno << "." << LogEnd;
		return;
	    }
	}
	
	if (s.find("<entry")!=string::npos) {
	    if (!started) {
		Log(Error) << "Cache:" << SRVCACHE_FILE << " file: opening tag <cache> missing." << LogEnd;
		return;
	    }
	    if ( (pos=s.find("duid=\""))!=string::npos) {
		s = s.substr(pos+6);
		string duid = s.substr(0, s.find("\""));
		s = s.substr(s.find("\"")+2);
		string addr = s.substr(0, s.find("<"));

		SPtr<TIPv6Addr> tmp2 = new TIPv6Addr(addr.c_str(), true);
		SPtr<TDUID>     tmp1 = new TDUID(duid.c_str());
		this->addCachedAddr(tmp1, tmp2);

	    } else {
		Log(Error) << "Cache: " << SRVCACHE_FILE << " file: missing duid=\"...\" in line " << lineno
			   << "." << LogEnd;
		return;
	    }
	}
	
	if (s.find("</cache>")!=string::npos) {
	    if (!started) {
		Log(Error) << "Cache: Reading file " << SRVCACHE_FILE << " failed: closing tag </cache> found at line " << lineno
			   << ", but opening tag is missing." << LogEnd;
		f.close();
		return;
	    }
	    parsed = true;
	    ended = true;
	}
    }
    f.close();

    if (this->Cache.count() != entries) {
	Log(Debug) << "Cache: " << SRVCACHE_FILE << " file: " << entries << " entries expected, but " 
		   << this->Cache.count() << " found." << LogEnd;
    }
}
