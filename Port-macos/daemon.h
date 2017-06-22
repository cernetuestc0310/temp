/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 * changes: Michal Kowalczuk <michal@kowalczuk.eu>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: daemon.h,v 1.2 2008-08-29 00:07:31 thomson Exp $
 *
 * $Log: daemon.h,v $
 * Revision 1.2  2008-08-29 00:07:31  thomson
 * Temporary license change(GPLv2 or later -> GPLv2 only)
 *
 * Revision 1.1  2008-07-20 15:54:36  thomson
 * Initial Mac OS port.
 *
 * Revision 1.4  2008-02-25 17:49:09  thomson
 * Authentication added. Megapatch by Michal Kowalczuk.
 * (small changes by Tomasz Mrugalski)
 *
 * Revision 1.3  2006-11-30 03:17:23  thomson
 * Auth related changes by Sammael.
 *
 * Revision 1.2  2006-11-24 01:25:17  thomson
 * consts added by Sammael.
 *
 * Revision 1.1  2005-02-03 22:06:40  thomson
 * Linux startup/pid checking changed.
 *
 */

#ifndef DAEMON_H
#define DAEMON_H

#ifndef SIGTERM
#define SIGTERM 15
#endif

#ifndef SIGINT
#define SIGINT 2
#endif

int start(const char * pidfile, const char * workdir);
int stop(const char * pidfile);

int init(const char * pidfile, const char * workdir);
int getPID(char * pidfile);
int getServerPID();
int getClientPID();
int getRelayPID();
int die(const char * pidfile);

void logStart(const char * note, const char * logname, const char * logfile);
void logEnd();

#endif
