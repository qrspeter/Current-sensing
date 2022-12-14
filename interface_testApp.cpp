/***************************************************************
 * Name:      interface_testApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Peter ()
 * Created:   2022-03-14
 * Copyright: Peter ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "interface_testApp.h"
#include "interface_testMain.h"

#include "sensor_fet/sensor_fet.h"

IMPLEMENT_APP(interface_testApp);

SENSOR_FET sensor;

bool interface_testApp::OnInit()
{
    interface_testFrame* frame = new interface_testFrame(0L, _("Test sensor interface"));
    frame->SetIcon(wxICON(aaaa)); // To Set App Icon
    frame->SetSize( 1000, 720);
    frame->Show();

    return true;
}
