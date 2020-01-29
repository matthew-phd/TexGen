#pragma once
//(c) Copyright Joe Eastwood 2019 - All rights reserved

/// Weave pattern control used for setting the weave pattern
class wxBraidPatternCtrl : public wxControl
{
	DECLARE_DYNAMIC_CLASS(wxBraidPatternCtrl)
	DECLARE_EVENT_TABLE()

public:
	// Constructors
	wxBraidPatternCtrl();
	wxBraidPatternCtrl(wxWindow* parent, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxSUNKEN_BORDER | wxFULL_REPAINT_ON_RESIZE,
		const wxValidator& validator = wxDefaultValidator);
	~wxBraidPatternCtrl();
	// Creation
	bool Create(wxWindow* parent, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxSUNKEN_BORDER | wxFULL_REPAINT_ON_RESIZE,
		const wxValidator& validator = wxDefaultValidator);

	/// Sets the mode to 3d, this must be called before set weave size
	void SetBraidSize(int iNumYYarns, int iNumXYarns, bool b3DMode);
	void SetBraidSize(const CTextileBraid &Braid);
	int GetBraidWidth();
	int GetBraidHeight();
	bool GetCellStatus(int i, int j);
	//	void SetCellStatus(int i, int j, bool bVal);
	bool bHasBraid() { return m_BraidPattern ? true : false; }
	CTextileBraid &GetBraid() { return *m_BraidPattern; }
	//bool Get3DMode() { return m_b3DMode; }

protected:
	enum
	{
		MENUID_ADDLAYER = 1,
		MENUID_REMOVELAYER,
		MENUID_SETYARNWIDTH,
		MENUID_SETYARNHEIGHT,
		MENUID_SETYARNSPACING,
	};

	struct YARN_POS_DATA
	{
		int iCellMin, iYarnMin, iYarnCenter, iYarnMax, iCellMax;
	};

	// Overrides
	wxSize DoGetBestSize() const { return wxSize(600, 400); }
	// Event handlers
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnMenu(wxCommandEvent& event);

	void Init();
	pair<int, int> GetCellIndices(wxPoint Position);
	pair<int, int> GetCellHeight(wxPoint Position);
	bool HandleIconSelect(wxPoint Position);
	void HandleIconSelect(wxRect MarqueeSelect);
	wxRect GetLeftIconsRegion();
	wxRect GetTopIconsRegion();
	wxRect GetTopIconRect(int i);
	wxRect GetLeftIconRect(int i);
	wxRect GetBraidRegion();
	wxRect GetCrossSectionRegion();
	wxRect GetCellRegion(int x, int y);
	wxPoint GetCrossSectionPos(int iIndex, double dZValue);
	wxRect GetMarqueeRect();
	YARN_POS_DATA GetWeftCoordinate(int iIndex);
	YARN_POS_DATA GetWarpCoordinate(int iIndex);
	int GetNumberOfYarns(int i, int j);
	void DrawTopDownView(wxDC &dc);
	void DrawCell(wxDC &dc, int i, int j, bool bDirection);
	void DrawCrossSection(wxDC &dc, int iIndex);
	void DrawHorizontalYarn(wxDC &dc, vector<double> &YarnZValues);
	void DrawVerticalYarn(wxDC &dc, int iIndex, double dZValue);
	void DrawLeftIcons(wxDC &dc);
	void DrawTopIcons(wxDC &dc);
	int GetNumSelected();
	void CalculatePixelRatio();

	double m_dPixelRatio;
	int m_iFreeWidth;
	int m_iFreeHeight;
	double m_dLastSelectedWidth;
	double m_dLastSelectedHeight;
	double m_dLastSelectedSpacing;
	CObjectContainer<CTextileBraid> m_BraidPattern;
	// The amount of space left for the icons
	int m_iIconSpace;
	int m_iBorderSpace;
	// These refers to the cross section index along the y axis
	int m_iCrossSectionIndex;
	// These refer to indices in the cross section
	// the first is the index along the length of the yarn in the x axis
	// the second is the height index
	pair<int, int> m_SelectedIndices;
	vector<bool> m_SelectedWarpYarns;
	vector<bool> m_SelectedWeftYarns;
	wxPoint m_MarqueeStart;
	wxPoint m_MarqueeEnd;
};



/// Circle range control event class
class wxBraidPatternCtrlEvent : public wxCommandEvent
{
	friend class wxBraidPatternCtrl;
public:
	wxBraidPatternCtrlEvent(wxEventType commandType = wxEVT_NULL, int id = 0)
		: wxCommandEvent(commandType, id)
	{}
	wxBraidPatternCtrlEvent(const wxBraidPatternCtrlEvent& event)
		: wxCommandEvent(event)
	{}
	virtual wxEvent *Clone() const
	{
		return new wxBraidPatternCtrlEvent(*this);
	}

protected:
	DECLARE_DYNAMIC_CLASS(wxBraidPatternCtrlEvent);
};

typedef void (wxEvtHandler::*wxCircleRangeCtrlEventFunction) (wxBraidPatternCtrlEvent&);

// Circle range control events and macros for handling them
BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_COMMAND_CROSSOVER_CLICKED, NULL)
END_DECLARE_EVENT_TYPES()

#define EVT_CROSSOVER_CLICKED(id, fn) DECLARE_EVENT_TABLE_ENTRY( \
wxEVT_COMMAND_CROSSOVER_CLICKED, id, -1, (wxObjectEventFunction) \
(wxEventFunction) (wxCircleRangeCtrlEventFunction) & fn, \
(wxObject *) NULL ),
