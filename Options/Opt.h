/*                                                                           
 * Dibbler - a portable DHCPv6                                               
 *                                                                           
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>                           
 *          Marek Senderski <msend@o2.pl>                                    
 *                                                                           
 * released under GNU GPL v2 only licence                                
 *                                                                           
 * $Id: Opt.h,v 1.11 2008-08-29 00:07:30 thomson Exp $
 *
 * $Log: Opt.h,v $
 * Revision 1.11  2008-08-29 00:07:30  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.10  2007-12-02 10:31:59  thomson
 * Requestor implemented (RFC5007)
 *
 * Revision 1.9  2007-03-28 00:39:46  thomson
 * Clnt: REQUEST now contains addresses offered in ADVERTISE (bug #152)
 *
 * Revision 1.8  2006-11-17 00:37:16  thomson
 * Partial AUTH support by Sammael, fixes by thomson
 *
 * Revision 1.7  2006-03-03 20:25:15  thomson
 * Comment added.
 *
 * Revision 1.6  2006/01/12 00:23:35  thomson
 * Cleanup changes. Now -pedantic option works.
 *
 * Revision 1.5  2004/10/27 22:07:56  thomson
 * Signed/unsigned issues fixed, Lifetime option implemented, INFORMATION-REQUEST
 * message is now sent properly. Valid lifetime granted by server fixed.
 *
 * Revision 1.4  2004/05/23 16:40:12  thomson
 * *** empty log message ***
 *
 *                                                                           
 */

#ifndef OPT_H
#define OPT_H

#include "SmartPtr.h"
#include "Container.h"
#include "DUID.h"

class TMsg;

class TOpt
{
  public:
    TOpt(int optType, TMsg* parent);
    virtual ~TOpt();

	/**
	 * Return the size of the option, including :
	 *  - Option number,
	 *  - Option size
	 *  - data
	 * 
	 * @return the size, I've already said that !
	 */
    virtual int getSize() = 0;
    
    /**
     * This method transform the instance of the option class into bytecode
     * ready to be sent to the client or server, conform to the RFC
     * 
     * @param buf The address where to store the option
     * @return The address where the option ends
     */
    virtual char * storeSelf(char* buf) = 0;
    virtual bool doDuties() = 0;
    
    /**
     * Validate the option
     * 
     * @return true if the option is valid.
     */
    virtual bool isValid();
    
    int getOptType();
    int getSubOptSize();
    
    char* storeSubOpt(char* buf);
    SmartPtr<TOpt> getOption(int optType);

    // suboptions management
    void firstOption();
    SmartPtr<TOpt> getOption();
    void addOption(SmartPtr<TOpt> opt);
    int countOption();
    void delAllOptions();
    void setParent(TMsg* Parent);

    SmartPtr<TDUID> getDUID();
    void setDUID(SmartPtr<TDUID> duid);

 protected:
    TContainer< SmartPtr<TOpt> > SubOptions;
    int OptType;
    TMsg* Parent;
    SmartPtr<TDUID> DUID;
};

#endif
