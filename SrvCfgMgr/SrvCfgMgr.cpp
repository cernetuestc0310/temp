/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 * changes: Petr Pisar <petr.pisar(at)atlas(dot)cz>
 *          Michal Kowalczuk <michal@kowalczuk.eu>
 *          Nguyen Vinh Nghiem
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: SrvCfgMgr.cpp,v 1.58 2008-10-12 20:07:31 thomson Exp $
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include "SmartPtr.h"
#include "Portable.h"
#include "SrvCfgMgr.h"
#include "SrvCfgIface.h"
#include "Logger.h"

using namespace std;

#include "IfaceMgr.h"
#include "SrvIfaceMgr.h"

#include "AddrClient.h"
#include "TimeZone.h"

#include "FlexLexer.h"
#include "SrvParser.h"

int TSrvCfgMgr::NextRelayID = RELAY_MIN_IFINDEX;

TSrvCfgMgr::TSrvCfgMgr(SmartPtr<TSrvIfaceMgr> ifaceMgr, string cfgFile, string xmlFile)
    :TCfgMgr((Ptr*)ifaceMgr)
{
    this->IfaceMgr = ifaceMgr;
    this->XmlFile = xmlFile;

    // load config file
    if (!this->parseConfigFile(cfgFile)) {
	this->IsDone = true;
	return;
    }

    // load or create DUID
    string duidFile = (string)SRVDUID_FILE;
    if (!this->setDUID(duidFile)) {
		this->IsDone=true;
		return;
    }

    this->dump();

#ifndef MOD_DISABLE_AUTH
    AuthKeys = new KeyList();
#endif

    IsDone = false;
}

bool TSrvCfgMgr::parseConfigFile(string cfgFile) {
    int result;
    ifstream f;

    // parse config file
    f.open( cfgFile.c_str() );
    if ( ! f.is_open() ) {
	    Log(Crit) << "Unable to open " << cfgFile << " file." << LogEnd;
	    return false;
    } else {
	Log(Notice) << "Parsing " << cfgFile << " config file..." << LogEnd;
    }
    yyFlexLexer lexer(&f,&clog);
    SrvParser parser(&lexer);
    result = parser.yyparse();
    Log(Debug) << "Parsing " << cfgFile << " done." << LogEnd;
    f.close();

    this->LogLevel = logger::getLogLevel();
    this->LogName  = logger::getLogName();

    if (result) {
        Log(Crit) << "Fatal error during config parsing." << LogEnd;
	this->IsDone = true;
        return false;
    }

    // setup global options
    this->setGlobalOptions(parser.ParserOptStack.getLast());

    // setup ClientClass  List
    ClientClassLst = parser.SrvCfgClientClassLst;

    Log(Info) << ClientClassLst.count() << " client class(es) defined." << LogEnd;

    // analyse interfaces mentioned in config file
    if (!this->matchParsedSystemInterfaces(&parser)) {
	this->IsDone = true;
	return false;
    }

    // check for invalid values, e.g. T1>T2
    if(!this->validateConfig()) {
	this->IsDone = true;
        return false;
    }

    if (this->stateless()) {
	Log(Notice) << "Running in stateless mode." << LogEnd;
    } else {
	Log(Notice) << "Running in stateful mode." << LogEnd;
    }

    return true;
}

void TSrvCfgMgr::dump() {
    std::ofstream xmlDump;
    xmlDump.open(this->XmlFile.c_str());
    xmlDump << *this;
    xmlDump.close();
}

bool TSrvCfgMgr::setGlobalOptions(SmartPtr<TSrvParsGlobalOpt> opt) {
    this->Workdir          = opt->getWorkDir();
    this->Stateless        = opt->getStateless();
    this->CacheSize        = opt->getCacheSize();
    this->InterfaceIDOrder = opt->getInterfaceIDOrder();
    this->InactiveMode     = opt->getInactiveMode(); // should the client accept not ready interfaces?
    this->GuessMode        = opt->getGuessMode();

#ifndef MOD_DISABLE_AUTH
    this->DigestLst        = opt->getDigest();
    this->AuthKeyGenNonceLen = opt->getAuthKeyLen();
    this->AuthLifetime = opt->getAuthLifetime();
#endif

    return true;
}

