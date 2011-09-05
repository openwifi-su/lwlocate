/**
 * LocDemo - a demo GUI application that uses libwlocate to display the
 * current geographic position
 * Copyright (C) 2010 Oxygenic/VWP virtual_worlds(at)gmx.de
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

#include "LocDemoApp.h"
#include "LocDemoWin.h"

#include <wx/socket.h>



IMPLEMENT_APP(LocDemoApp)



bool LocDemoApp::OnInit()
{
   LocDemoWin *myWin;

   wxLog::EnableLogging(false);
   wxInitAllImageHandlers();
   wxSocketBase::Initialize();

   myWin = new LocDemoWin(wxT("OpenWLANMap Location Demo"));
   if (!wxApp::OnInit()) return false;
   myWin->Show(true);
   return true;
}



