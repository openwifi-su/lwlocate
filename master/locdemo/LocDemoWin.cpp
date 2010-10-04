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

#include <list>

#include "LocDemoWin.h"

#include <wx/sstream.h>
#include <wx/protocol/http.h>
#include <wx/file.h>
#include <wx/sizer.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/timer.h>

#include "libwlocate.h"

using namespace std;

#define X_OFFSET 162


IMPLEMENT_CLASS(LocDemoWin, wxFrame)

BEGIN_EVENT_TABLE(LocDemoWin, wxFrame)
  EVT_BUTTON(wxID_ANY,LocDemoWin::OnButton)
  EVT_PAINT(LocDemoWin::OnPaint) 
  EVT_TIMER(1,LocDemoWin::OnTimer)
END_EVENT_TABLE()



LocDemoWin::LocDemoWin(const wxString& title)
           :wxFrame(NULL, wxID_ANY, title, wxPoint(20,0), wxSize(930,768),wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxSYSTEM_MENU|wxCAPTION|wxCLOSE_BOX|wxCLIP_CHILDREN)
{
   wxInt32  x,y;
   
   for (x=-1; x<=1; x++)
    for (y=-1; y<=1; y++) locTile[x+1][y+1]=NULL;
   
   m_zoom=17;

   SetBackgroundColour(*wxWHITE);
   wxFlexGridSizer *fSizer=new wxFlexGridSizer(1,2,2,2);
   this->SetSizer(fSizer);
   fSizer->AddGrowableCol(0,1);
   fSizer->AddGrowableCol(1,10);
   wxPanel *rootPanel=new wxPanel(this);//,WXID_ANY,wxDefaultPos,wxSize(X_OFFSET,750));
   fSizer->Add(rootPanel);

   wxGridSizer *gSizer=new wxGridSizer(12,1,2,2);
   rootPanel->SetBackgroundColour(*wxWHITE);
   rootPanel->SetSizer(gSizer);

   wxStaticText *text=new wxStaticText(rootPanel,wxID_ANY,_T(""));
   gSizer->Add(text,0,wxALIGN_LEFT);
   updateButton=new wxButton(rootPanel,wxID_ANY,_T("Update Position"));
   gSizer->Add(updateButton,1,wxEXPAND);

   text=new wxStaticText(rootPanel,wxID_ANY,_T("Latitude:"));
   gSizer->Add(text,0,wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
   m_latField=new wxTextCtrl(rootPanel,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
   gSizer->Add(m_latField,1,wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
   
   text=new wxStaticText(rootPanel,wxID_ANY,_T("Longitude:"));
   gSizer->Add(text,0,wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
   m_lonField=new wxTextCtrl(rootPanel,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
   gSizer->Add(m_lonField,1,wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
   
   text=new wxStaticText(rootPanel,wxID_ANY,_T("Quality:"));
   gSizer->Add(text,0,wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
   m_qualityField=new wxTextCtrl(rootPanel,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
   gSizer->Add(m_qualityField,1,wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);

   text=new wxStaticText(rootPanel,wxID_ANY,_T("Country:"));
   gSizer->Add(text,0,wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
   m_countryField=new wxTextCtrl(rootPanel,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
   gSizer->Add(m_countryField,1,wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);

   m_followPathCB=new wxCheckBox(rootPanel,wxID_ANY,_T("Update cyclically"));
   gSizer->Add(m_followPathCB,1,wxEXPAND);

   zoomInButton=new wxButton(rootPanel,wxID_ANY,_T("Zoom In"));
   gSizer->Add(zoomInButton,1,wxEXPAND);
   zoomOutButton=new wxButton(rootPanel,wxID_ANY,_T("Zoom Out"));
   gSizer->Add(zoomOutButton,1,wxEXPAND);

   infoButton=new wxButton(rootPanel,wxID_ANY,_T("About"));
   gSizer->Add(infoButton,1,wxEXPAND);
   
   SetDoubleBuffered(true);

   getLocation(false);

   m_timer = new wxTimer(this,1);
   m_timer->Start(8000);
}


LocDemoWin::~LocDemoWin()
{
   wxInt32 x,y;

   for (x=-1; x<=1; x++)
    for (y=-1; y<=1; y++) if (locTile[x+1][y+1]) delete locTile[x+1][y+1];
}

void LocDemoWin::OnTimer(wxTimerEvent& WXUNUSED(event))
{
   if (m_followPathCB->GetValue())
   {
      m_timer->Stop();
      getLocation(true);
      Refresh();
      m_timer->Start();
   }
}



int long2tilex(double lon, int z) 
{ 
	return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z))); 
}
 
int
 lat2tiley(double lat, int z)
{ 
	return (int)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z))); 
}
 
double tilex2long(int x, int z) 
{
	return x / pow(2.0, z) * 360.0 - 180;
}
 
double tiley2lat(int y, int z) 
{
	double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}



void LocDemoWin::OnPaint(wxPaintEvent& WXUNUSED(event))
{
   wxInt32                x,y;
   double                 tileLat1,tileLon1,tileLat2,tileLon2;
   wxPaintDC              dc(this); 
   wxPen                  borderPen(*wxRED,3);
   wxPen                  pathPen(*wxBLUE,2);
   list<double>::iterator itLat,itLon;

   for (x=-1; x<=1; x++)
    for (y=-1; y<=1; y++)
   {
      if (locTile[x+1][y+1])
      {
         dc.DrawBitmap(*locTile[x+1][y+1],((x+1)*256)+X_OFFSET,(y+1)*256,false);
      }
   }

   tileLat1=tiley2lat(m_tileY,m_zoom);
   tileLat2=tiley2lat(m_tileY+1,m_zoom);	
   tileLon1=tilex2long(m_tileX,m_zoom);
   tileLon2=tilex2long(m_tileX+1,m_zoom);

   dc.SetBrush(*wxTRANSPARENT_BRUSH);
   if (m_latList.size()>1)
   {
      double currY,currX,prevY,prevX;

      dc.SetPen(pathPen);
      itLat=m_latList.begin(); itLat++;
      itLon=m_lonList.begin(); itLon++;
      prevY=256+         (256.0*(*itLat-tileLat1)/(tileLat2-tileLat1));
      prevX=256+X_OFFSET+(256.0*(*itLon-tileLon1)/(tileLon2-tileLon1));
      for ( ; itLat!= m_latList.end(); itLat++ )
      {
         currY=256+         (256.0*(*itLat-tileLat1)/(tileLat2-tileLat1));
         currX=256+X_OFFSET+(256.0*(*itLon-tileLon1)/(tileLon2-tileLon1));
         dc.DrawLine(prevX,prevY,currX,currY);
         prevY=currY;
         prevX=currX;
         itLon++;
      }
   }
   
   y=256+         (256.0*(m_lat-tileLat1)/(tileLat2-tileLat1));
   x=256+X_OFFSET+(256.0*(m_lon-tileLon1)/(tileLon2-tileLon1));

   if ((x!=0) && (y!=0))
   {
      wxFloat64 zoomFactor;

      dc.SetPen(borderPen);
      if (m_quality>0)
      {
         zoomFactor=pow(2.0,(17.0-m_zoom));
         dc.DrawCircle(x,y,(120-m_quality)/zoomFactor);
      }
      else
      {
         zoomFactor=pow(2.0,(10.0-m_zoom));
         dc.DrawCircle(x,y,130/zoomFactor);
      }
   }
}



void LocDemoWin::updateTiles(wxFloat64 lat,wxFloat64 lon)
{
	wxInt32        x,y;
   wxHTTP        *get;
   wxString       path;
   wxInputStream *httpStream;
   wxImage       *tmpImage;

   m_tileX=long2tilex(lon,m_zoom);
   m_tileY=lat2tiley(lat,m_zoom);

   for (x=-1; x<=1; x++)
    for (y=-1; y<=1; y++)
   {
      if (locTile[x+1][y+1]) delete locTile[x+1][y+1];
      locTile[x+1][y+1]=NULL;

      if (!wxFile::Exists(wxStandardPaths::Get().GetUserDataDir()+wxFileName::GetPathSeparator()+wxString::Format(_T("tile_%d_%d_%d.png"),m_zoom,m_tileX+x,m_tileY+y)))
      {
      	get=new wxHTTP();
         get->SetTimeout(10); // 10 seconds of timeout instead of 10 minutes ...
         get->SetHeader(_T("User-Agent"),_T("LocDemo libwlocate demo application"));
         while (!get->Connect(_T("tah.openstreetmap.org")))
          wxSleep(5);
 
         path=wxString::Format(_T("/Tiles/tile/%d/%d/%d.png"),m_zoom,m_tileX+x,m_tileY+y);
         httpStream = get->GetInputStream(path);
         if (get->GetError() == wxPROTO_NOERR)
         {
            wxFile  *FHandle;

            wxFileName::Mkdir(wxStandardPaths::Get().GetUserDataDir());
            path=wxStandardPaths::Get().GetUserDataDir()+wxFileName::GetPathSeparator()+wxString::Format(_T("tile_%d_%d_%d.png"),m_zoom,m_tileX+x,m_tileY+y);
            FHandle=new wxFile(path,wxFile::write);
            if ((FHandle) && (FHandle->IsOpened()))
            {
            	void *mem;
         
               mem=malloc(httpStream->GetSize());
               if (mem)
               {
            	   httpStream->Read(mem,httpStream->GetSize());
               	FHandle->Write(mem,httpStream->GetSize());
               	delete FHandle;
                  free(mem);
               }
            } 
            wxDELETE(httpStream);
         }
         get->Close();
         delete get;
      }
      path=wxStandardPaths::Get().GetUserDataDir()+wxFileName::GetPathSeparator()+wxString::Format(_T("tile_%d_%d_%d.png"),m_zoom,m_tileX+x,m_tileY+y);
      tmpImage=new wxImage(path,wxBITMAP_TYPE_PNG);
      if ((tmpImage) && (tmpImage->Ok()))
      {
       	locTile[x+1][y+1]=new wxBitmap(*tmpImage);
  	      delete tmpImage;
      }         
   }
}



void LocDemoWin::getLocation(bool silent)
{
   wxInt32  ret;
   wxFrame *splash=NULL;
   
   if (!silent)
   {
      splash = new wxFrame(NULL,wxID_ANY,_T("Getting Position Data and Map"),wxDefaultPosition,wxSize(500,80),wxSTAY_ON_TOP|wxFRAME_NO_TASKBAR|wxCAPTION);
      new wxStaticText(splash,wxID_ANY,_T("Operation in progress, please wait..."),wxPoint(20,30));
      splash->Center();
#ifndef _DEBUG
   splash->Show(); 
#endif
   }

   m_lat=0;
   m_lon=0;
   m_quality=0;
   ret=wloc_get_location(&m_lat,&m_lon,&m_quality,&m_ccode);
   if (ret==WLOC_OK)
   {
      char country[3]={0,0,0};
      wxMBConvUTF8  conv;
      wchar_t       wc[3];

      m_latField->SetValue(_T("")); *m_latField<<m_lat; 
      m_lonField->SetValue(_T("")); *m_lonField<<m_lon; 
      if (m_quality>0)
      {
         m_latList.push_back(m_lat);
         m_lonList.push_back(m_lon);
      }
      
      m_qualityField->SetValue(_T("")); *m_qualityField<<m_quality;
      if (!m_followPathCB->GetValue())
      {
         if (m_quality==0) m_zoom=10;
         else m_zoom=17;
      }
      if (wloc_get_country_from_code(m_ccode,country)==WLOC_OK)
      {
         conv.MB2WC(wc,country,3);
         m_countryField->SetValue(_T(""));  *m_countryField<<wc;
      }
      else m_countryField->SetValue(_T("?"));

      updateTiles(m_lat,m_lon);
      if (!silent) splash->Close();
      delete splash;
   }
   else
   {
      m_latField->SetValue(_T("---"));
      m_lonField->SetValue(_T("---"));
      m_qualityField->SetValue(_T("---"));
      m_countryField->SetValue(_T("--"));
      splash->Close();
      delete splash;
      if (ret==WLOC_CONNECTION_ERROR)
       wxMessageBox(_T("Could not connect to server and retrieve data!"),_T("Error"),wxOK|wxICON_ERROR);
      else if (ret==WLOC_LOCATION_ERROR)
       wxMessageBox(_T("Could not retrieve location, no data available for your position!"),_T("Error"),wxOK|wxICON_ERROR);
      else   
       wxMessageBox(_T("Could not retrieve location, an unidentified error happened :-("),_T("Error"),wxOK|wxICON_ERROR);
   }
}


void LocDemoWin::OnButton(wxCommandEvent &event)
{
   if (event.GetId()==updateButton->GetId())
   {
      getLocation(false);
      Refresh();
   }
   else if (event.GetId()==zoomOutButton->GetId())
   {
      if (m_zoom>2) m_zoom--;
      updateTiles(m_lat,m_lon);
      Refresh();
   }
   else if (event.GetId()==zoomInButton->GetId())
   {
      if (m_zoom<17) m_zoom++;
      updateTiles(m_lat,m_lon);
      Refresh();
   }
   else if (event.GetId()==infoButton->GetId()) wxMessageBox(_T("LocDemo Version 0.6 is (c) 2010 by Oxy/VWP\nIt demonstrates the usage of libwlocate and is available under the terms of the GNU Public License\nFor more details please refer to http://www.openwlanmap.org"),_T("Information"),wxOK|wxICON_INFORMATION);
}