/*
 * Now parsed information should be placed in config manager
 * in accordance with information provided by interface manager
 */
bool TSrvCfgMgr::matchParsedSystemInterfaces(SrvParser *parser) {
    int cfgIfaceCnt;
    cfgIfaceCnt = parser->SrvCfgIfaceLst.count();
    Log(Debug) << cfgIfaceCnt << " interface(s) specified in " << SRVCONF_FILE << LogEnd;

    SmartPtr<TSrvCfgIface> cfgIface;
    SmartPtr<TIfaceIface>  ifaceIface;

    parser->SrvCfgIfaceLst.first();
    while(cfgIface=parser->SrvCfgIfaceLst.get()) {
	// for each interface from config file

	// map deny and allow list
    	cfgIface->mapAllowDenyList(parser->SrvCfgClientClassLst);

	// relay interface
	if (cfgIface->isRelay()) {
	    cfgIface->setID(this->NextRelayID++);
	    if (!this->setupRelay(cfgIface)) {
		return false;
	    }
	    this->addIface(cfgIface);

	    continue; // skip physical interface checking part
	}

	// physical interface
	if (cfgIface->getID()==-1) {
	    // ID==-1 means that user referenced to interface by name
	    ifaceIface = IfaceMgr->getIfaceByName(cfgIface->getName());
	} else {
	    ifaceIface = IfaceMgr->getIfaceByID(cfgIface->getID());
	}
	if (!ifaceIface) {
	    Log(Crit) << "Interface " << cfgIface->getFullName()
		      << " is not present in the system or does not support IPv6."
		      << LogEnd;
	    return false;
	}

	// Complete name and ID (one of them usually misses depending on
	// identifier used in config file.
	// FIXME: Client's class uses setIface{Name,ID}(). We should unite the
	// method names.
	cfgIface->setName(ifaceIface->getName());
	cfgIface->setID(ifaceIface->getID());


	// Check for link scope address presence
        if (!ifaceIface->countLLAddress()) {
            if (this->inactiveMode()) {
                Log(Notice) << "Interface " << ifaceIface->getFullName()
                            << " is not operational yet (does not have link scope address), skipping it for now." << LogEnd;
                this->addIface(cfgIface);
                this->makeInactiveIface(cfgIface->getID(), true); // move it to InactiveLst
		continue;
            }

	    Log(Crit) << "Interface " << ifaceIface->getName() << "/" << ifaceIface->getID()
		      << " is down or doesn't have any link scope address." << LogEnd;
	    return false;
        }

	// Check if the interface is during bring-up phase (i.e. DAD procedure for link-local addr is not complete yet)
	char tmp[64];
	ifaceIface->firstLLAddress();
	inet_ntop6(ifaceIface->getLLAddress(), tmp);
	if (is_addr_tentative(ifaceIface->getName(), ifaceIface->getID(), tmp) == LOWLEVEL_TENTATIVE_YES) {
	    Log(Notice) << "Interface " << ifaceIface->getFullName() << " has link-scope address " << tmp
			<< ", but it is currently tentative." << LogEnd;

	    if (this->inactiveMode()) {
		Log(Notice) << "Interface " << ifaceIface->getFullName()
			    << " is not operational yet (link-scope address is not ready), skipping it for now." << LogEnd;
		addIface(cfgIface);
		makeInactiveIface(cfgIface->getID(), true); // move it to InactiveLst
		continue;
	    }

	    Log(Crit) << "Interface " << ifaceIface->getFullName()
		      << " has tentative link-scope address (and inactive-mode is disabled)." << LogEnd;
	    return false;

	}

	this->addIface(cfgIface);
	Log(Info) << "Interface " << cfgIface->getFullName()
		  << " configuration has been loaded." << LogEnd;
    }
    return true;
}

