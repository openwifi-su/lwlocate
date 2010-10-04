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

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <list>

class LocDemoWin : public wxFrame
{
public:
    LocDemoWin(const wxString& title);
    virtual ~LocDemoWin();
    
private:
    void        OnButton(wxCommandEvent &event);
    void        OnPaint(wxPaintEvent& event);
    void        OnTimer(wxTimerEvent& event);
    void        updateTiles(wxFloat64 lat,wxFloat64 lon);
    void        getLocation(bool silent);
    
    wxButton         *updateButton,*infoButton,*zoomInButton,*zoomOutButton;
    wxBitmap         *locTile[3][3];
    wxInt32           m_tileX,m_tileY;
    wxTextCtrl       *m_latField,*m_lonField,*m_qualityField,*m_countryField;
    wxCheckBox       *m_followPathCB;
    double            m_lat,m_lon;
    char              m_quality;
    short             m_ccode;
    wxByte            m_zoom;
    wxTimer          *m_timer;
    std::list<double> m_latList,m_lonList;
    
    DECLARE_CLASS(LocDemoWin)
    DECLARE_EVENT_TABLE()
};

