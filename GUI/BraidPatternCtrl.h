
#pragma once

class wxBraidPatternCtrl : public wxControl
{
	DECLARE_DYNAMIC_CLASS(wxBraidPatternCtrl)
	DECLARE_EVENT_TABLE()

public:
	wxBraidPatternCtrl();
	wxBraidPatternCtrl(wxWindow* parent, wxWindow id, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long stype = wxSUNKEN_BORDER | wxFULL_REPAINT_ON_RESIZE,
		const wxValidator& validator = wxDefaultValidator);
	~wxBraidPatternCtrl();
};