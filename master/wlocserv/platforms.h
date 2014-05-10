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

#ifndef PLATFORMS_H
#define PLATFORMS_H


#ifdef ENV_WINDOWS
 #include <windows.h>

 #define MSG_NOSIGNAL 0
#endif

#ifdef ENV_LINUX
typedef void* HINSTANCE;

extern bool reloadSignal;
#endif

#ifdef ENV_QNX
typedef void* HINSTANCE;
#endif

void showLog(const char *format,...);
bool LeaveServer(void);

#ifndef ENV_WINDOWS
void setupSignalTraps();
#else
int splitCmdLine(unsigned short *lpCmdLine,char *argv[]);
#endif

#endif // PLATFORMS_H
