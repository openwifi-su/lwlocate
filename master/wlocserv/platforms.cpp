/**
 * Copyright (C) 2004-2012 Oxygenic/VWP virtual_worlds(at)gmx.de
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "liboapc.h"
#include "platforms.h"

static char    leave=false;

#ifdef ENV_WINDOWS

#include <process.h>
#include "Commctrl.h"



void showLog(const char *format,...)
{
va_list  arglist;
char     sText[1500];

va_start(arglist,format);
vsprintf(sText,format,arglist);
va_end(arglist);
printf("%s\r\n",sText);
}



int splitCmdLine(unsigned short *lpCmdLine,char *argv[])
{
int          ctr=1;
unsigned int i;
char        *c,*cprev,*cstart;
char        *cCmdLine;

cCmdLine=(char*)malloc(wcslen(lpCmdLine)+1);
if (!cCmdLine) return 0;
memset(cCmdLine,0,wcslen(lpCmdLine)+1);
for (i=0; i<wcslen(lpCmdLine); i++)
{
   cCmdLine[i]=(char)lpCmdLine[i];
}
if (strlen(cCmdLine)<2) return 0;
c=cCmdLine+1;
cprev=cCmdLine;
cstart=cCmdLine;
while (*c!=0)
   {
   if ((*c==' ') && (*cprev!='\\'))
      {
      *c=0;
      argv[ctr]=cstart;
      ctr++;
      cstart=c+1;
      }
   c++;
   cprev++;
   }
argv[ctr]=cstart;
free(cCmdLine);
return ctr;
}


bool LeaveServer(void)
{
   return false;
//TODO: implement end condition here
}



#else // ENV_WINDOWS


#ifndef ENV_QNX
 #include <sys/signal.h>
#else
 #include <netdb.h>
#endif
#include <signal.h>
#include <errno.h>
#include <pthread.h>

static char           quit=false;



bool LeaveServer(void)
{
	return quit;
}



void signalh (const int sig)
{
//  setup_signal_traps ();
switch (sig) 
   {
   case SIGPIPE:
   case EPIPE:
      printf("EPIPE received\n");
      break;
   case SIGHUP:
//      reloadSignal=true;
      break;
   case SIGBUS:
   case SIGFPE:
   case SIGIO:
   case SIGTERM:
      leave=true;
      break;
   case SIGINT:
   case SIGQUIT:
      printf ("\nexiting\n");
      quit=1;
      break;
   }
}



void setupSignalTraps(void)
{
   signal (SIGINT, signalh);
   signal (SIGQUIT, signalh);
   signal (SIGFPE, signalh);
   signal (SIGBUS, signalh);
   signal (SIGTERM, signalh);
   signal (SIGHUP, signalh);
   signal (SIGIO, signalh);
   #ifdef ENV_SPARCSOLARIS
   signal (EPIPE, signalh);
   signal (SIGPIPE, SIG_IGN);
   #else
   signal (SIGPIPE, signalh);
   #endif
}



void showLog(const char *format,...)
{
va_list  arglist;
char     sText[1500];

va_start(arglist,format);
vsprintf(sText,format,arglist);
va_end(arglist);
sText[strlen(sText)]='\0';
printf("%s\n",sText);
}

#endif