SmartPtr<TSrvCfgIface> TSrvCfgMgr::getIface() {
	return this->SrvCfgIfaceLst.get();
}

void TSrvCfgMgr::addIface(SmartPtr<TSrvCfgIface> ptr) {
    SrvCfgIfaceLst.append(ptr);
}

/**
 * Moves interface identified by ifindex between SrvCfgIfaceLst and InactiveLst.
 *
 * @param ifindex Index of the moving interface
 * @param inactive true for makeing inactive, false for makeing active
 */
void TSrvCfgMgr::makeInactiveIface(int ifindex, bool inactive) {
    SmartPtr<TSrvCfgIface> x;

    if (inactive)
    {
	SrvCfgIfaceLst.first();
	while (x= SrvCfgIfaceLst.get()) {
	    if (x->getID() == ifindex) {
		Log(Info) << "Switching " << x->getFullName() << " to inactive-mode." << LogEnd;
		SrvCfgIfaceLst.del();
		InactiveLst.append(x);
		return;
	    }
	}
	Log(Error) << "Unable to switch interface ifindex=" << ifindex << " to inactive-mode: interface not found." << LogEnd;
	// something is wrong, VERY wrong
    }

    else
    {
        InactiveLst.first();
        while (x= InactiveLst.get()) {
            if (x->getID() == ifindex) {
                Log(Info) << "Switching " << x->getFullName() << " to normal mode." << LogEnd;
                InactiveLst.del();
                InactiveLst.first();
                addIface(x);
                return;
            }
        }

        Log(Error) << "Unable to switch interface ifindex=" << ifindex << " from inactive-mode to normal operation: interface not found." << LogEnd;
    }
}

/**
 * Returns number of inactive interfaces
 */
int TSrvCfgMgr::inactiveIfacesCnt() {
    return InactiveLst.count();
}

void TSrvCfgMgr::firstIface() {
    SrvCfgIfaceLst.first();
}

long TSrvCfgMgr::countIface() {
    return SrvCfgIfaceLst.count();
}

/**
 *
 * Tryies to find interface in list of inactive interfaces which becames
 * ready. In positive case such an interface is moved back into standard list.
 * Only first matching interface is processed.
 *
 * @returns Interface that becomes ready and has been moved out of inactive list.
 * If no such interface exists, 0 will be returned.
 */
SPtr<TSrvCfgIface> TSrvCfgMgr::checkInactiveIfaces() {
    if (!InactiveLst.count())
	return 0;

    IfaceMgr->redetectIfaces();
    SPtr<TSrvCfgIface> x;
    SPtr<TIfaceIface> iface;
    InactiveLst.first();
    while (x = InactiveLst.get()) {
	iface = IfaceMgr->getIfaceByID(x->getID());
	if (!iface) {
	    Log(Error) << "TSrvCfgMgr::checkInactiveIfaces(): " <<
		"Unable to find interface with ifindex=" << x->getID() << LogEnd;
	    continue;
	}
	iface->firstLLAddress();
	if (iface->flagUp() && iface->flagRunning() && iface->getLLAddress()) {
	    // check if its link-local address is not tentative
	    char tmp[64];
	    iface->firstLLAddress();
	    inet_ntop6(iface->getLLAddress(), tmp);
	    if (is_addr_tentative(iface->getName(), iface->getID(), tmp)==LOWLEVEL_TENTATIVE_YES) {
		Log(Debug) << "Interface " << iface->getFullName() << " is up and running, but link-scope address " << tmp
			   << " is currently tentative." << LogEnd;
		continue;
	    }


	    makeInactiveIface(x->getID(), false); // move it to InactiveLst
	    return x;
	}
    }

    return 0;
}


TSrvCfgMgr::~TSrvCfgMgr() {
    Log(Debug) << "SrvCfgMgr cleanup." << LogEnd;
}

