/***************************************************************
 * Name:      current_sensingApp.cpp
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

#include "current_sensingApp.h"
#include "current_sensingMain.h"

#include "sensor_fet/sensor_fet.h"

IMPLEMENT_APP(current_sensingApp);


bool current_sensingApp::OnInit()
{
    current_sensingFrame* frame = new current_sensingFrame(0L, _("Test sensor interface"));
    frame->SetIcon(wxICON(aaaa)); // To Set App Icon
    frame->SetSize( 1000, 750);
    frame->Show();

    return true;
}
