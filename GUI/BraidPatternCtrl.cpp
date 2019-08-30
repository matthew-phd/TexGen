#include "PrecompiledHeaders.h"
#include "BraidPatternCtrl.h"

BEGIN_EVENT_TABLE(wxBraidPatternCtrl, wxControl)
	EVT_PAINT(wxBraidPatternCtrl::OnPaint)
	EVT_MOUSE_EVENTS(wxBraidPatternCtrl::OnMouseEvent)
	EVT_SIZE(wxBraidPatternCtrl::OnSize)
	EVT_MENU(wxID_ANY, wxBraidPatternCtrl::onMenu)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(wxBraidPatternCtrl, wxContorl)

DEFINE_EVENT_TYPE(EVT_CROSSOVER_CLICKED)

IMPLEMENT_DYNAMIC_CLASS(wxBriadPatternCtrlEvent, wxCommandEvent)

wxBraidPatternCtrl::wxBraidPatternCtrl()
{
	Init();
}

wxBraidPatternCtrl::wxBraidPatternCtrl(wxWindow* parent, wxWindow id, const wxPoint& pos = wxDefaultPosition,
	const wxSize& size = wxDefaultSize, long stype = wxSUNKEN_BORDER | wxFULL_REPAINT_ON_RESIZE,
	const wxValidator& validator = wxDefaultValidator)
{
	Init();
	Create(parent, id, pos, size, style, validator);
	SetBackgroundStyle(wxBG_STYLE_CUSTOM)
}

wxBraidPatternCtrl::~wxBraidPatternCtrl()
{

}