/** 
 * checks if the specified address belongs to the currently supported TA pool
 * (used in CONFIRM message)
 * 
 * @param iface 
 * @param addr 
 * 
 * @return 
 */
bool TSrvCfgMgr::isTAAddrSupported(int iface, SmartPtr<TIPv6Addr> addr) {
    SmartPtr<TSrvCfgIface> ptrIface = this->getIfaceByID(iface);
    if (!ptrIface)
	return false;
    ptrIface->firstTA();
    SmartPtr<TSrvCfgTA> ta = ptrIface->getTA();
    if (!ta)
	return false;
    return ta->addrInPool(addr);
}

/** 
 * checks if this Addr is already configured in that IA belonging to that client
 * (used in CONFIRM message)
 * 
 * @param iface 
 * @param addr 
 * 
 * @return true, if address is supported
 */
bool TSrvCfgMgr::isIAAddrSupported(int iface, SmartPtr<TIPv6Addr> addr) {
    SmartPtr<TSrvCfgIface> ptrIface = this->getIfaceByID(iface);
    if (!ptrIface)
	return false;
    ptrIface->firstAddrClass();
    SmartPtr<TSrvCfgAddrClass> addrClass;
    while (addrClass = ptrIface->getAddrClass()) {
	if (addrClass->addrInPool(addr))
	    return true;
    }
    return false;
}

/** 
 * returns how many addresses can be assigned to this client?
 * factors used:
 * - iface-max-lease
 * - clntSupported()
 * - class-max-lease in each class
 * - assignedCount in each class
 * 
 * @param clntDuid 
 * @param clntAddr 
 * @param iface 
 * 
 * @return return value <= clnt-max-lease <= iface-max-lease
 */
long TSrvCfgMgr::countAvailAddrs(SmartPtr<TDUID> clntDuid,
				 SmartPtr<TIPv6Addr> clntAddr,  int iface)
{
    // FIXME: long long long int (128bit) could come in handy
    double avail         = 0; // how many are available?
    double ifaceAssigned = 0; // how many are assigned on this iface?
    SmartPtr<TSrvCfgIface> ptrIface;
    ptrIface = this->getIfaceByID(iface);
    if (!ptrIface) {
	Log(Error) << "Interface " << iface << " does not exist in SrvCfgMgr." << LogEnd;
	return 0;
    }

    unsigned long ifaceMaxLease = ptrIface->getIfaceMaxLease();

    SmartPtr<TSrvCfgAddrClass> ptrClass;
    ptrIface->firstAddrClass();
    while (ptrClass = ptrIface->getAddrClass()) {
	if (!ptrClass->clntSupported(clntDuid,clntAddr))
	    continue;
	unsigned long classMaxLease;
	unsigned long classAssigned;
	double tmp;
	classMaxLease = ptrClass->getClassMaxLease();
	classAssigned = ptrClass->getAssignedCount();
	tmp = classMaxLease - classAssigned;
	ifaceAssigned += classAssigned;
	if (tmp>0)
	    avail += tmp;
    }

    if (avail > (ifaceMaxLease-ifaceAssigned))
	avail = ifaceMaxLease-ifaceAssigned;
    return (long)avail;
}

/** 
 * return class, which address belongs to
 * 
 * @param iface 
 * @param addr 
 * 
 * @return pointer to the class (or 0, if no suitable class found)
 */
SmartPtr<TSrvCfgAddrClass> TSrvCfgMgr::getClassByAddr(int iface, SmartPtr<TIPv6Addr> addr)
{
    this->firstIface();
    SmartPtr<TSrvCfgIface> ptrIface;
    ptrIface = this->getIfaceByID(iface);

    if (!ptrIface) {
	Log(Error) << "Trying to find class on unknown (" << iface <<") interface." << LogEnd;
	return 0; // NULL
    }

    SmartPtr<TSrvCfgAddrClass> ptrClass;
    ptrIface->firstAddrClass();
    while (ptrClass = ptrIface->getAddrClass()) {
	if (ptrClass->addrInPool(addr))
	    return ptrClass;
    }

    return 0; // NULL
}

