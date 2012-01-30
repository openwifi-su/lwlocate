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

#ifndef MAINDEFS_H
#define MAINDEFS_H

#define FCOMMON_NAME      "WLocate Server"
#define FCOMMON_VERSION   0.2
#define FCOMMON_URL       ""
#define FCOMMON_COPYRIGHT "(c) 2009-2012"

#ifdef ENV_LINUX
#define ENV_POSIX
#endif

#ifdef ENV_QNX
#define ENV_POSIX
#endif

#ifdef ENV_SPARCSOLARIS
#define ENV_POSIX
#endif

#define ERR_NO_MEMORY       0x20010000
#define ERR_LIST_EMPTY      0x20020000
#define ERR_NO_SUCH_ELEMENT 0x20030000

#endif
