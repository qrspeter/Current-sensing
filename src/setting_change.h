#ifndef SETTING_CHANGE_H_INCLUDED
#define SETTING_CHANGE_H_INCLUDED

class setting_change: public wxFrame
{
    public:
        setting_change(wxFrame *frame, const wxString& title);
        ~setting_change();
    private:
        enum
        {

        };

 //       wxButton* setting_OK;


        DECLARE_EVENT_TABLE()
};



#endif // SETTING_CHANGE_H_INCLUDED