/**
 * get a class, which prefix belongs to
 *
 * @param iface
 * @param addr
 *
 * @return class (or 0 if no class is found)
 */
SmartPtr<TSrvCfgPD> TSrvCfgMgr::getClassByPrefix(int iface, SmartPtr<TIPv6Addr> addr)
{
    this->firstIface();
    SmartPtr<TSrvCfgIface> ptrIface;
    ptrIface = this->getIfaceByID(iface);

    if (!ptrIface) {
	Log(Error) << "Trying to find class on unknown (" << iface <<") interface." << LogEnd;
	return 0;
    }

    SmartPtr<TSrvCfgPD> ptrClass;
    ptrIface->firstPD();
    while (ptrClass = ptrIface->getPD()) {
	if (ptrClass->prefixInPool(addr))
	    return ptrClass;
    }

    return 0;
}


//on basis of duid/address/iface assign addresss to client
SmartPtr<TIPv6Addr> TSrvCfgMgr::getRandomAddr(SmartPtr<TDUID> clntDuid,
					      SmartPtr<TIPv6Addr> clntAddr,
					      int iface) {
    SmartPtr<TSrvCfgIface> ptrIface;
    SmartPtr<TSrvCfgAddrClass> ptrClass;
    SmartPtr<TIPv6Addr>    any;

    ptrIface = this->getIfaceByID(iface);
    if (!ptrIface) {
	return 0;
    }

    any = new TIPv6Addr();
    ptrIface->firstAddrClass();
    ptrClass = ptrIface->getAddrClass();
    return ptrClass->getRandomAddr();

    //FIXME: get addrs from first address only
}


/*
 * Method checks whether client is supported and assigned addresses from any class
 */
bool TSrvCfgMgr::isClntSupported(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> clntAddr, int iface)
{
    SmartPtr<TSrvCfgIface> ptrIface;
    firstIface();
    while((ptrIface=getIface())&&(ptrIface->getID()!=iface)) ;

    /* FIXME: reject-client and accept-only does not work in stateless mode */
    if (this->stateless())
	return true;

    int classCnt = 0;
    if (ptrIface)
    {
        SmartPtr<TSrvCfgAddrClass> ptrClass;
        ptrIface->firstAddrClass();
        while(ptrClass=ptrIface->getAddrClass()) {
            if (ptrClass->clntSupported(duid,clntAddr))
                return true;
	    classCnt++;
	}
    }
    if (!classCnt) {
	Log(Warning) << "There are no address class defined on the " << ptrIface->getFullName()
		     << ". Maybe you are trying to configure clients on cascade relay interface? "
		     << "If that is so, please define separate class on this interface, too." << LogEnd;
    }
    return false;
}


/*
* Method checks whether client is supported and assigned addresses from any class
*/
bool TSrvCfgMgr::isClntSupported(SmartPtr<TDUID> duid, SmartPtr<TIPv6Addr> clntAddr, int iface, SmartPtr<TSrvMsg> msg)
{
   SmartPtr<TSrvCfgIface> ptrIface;
   firstIface();
   while((ptrIface=getIface())&&(ptrIface->getID()!=iface)) ;

   /* FIXME: reject-client and accept-only does not work in stateless mode */
   if (this->stateless())
	return true;

   int classCnt = 0;
   if (ptrIface)
   {
       SmartPtr<TSrvCfgAddrClass> ptrClass;
       ptrIface->firstAddrClass();
       while(ptrClass=ptrIface->getAddrClass()) {
           if (ptrClass->clntSupported(duid,clntAddr,msg))
               return true;
	    classCnt++;
	}
   }
   if (!classCnt) {
	Log(Warning) << "There are no address class defined on the " << ptrIface->getFullName()
		     << ". Maybe you are trying to configure clients on cascade relay interface? "
		     << "If that is so, please define separate class on this interface, too." << LogEnd;
   }
   return false;
}

