/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: dibbler-relay.cpp,v 1.13 2008-10-10 20:39:12 thomson Exp $
 *
 * $Log: dibbler-relay.cpp,v $
 * Revision 1.13  2008-10-10 20:39:12  thomson
 * pid_t is now used instead of int (bug #180)
 *
 * Revision 1.12  2008-08-29 00:07:31  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.11  2008-06-01 21:45:48  thomson
 * Return coded are now returned properly under Linux (bug #175)
 *
 * Revision 1.10  2005-10-11 20:52:47  thomson
 * Problem with command-line parsing fixed.
 *
 * Revision 1.9  2005/07/31 14:39:40  thomson
 * Minor changes related to 0.4.1 release.
 *
 * Revision 1.8  2005/07/21 21:40:19  thomson
 * PID checking process is improved.
 *
 * Revision 1.7  2005/04/28 21:20:52  thomson
 * Support for multiple relays added.
 *
 * Revision 1.6  2005/02/03 22:42:25  thomson
 * *** empty log message ***
 *
 * Revision 1.5  2005/02/03 22:06:40  thomson
 * Linux startup/pid checking changed.
 *
 * Revision 1.4  2005/02/03 20:09:11  thomson
 * *** empty log message ***
 *
 * Revision 1.3  2005/01/30 23:12:28  thomson
 * *** empty log message ***
 *
 * Revision 1.2  2005/01/30 22:53:28  thomson
 * *** empty log message ***
 *
 * Revision 1.1  2005/01/11 22:53:35  thomson
 * Relay skeleton implemented.
 *
 */

#include <signal.h>
#include "DHCPRelay.h"
#include "Portable.h"
#include "Logger.h"
#include "daemon.h"

TDHCPRelay * ptr;

void signal_handler(int n) {
    Log(Crit) << "Signal received. Shutting down." << LogEnd;
    ptr->stop();
}

int status() {
    pid_t pid = getServerPID();
    if (pid==-1) {
	cout << "Dibbler server: NOT RUNNING." << endl;
    } else {
	cout << "Dibbler server: RUNNING, pid=" << pid << endl;
    }
    
    pid = getClientPID();
    if (pid==-1) {
	cout << "Dibbler client: NOT RUNNING." << endl;
    } else {
	cout << "Dibbler client: RUNNING, pid=" << pid << endl;
    }

    pid = getRelayPID();
    if (pid==-1) {
	cout << "Dibbler relay : NOT RUNNING." << endl;
    } else {
	cout << "Dibbler relay : RUNNING, pid=" << pid << endl;
    }

    return (pid>0)? 0: -1;
}

int run() {
    if (!init(RELPID_FILE, WORKDIR)) {
	return -1;
    }

    TDHCPRelay relay(RELCONF_FILE);
    
    ptr = &relay;
    if (ptr->isDone()) {
	return -1;
    }
    
    // connect signals
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    
    ptr->run();
    
    die(RELPID_FILE);
    return 0;
}

int help() {
    cout << "Usage:" << endl;
    cout << " dibbler-relay ACTION" << endl
	 << " ACTION = status|start|stop|install|uninstall|run" << endl
	 << " status    - show status and exit" << endl
	 << " start     - start installed service" << endl
	 << " stop      - stop installed service" << endl
	 << " install   - Not available in Linux/Unix systems." << endl
	 << " uninstall - Not available in Linux/Unix systems." << endl
	 << " run       - run in the console" << endl
	 << " help      - displays usage info." << endl;
    return 0;
}

int main(int argc, char * argv[])
{
    char command[256];
    int result=-1;
    int len;

    logStart("(RELAY, Linux port)", "Relay", RELLOG_FILE);

    // parse command line parameters
    if (argc>1) {
	len = strlen(argv[1])+1;
	if (len>255)
	    len = 255;
	strncpy(command,argv[1],len);
    } else {
	memset(command,0,256);
    }

    if (!strncasecmp(command,"start",5) ) {
	result = start(RELPID_FILE, WORKDIR);
    } else
    if (!strncasecmp(command,"run",3) ) {
	result = run();
    } else
    if (!strncasecmp(command,"stop",4)) {
	result = stop(RELPID_FILE);
    } else
    if (!strncasecmp(command,"status",6)) {
	result = status();
    } else
    if (!strncasecmp(command,"help",4)) {
	result = help();
    } else
    if (!strncasecmp(command,"install",7)) {
	cout << "Function not available in Linux/Unix systems." << endl;
	result = 0;
    } else
    if (!strncasecmp(command,"uninstall",9)) {
	cout << "Function not available in Linux/Unix systems." << endl;
	result = 0;
    } else
    {
	help();
    }

    logEnd();

    return result? EXIT_FAILURE: EXIT_SUCCESS;
}
