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

class LocDemoWin : public wxFrame
{
public:
    LocDemoWin(const wxString& title);
    virtual ~LocDemoWin();
    
private:
    void        OnButton(wxCommandEvent &event);
    void        OnPaint(wxPaintEvent& event);
    void        updateTiles(wxFloat64 lat,wxFloat64 lon);
    void        getLocation(void);
    
    wxButton   *updateButton,*infoButton;
    wxBitmap   *locTile[3][3];
    wxInt32     m_tileX,m_tileY;
    wxTextCtrl *m_latField,*m_lonField,*m_qualityField,*m_countryField;
    double      m_lat,m_lon;
    char        m_quality;
    short       m_ccode;
    wxByte      m_zoom;
    
    DECLARE_CLASS(LocDemoWin)
    DECLARE_EVENT_TABLE()
};