bool TSrvCfgMgr::isDone() {
    return IsDone;
}

bool TSrvCfgMgr::validateConfig() {
    SmartPtr<TSrvCfgIface> ptrIface;

    if (0 == (this->countIface() + this->inactiveIfacesCnt())) {
	Log(Crit) << "Config problem: No interface defined." << LogEnd;
	return false;
    }

    firstIface();
    while(ptrIface=getIface()) {
	if (!this->validateIface(ptrIface))
	    return false;
    }
    return true;
}

bool TSrvCfgMgr::validateIface(SmartPtr<TSrvCfgIface> ptrIface)
{
    bool dummyRelay = false;
    SmartPtr<TSrvIfaceIface> iface = (Ptr*)this->IfaceMgr->getIfaceByID(ptrIface->getID());
    if (iface && ptrIface->isRelay() && iface->getRelayCnt())
	dummyRelay = true;

    if (ptrIface->countAddrClass() && this->stateless()) {
	Log(Crit) << "Config problem: Interface " << ptrIface->getFullName()
		  << ": Class definitions present, but stateless mode set." << LogEnd;
	return false;
    }
    if (!ptrIface->countAddrClass() && !this->stateless()) {
	if (!dummyRelay) {
	    Log(Crit) << "Config problem: Interface " << ptrIface->getName() << "/" << ptrIface->getID()
		      << ": No class definitions present, but stateless mode not set." << LogEnd;
	    return false;
	} else {
	    Log(Warning) << "Interface " << ptrIface->getFullName() << " has no addrs defined, working as cascade relay interface."
			 << LogEnd;
	}
    }

    if (ptrIface->supportFQDN() && !ptrIface->supportDNSServer()) {
	Log(Crit) << "FQDN defined on the " << ptrIface->getFullName() << ", but no DNS servers defined."
		  << " Please disable FQDN support or add DNS servers." << LogEnd;
	return false;
    }

    SmartPtr<TSrvCfgAddrClass> ptrClass;
    ptrIface->firstAddrClass();
    while(ptrClass=ptrIface->getAddrClass()) {
	if (!this->validateClass(ptrIface, ptrClass)) {
	    Log(Crit) << "Config problem: Interface " << ptrIface->getName() << "/" << ptrIface->getID()
		      << ": Invalid class defined." << LogEnd;
	    return false;
	}
    }
    return true;
}

bool TSrvCfgMgr::validateClass(SmartPtr<TSrvCfgIface> ptrIface, SmartPtr<TSrvCfgAddrClass> ptrClass)
{
    if (ptrClass->isLinkLocal()) {
	Log(Crit) << "One of the classes defined on the " << ptrIface->getName() << "/" << ptrIface->getID()
		  << " overlaps with link local addresses." << LogEnd;
	return false;
    }

    if ( ptrClass->getPref(0) > ptrClass->getValid(0x7fffffff) )
    {
	Log(Crit) << "Prefered time max value (" <<ptrClass->getPref(0x7fffffff)
		  << ") can't be lower than valid time min. value (" << ptrClass->getValid(0)
		  << ") on the " << ptrIface->getName() << "/"
		  << ptrIface->getID() << " interface." << LogEnd;
	return false;
    }
    if ( ptrClass->getT1(0)>ptrClass->getT2(0x7fffffff) )
    {
	Log(Crit) << "T1 timeout upper bound (" <<ptrClass->getPref(0x7fffffff)
		  << " can't be lower than T2 lower bound (" << ptrClass->getT2(0)
		  << ") on the "<<ptrIface->getName()<<"/"
		  << ptrIface->getID() << " interface." << LogEnd;
	return false;
    }
    return true;
}

SmartPtr<TSrvCfgIface> TSrvCfgMgr::getIfaceByID(int iface) {
    SmartPtr<TSrvCfgIface> ptrIface;
    this->firstIface();
    while ( ptrIface = this->getIface() ) {
	if ( ptrIface->getID()==iface )
	    return ptrIface;
    }
    Log(Error) << "Invalid interface (id=" << iface
	       << ") specifed, cannot get address." << LogEnd;
    return 0; // NULL
}


void TSrvCfgMgr::delClntAddr(int iface, SmartPtr<TIPv6Addr> addr) {
    SmartPtr<TSrvCfgIface> ptrIface;
    ptrIface = this->getIfaceByID(iface);
    if (!ptrIface) {
	Log(Warning) << "Unable to decrease address usage: unknown interface (id="
		     << iface << ")" << LogEnd;
	return;
    }
    ptrIface->delClntAddr(addr);
}

void TSrvCfgMgr::addClntAddr(int iface, SmartPtr<TIPv6Addr> addr) {
    SmartPtr<TSrvCfgIface> ptrIface;
    ptrIface = this->getIfaceByID(iface);
    if (!ptrIface) {
	Log(Warning) << "Unable to increase address usage: unknown interface (id="
		     << iface << ")" << LogEnd;
	return;
    }
    ptrIface->addClntAddr(addr);
}

void TSrvCfgMgr::addTAAddr(int iface) {
    SmartPtr<TSrvCfgIface> ptrIface;
    ptrIface = this->getIfaceByID(iface);
    if (!ptrIface) {
	Log(Error) << "Unable to increase address usage: interface (id="
		   << iface << ") not found." << LogEnd;
	return;
    }
    ptrIface->addTAAddr();
}

void TSrvCfgMgr::delTAAddr(int iface) {
    SmartPtr<TSrvCfgIface> ptrIface;
    ptrIface = this->getIfaceByID(iface);
    if (!ptrIface) {
	Log(Error) << "Unable to decrease address usage: interface (id="
		   << iface << ") not found." << LogEnd;
	return;
    }
    ptrIface->delTAAddr();
}

bool TSrvCfgMgr::stateless() {
    return this->Stateless;
}

bool TSrvCfgMgr::inactiveMode()
{
    return InactiveMode;
}

bool TSrvCfgMgr::guessMode()
{
    return GuessMode;
}

bool TSrvCfgMgr::setupRelay(SmartPtr<TSrvCfgIface> cfgIface) {
    SmartPtr<TIfaceIface> iface;
    string name = cfgIface->getRelayName();

    iface = IfaceMgr->getIfaceByName(name);
    if (!iface) {
	Log(Crit) << "Underlaying interface for " << cfgIface->getName() << "/" << cfgIface->getID()
		  << " with name " << name << " is missing." << LogEnd;
	return false;
    }
    cfgIface->setRelayID(iface->getID());

    if (!IfaceMgr->setupRelay(cfgIface->getName(), cfgIface->getID(), iface->getID(), cfgIface->getRelayInterfaceID())) {
	Log(Crit) << "Relay setup for " << cfgIface->getName() << "/" << cfgIface->getID() << " interface failed." << LogEnd;
	return false;
    }

    return true;
}

/**
 * returns size (in bytes of the configured cache size
 *
 * @return
 */
int TSrvCfgMgr::getCacheSize() {
    return this->CacheSize;
}

ESrvIfaceIdOrder TSrvCfgMgr::getInterfaceIDOrder()
{
    return InterfaceIDOrder;
}

/**
 * decreases prefix usage count (i.e. decreases prefix-pool usage by one)
 * actual prefix is deleted in AddrMgr
 *
 * @param iface
 * @param prefix
 *
 * @return
 */
bool TSrvCfgMgr::decrPrefixCount(int ifindex, SPtr<TIPv6Addr> prefix)
{
    SmartPtr<TSrvCfgIface> iface;
    iface = getIfaceByID(ifindex);
    if (!iface) {
	Log(Error) << "Unable to find interface with ifindex=" << ifindex << ", prefix deletion aborted." << LogEnd;
	return false;
    }
    return iface->delClntPrefix(prefix);
}

bool TSrvCfgMgr::incrPrefixCount(int ifindex, SPtr<TIPv6Addr> prefix)
{
    SmartPtr<TSrvCfgIface> iface;
    iface = getIfaceByID(ifindex);
    if (!iface) {
	Log(Error) << "Unable to find interface with ifindex=" << ifindex << ", prefix increase count aborted." << LogEnd;
	return false;
    }
    return iface->addClntPrefix(prefix);
}

#ifndef MOD_DISABLE_AUTH
List(DigestTypes) TSrvCfgMgr::getDigestLst() {
    return this->DigestLst;
}

enum DigestTypes TSrvCfgMgr::getDigest() {
    SmartPtr<DigestTypes> dt;

    if (0 == DigestLst.count())
        return DIGEST_NONE;

    dt = DigestLst.getFirst();
    if (!dt || *dt >= DIGEST_INVALID)
        return DIGEST_NONE;

    return *dt;
}

unsigned int TSrvCfgMgr::getAuthLifetime()
{
    return AuthLifetime;
}

unsigned int TSrvCfgMgr::getAuthKeyGenNonceLen()
{
    return AuthKeyGenNonceLen;
}
#endif

// --------------------------------------------------------------------
// --- operators ------------------------------------------------------
// --------------------------------------------------------------------

ostream & operator<<(ostream &out, TSrvCfgMgr &x) {
    out << "<SrvCfgMgr>" << std::endl;
    out << "  <workDir>" << x.getWorkDir()  << "</workDir>" << endl;
    out << "  <LogName>" << x.getLogName()  << "</LogName>" << endl;
    out << "  <LogLevel>" << x.getLogLevel() << "</LogLevel>" << endl;
    out << "  <InactiveMode>" << (x.InactiveMode?1:0) << "</InactiveMode>" << endl;
    out << "  <GuessMode>" << (x.GuessMode?1:0) << "</GuessMode>" << endl;
    if (x.DUID)
	out << "  " << *x.DUID;
    else
	out << "  <!-- DUID not set -->" << std::endl;

    out << "  <InterfaceIDOrder>";
    switch (x.InterfaceIDOrder) {
    case SRV_IFACE_ID_ORDER_BEFORE:
	out << "before";
	break;
    case SRV_IFACE_ID_ORDER_AFTER:
	out << "after";
	break;
    case SRV_IFACE_ID_ORDER_NONE:
	out << "none";
	break;
    default:
	out << "unknown(" << x.InterfaceIDOrder << ")";
	break;
    }
    out << "</InterfaceIDOrder>" << endl;

#ifndef MOD_DISABLE_AUTH
    out << "  <auth count=\"" << x.DigestLst.count() << "\">";
    x.DigestLst.first();
    SPtr<DigestTypes> dig;
    while (dig=x.DigestLst.get()) {
	switch (*dig) {
	case DIGEST_NONE:
	    out << "digest-none ";
	    break;
	case DIGEST_HMAC_SHA1:
	    out << "digest-hmac-sha1";
	    break;
	default:
	    break;
	}
	out << "X";
    }

    out << "</auth>" << endl;
#endif

    if (x.stateless())
	out << "  <stateless/>" << std::endl;
    else
	out << "  <!-- <stateless/> -->" << std::endl;

    SmartPtr<TSrvCfgIface> ptrIface;
    x.firstIface();
    while (ptrIface = x.getIface()) {
	out << *ptrIface;
    }
    out << "</SrvCfgMgr>" << std::endl;
    return out;
}


void TSrvCfgMgr::InClientClass(SmartPtr<TSrvMsg> msg)
{
	// For each client class, check whether the message belong to ClientClass
	ClientClassLst.first();
	SmartPtr<TSrvCfgClientClass> clntClass;

	while( clntClass= ClientClassLst.get() )
	{
	    Log(Debug) << "Checking if client belongs to " << clntClass->getClassName() << " client class";
		if (clntClass->isStatisfy(msg))
			Log(Cont)<<" ... yes." << LogEnd;
		else
			Log(Cont)<<" ... no." << LogEnd;
	}
}


