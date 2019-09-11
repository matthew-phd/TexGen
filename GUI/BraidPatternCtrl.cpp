#include "PrecompiledHeaders.h"
#include "BraidPatternCtrl.h"

BEGIN_EVENT_TABLE(wxBraidPatternCtrl, wxControl)
	EVT_PAINT(wxBraidPatternCtrl::OnPaint)
	EVT_MOUSE_EVENTS(wxBraidPatternCtrl::OnMouseEvent)
	EVT_SIZE(wxBraidPatternCtrl::OnSize)
	EVT_MENU(wxID_ANY, wxBraidPatternCtrl::OnMenu)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(wxBraidPatternCtrl, wxControl)

DEFINE_EVENT_TYPE(EVT_CROSSOVER_CLICKED)

IMPLEMENT_DYNAMIC_CLASS(wxBraidPatternCtrlEvent, wxCommandEvent)

wxBraidPatternCtrl::wxBraidPatternCtrl()
{
	Init();
}

wxBraidPatternCtrl::wxBraidPatternCtrl(wxWindow* parent, wxWindowID id,
	const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator)
{
	Init();
	Create(parent, id, pos, size, style, validator);
	// Used in conjunction with wxBufferedPaintDC
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

wxBraidPatternCtrl::~wxBraidPatternCtrl()
{
}

bool wxBraidPatternCtrl::Create(wxWindow* parent, wxWindowID id,
	const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator)
{
	if (!wxControl::Create(parent, id, pos, size, style, validator))
		return false;
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

	SetInitialSize(size);

	return true;
}

void wxBraidPatternCtrl::Init()
{
	m_dPixelRatio = 1;
	m_iCrossSectionIndex = 0;
	m_SelectedIndices = make_pair(-1, -1);

	m_iIconSpace = 20;
	m_iBorderSpace = 10;

	m_MarqueeStart.x = -1;
	m_MarqueeStart.y = -1;

}

void wxBraidPatternCtrl::OnSize(wxSizeEvent& event)
{
}

void wxBraidPatternCtrl::OnPaint(wxPaintEvent& event)
{
	// This avoids flickering... very nice
	wxAutoBufferedPaintDC dc(this);
	dc.Clear();
	//	wxPaintDC dc(this);

	if (!m_BraidPattern)
		return;

	CalculatePixelRatio();

	wxRegion UpdateRegion = GetUpdateRegion();
	if (UpdateRegion.Intersect(GetBraidRegion()))
		DrawTopDownView(dc);

	if (UpdateRegion.Intersect(GetLeftIconsRegion()))
		DrawLeftIcons(dc);
	if (UpdateRegion.Intersect(GetTopIconsRegion()))
		DrawTopIcons(dc);
	if (UpdateRegion.Intersect(GetCrossSectionRegion()))
		DrawCrossSection(dc, m_iCrossSectionIndex);

	if (m_MarqueeStart.x != -1 || m_MarqueeStart.y != -1)
	{
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(wxPen(*wxBLACK, 1, wxDOT));
		dc.DrawRectangle(GetMarqueeRect());
		dc.SetBrush(wxNullBrush);
		dc.SetPen(wxNullPen);
	}
}

void wxBraidPatternCtrl::DrawTopDownView(wxDC &dc)
{
	CTextileBraid* pBraid = dynamic_cast<CTextileBraid*>(&*m_BraidPattern);
	int iWidth = m_BraidPattern->GetNumWarpYarns();
	int iHeight = m_BraidPattern->GetNumWeftYarns();
	int i, j;
	bool bHasWeftYarns = true;
	bool bHasWarpYarns = true;
	for (i = 0; i<iWidth; ++i)
	{
		for (j = 0; j<iHeight; ++j)
		{
			bool bDir = GetCellStatus(i, j);
			if (bDir == PATTERN_WEFTYARN)
			{
				if (bHasWarpYarns)
					DrawCell(dc, i, j, PATTERN_WARPYARN);
				if (bHasWeftYarns)
					DrawCell(dc, i, j, PATTERN_WEFTYARN);
			}
			else if (bDir == PATTERN_WARPYARN)
			{
				if (bHasWeftYarns)
					DrawCell(dc, i, j, PATTERN_WEFTYARN);
				if (bHasWarpYarns)
					DrawCell(dc, i, j, PATTERN_WARPYARN);
			}
		}
	}
	wxRect Region = GetBraidRegion();
	YARN_POS_DATA PosData = GetWarpCoordinate(m_iCrossSectionIndex);
	wxPen Pen(*wxCYAN, 3, wxDOT);
	dc.SetPen(Pen);
	dc.DrawLine(Region.x, PosData.iYarnCenter, Region.x + Region.width, PosData.iYarnCenter);
	dc.SetPen(wxNullPen);
}

void wxBraidPatternCtrl::DrawCrossSection(wxDC &dc, int iIndex)
{
	//	wxPen Pen(*wxBLUE, GetYarnHeight(), wxSOLID);
	//	dc.SetPen(Pen);
	dc.SetBrush(*wxBLUE_BRUSH);

	int i, j, iHeight;
	const CTextileBraid &BraidPattern = *m_BraidPattern;
	vector<bool>::const_iterator itCell;
	vector<double> YarnZValues;
	bool bFound = false;
	double dZValue;
	for (iHeight = 0; ; ++iHeight)
	{
		YarnZValues.clear();
		for (j = 0; j<m_BraidPattern->GetNumWarpYarns(); ++j)
		{
			bFound = false;
			const vector<bool> &CellPattern = BraidPattern.GetCell(j, iIndex);
			i = 0;
			for (itCell = CellPattern.begin(); itCell != CellPattern.end(); ++itCell)
			{
				if (*itCell == PATTERN_WEFTYARN)
				{
					if (i == iHeight)
					{
						bFound = true;
						dZValue = double(distance(CellPattern.begin(), itCell) + 0.5) / double(CellPattern.size());
						YarnZValues.push_back(dZValue);
						break;
					}
					else
					{
						++i;
					}
				}
			}
			if (!bFound)
				break;
		}
		if (bFound)
			DrawHorizontalYarn(dc, YarnZValues);
		else
			break;
	}

	//	dc.SetPen(wxNullPen);
	dc.SetBrush(wxNullBrush);


	dc.SetBrush(*wxRED_BRUSH);

	for (j = 0; j<m_BraidPattern->GetNumWeftYarns(); ++j)
	{
		const vector<bool> &CellPattern = BraidPattern.GetCell(j, iIndex);
		for (itCell = CellPattern.begin(); itCell != CellPattern.end(); ++itCell)
		{
			dZValue = double(distance(CellPattern.begin(), itCell)) / double(CellPattern.size() - 1);
			if (*itCell == PATTERN_YYARN)
			{
				dZValue = double(distance(CellPattern.begin(), itCell) + 0.5) / double(CellPattern.size());
				DrawVerticalYarn(dc, j, dZValue);
			}
		}
	}

	dc.SetBrush(wxNullBrush);
}

wxPoint wxBraidPatternCtrl::GetCrossSectionPos(int iIndex, double dZValue)
{
	wxRect Region = GetCrossSectionRegion();
	wxPoint Point;
	YARN_POS_DATA PosData = GetWeftCoordinate(iIndex);
	Point.x = PosData.iYarnCenter;
	//	Point.x = Region.x + Region.width*(double(iIndex+0.5)/double(m_WeavePattern->GetNumYYarns()));
	Point.y = Region.y + Region.height*(1 - dZValue);
	return Point;
}

void wxBraidPatternCtrl::DrawHorizontalYarn(wxDC &dc, vector<double> &YarnZValues)
{
	if (YarnZValues.empty())
		return;

	int iYarnHeight = m_dPixelRatio * m_BraidPattern->GetWeftYarnHeight(m_iCrossSectionIndex);
	//	int iYarnHeight = GetYarnHeight();
	int i, iSize = (int)YarnZValues.size();
	vector<double>::const_iterator itZValue;
	vector<CNode> MasterNodes;
	wxRect CrossSectionRegion = GetCrossSectionRegion();
	CNode FirstNode, LastNode;
	for (itZValue = YarnZValues.begin(), i = 0; itZValue != YarnZValues.end(); ++itZValue, ++i)
	{
		wxPoint Point = GetCrossSectionPos(i, *itZValue);
		if (i == 0)
			LastNode = CNode(XYZ(Point.x + CrossSectionRegion.width, 0, Point.y), XYZ(1, 0, 0));
		if (i == (int)YarnZValues.size() - 1)
			FirstNode = CNode(XYZ(Point.x - CrossSectionRegion.width, 0, Point.y), XYZ(1, 0, 0));
		MasterNodes.push_back(CNode(XYZ(Point.x, 0, Point.y), XYZ(1, 0, 0)));
	}
	MasterNodes.insert(MasterNodes.begin(), FirstNode);
	MasterNodes.push_back(LastNode);

	int iDivisions = 50;

	wxPoint* aPoints = new wxPoint[iDivisions * 2];

	CInterpolationBezier Interp(true);
	vector<CSlaveNode> SlaveNodes = Interp.GetSlaveNodes(MasterNodes, iDivisions, false);
	XYZ Position, P1, P2;
	XYZ Tangent;
	for (i = 0; i<iDivisions; ++i)
	{
		Position = SlaveNodes[i].GetPosition();
		//		Tangent = SlaveNodes[i].GetTangent();
		Tangent = XYZ(1, 0, 0);
		Tangent = XYZ(Tangent.z, 0, -Tangent.x);
		P1 = Position + Tangent * (0.5*iYarnHeight);
		P2 = Position - Tangent * (0.5*iYarnHeight);
		aPoints[i].x = P1.x;
		aPoints[i].y = P1.z;
		aPoints[2 * iDivisions - (i + 1)].x = P2.x;
		aPoints[2 * iDivisions - (i + 1)].y = P2.z;
	}

	//	dc.DrawLines(iSize, aPoints);
	//	dc.DrawSpline(iSize, aPoints);
	dc.SetClippingRegion(CrossSectionRegion);
	dc.DrawPolygon(2 * iDivisions, aPoints);
	dc.DestroyClippingRegion();

	delete[] aPoints;
}

void wxBraidPatternCtrl::DrawVerticalYarn(wxDC &dc, int iIndex, double dZValue)
{
	wxPoint Center = GetCrossSectionPos(iIndex, dZValue);

	YARN_POS_DATA XPosData = GetWeftCoordinate(iIndex);
	int iYarnHeight = m_dPixelRatio * m_BraidPattern->GetWarpYarnHeight(iIndex);
	wxSize Size(XPosData.iYarnMax - XPosData.iYarnMin, iYarnHeight);

	dc.DrawEllipse(Center.x - Size.x / 2, Center.y - Size.y / 2, Size.x, Size.y);
}

void wxBraidPatternCtrl::DrawCell(wxDC &dc, int i, int j, bool bDirection)
{
	YARN_POS_DATA XPosData = GetWeftCoordinate(i);
	YARN_POS_DATA YPosData = GetWarpCoordinate(j);
	wxRect Cell;
	if (bDirection == PATTERN_WARPYARN)
	{
		// Yarns going in the Y direction
		Cell.x = XPosData.iYarnMin;
		Cell.width = XPosData.iYarnMax - XPosData.iYarnMin;
		Cell.y = YPosData.iCellMin;
		Cell.height = YPosData.iCellMax - YPosData.iCellMin;

		dc.GradientFillLinear(Cell, wxColour(255, 0, 0), wxColour(200, 0, 0), wxEAST);
		dc.DrawLine(Cell.x, Cell.y, Cell.x, Cell.y + Cell.height);
		dc.DrawLine(Cell.x + Cell.width, Cell.y, Cell.x + Cell.width, Cell.y + Cell.height);
	}
	else if (bDirection == PATTERN_WEFTYARN)
	{
		Cell.x = XPosData.iCellMin;
		Cell.width = XPosData.iCellMax - XPosData.iCellMin;
		Cell.y = YPosData.iYarnMin;
		Cell.height = YPosData.iYarnMax - YPosData.iYarnMin;

		dc.GradientFillLinear(Cell, wxColour(0, 0, 255), wxColour(0, 0, 200), wxSOUTH);
		dc.DrawLine(Cell.x, Cell.y, Cell.x + Cell.width, Cell.y);
		dc.DrawLine(Cell.x, Cell.y + Cell.height, Cell.x + Cell.width, Cell.y + Cell.height);
	}
}

int wxBraidPatternCtrl::GetWeaveWidth()
{
	if (!m_BraidPattern)
		return 0;
	return m_BraidPattern->GetNumWarpYarns();
}

int wxBraidPatternCtrl::GetWeaveHeight()
{
	if (!m_BraidPattern)
		return 0;
	return m_BraidPattern->GetNumWeftYarns();
}

bool wxBraidPatternCtrl::GetCellStatus(int i, int j)
{
	if (!m_BraidPattern)
		return false;
	const CTextileBraid &BraidPattern = *m_BraidPattern;
	const vector<bool> &CellPattern =BraidPattern.GetCell(i, j);
	if (CellPattern.empty())
		return false;	// Return some rubish
	return CellPattern.back();
}
/*
void wxWeavePatternCtrl::SetCellStatus(int i, int j, bool bVal)
{
if (!m_WeavePattern)
return;
if (GetCellStatus(i, j) != bVal)
{
m_WeavePattern->SwapPosition(i, j);
}
}
*/

void wxBraidPatternCtrl::DrawTopIcons(wxDC &dc)
{
	wxRect TopIconsRect = GetTopIconsRegion();

	wxRect Cell;
	wxPoint RefPoint;

	CTextileBraid* pBraid = dynamic_cast<CTextileBraid*>(&*m_BraidPattern);

	int i, iLayers = 0;
	for (i = 0; i<m_BraidPattern->GetNumWarpYarns(); ++i)
	{
		Cell = GetTopIconRect(i);
		//if (pWeave3D)
			//iLayers = pWeave3D->GetNumYLayers(i);

		//wxString NumLayers;
		//if (iLayers)
			//NumLayers << iLayers;

		if (m_SelectedYYarns[i])
			dc.SetBrush(*wxRED_BRUSH);
		else
			dc.SetBrush(*wxWHITE_BRUSH);
		dc.DrawRectangle(Cell.x, Cell.y, Cell.width, Cell.height);
		//dc.DrawLabel(NumLayers, Cell, wxALIGN_CENTER);
		dc.SetBrush(wxNullBrush);
	}
}

void wxBraidPatternCtrl::DrawLeftIcons(wxDC &dc)
{
	wxRect LeftIconsRect = GetLeftIconsRegion();

	wxRect Cell;
	wxPoint RefPoint;

	CTextileBraid* pBraid = dynamic_cast<CTextileBraid*>(&*m_BraidPattern);

	int i, iLayers = 0;
	for (i = 0; i<m_BraidPattern->GetNumWeftYarns(); ++i)
	{
		Cell = GetLeftIconRect(i);
		//if (pWeave3D)
			//iLayers = pWeave3D->GetNumXLayers(i);

		//wxString NumLayers;
		//if (iLayers)
			//NumLayers << iLayers;

		if (m_SelectedXYarns[i])
			dc.SetBrush(*wxRED_BRUSH);
		else
			dc.SetBrush(*wxWHITE_BRUSH);
		dc.DrawRectangle(Cell.x, Cell.y, Cell.width, Cell.height);
		//dc.DrawLabel(NumLayers, Cell, wxALIGN_CENTER);
		dc.SetBrush(wxNullBrush);
	}
}

wxRect wxBraidPatternCtrl::GetTopIconRect(int i)
{
	wxRect TopIconsRect = GetTopIconsRegion();
	YARN_POS_DATA XPosData = GetWeftCoordinate(i);

	TopIconsRect.x = XPosData.iCellMin;
	TopIconsRect.width = XPosData.iCellMax - XPosData.iCellMin;

	return TopIconsRect;
}

wxRect wxWeavePatternCtrl::GetLeftIconRect(int i)
{
	wxRect LeftIconsRect = GetLeftIconsRegion();
	YARN_POS_DATA YPosData = GetYCoordinate(i);

	LeftIconsRect.y = YPosData.iCellMin;
	LeftIconsRect.height = YPosData.iCellMax - YPosData.iCellMin;

	return LeftIconsRect;
}

void wxWeavePatternCtrl::HandleIconSelect(wxRect MarqueeSelect)
{
	wxRect TopIconsRect = GetTopIconsRegion();
	wxRect LeftIconsRect = GetLeftIconsRegion();
	if (TopIconsRect.Intersects(MarqueeSelect))
	{
		int i;
		for (i = 0; i<m_WeavePattern->GetNumYYarns(); ++i)
		{
			if (GetTopIconRect(i).Intersects(MarqueeSelect))
			{
				m_SelectedYYarns[i] = true;
				m_dLastSelectedWidth = m_WeavePattern->GetYYarnWidths(i);
				m_dLastSelectedHeight = m_WeavePattern->GetYYarnHeights(i);
				m_dLastSelectedSpacing = m_WeavePattern->GetYYarnSpacings(i);
			}
		}
	}
	if (LeftIconsRect.Intersects(MarqueeSelect))
	{
		int i;
		for (i = 0; i<m_WeavePattern->GetNumXYarns(); ++i)
		{
			if (GetLeftIconRect(i).Intersects(MarqueeSelect))
			{
				m_SelectedXYarns[i] = true;
				m_dLastSelectedWidth = m_WeavePattern->GetXYarnWidths(i);
				m_dLastSelectedHeight = m_WeavePattern->GetXYarnHeights(i);
				m_dLastSelectedSpacing = m_WeavePattern->GetXYarnSpacings(i);
			}
		}
	}
}

bool wxWeavePatternCtrl::HandleIconSelect(wxPoint Position)
{
	wxRect TopIconsRect = GetTopIconsRegion();
	wxRect LeftIconsRect = GetLeftIconsRegion();
	if (TopIconsRect.Contains(Position))
	{
		int i;
		for (i = 0; i<m_WeavePattern->GetNumYYarns(); ++i)
		{
			if (GetTopIconRect(i).Contains(Position))
			{
				m_SelectedYYarns[i] = true;
				m_dLastSelectedWidth = m_WeavePattern->GetYYarnWidths(i);
				m_dLastSelectedHeight = m_WeavePattern->GetYYarnHeights(i);
				m_dLastSelectedSpacing = m_WeavePattern->GetYYarnSpacings(i);
				break;
			}
		}
		if (i != m_WeavePattern->GetNumYYarns())
		{
			Refresh();
			return true;
		}
	}
	else if (LeftIconsRect.Contains(Position))
	{
		int i;
		for (i = 0; i<m_WeavePattern->GetNumXYarns(); ++i)
		{
			if (GetLeftIconRect(i).Contains(Position))
			{
				m_SelectedXYarns[i] = true;
				m_dLastSelectedWidth = m_WeavePattern->GetXYarnWidths(i);
				m_dLastSelectedHeight = m_WeavePattern->GetXYarnHeights(i);
				m_dLastSelectedSpacing = m_WeavePattern->GetXYarnSpacings(i);
				break;
			}
		}
		if (i != m_WeavePattern->GetNumYYarns())
		{
			Refresh();
			return true;
		}
	}
	return false;
}

wxRect wxWeavePatternCtrl::GetLeftIconsRegion()
{
	wxRect Rect = GetWeaveRegion();
	Rect.x -= m_iIconSpace;
	Rect.width = m_iIconSpace - 2;
	return Rect;
}

wxRect wxWeavePatternCtrl::GetTopIconsRegion()
{
	wxRect Rect = GetWeaveRegion();
	Rect.y -= m_iIconSpace;
	Rect.height = m_iIconSpace - 2;
	return Rect;
}

void wxWeavePatternCtrl::CalculatePixelRatio()
{
	wxRect Rect = GetClientRect();
	double dWidth = m_WeavePattern->GetWidth();
	double dHeight = m_WeavePattern->GetHeight();
	Rect.width -= m_iIconSpace;
	Rect.height -= m_iIconSpace;
	dHeight += m_WeavePattern->GetFabricThickness();
	Rect.height -= m_iBorderSpace;
	m_dPixelRatio = min(Rect.width / dWidth, Rect.height / dHeight);
	m_iFreeWidth = Rect.width - m_dPixelRatio * dWidth;
	m_iFreeHeight = Rect.height - m_dPixelRatio * dHeight;
}

wxRect wxBraidPatternCtrl::GetBraidRegion()
{
	wxRect ClientRect = GetClientRect();
	wxRect Rect = ClientRect;
	Rect.x += m_iIconSpace;
	Rect.y += m_iIconSpace;
	Rect.width = m_BraidPattern->GetWidth()*m_dPixelRatio;
	Rect.height = m_BraidPattern->GetHeight()*m_dPixelRatio;

	Rect.x += m_iFreeWidth / 2;
	Rect.y += m_iFreeHeight / 2;

	return Rect;
}

wxRect wxWeavePatternCtrl::GetCrossSectionRegion()
{
	wxRect WeaveRegion = GetWeaveRegion();

	wxRect ClientRect = GetClientRect();
	wxRect CrossSectionRect;
	CrossSectionRect.y = WeaveRegion.y + WeaveRegion.height + m_iBorderSpace;
	CrossSectionRect.height = m_WeavePattern->GetFabricThickness() * m_dPixelRatio;

	CrossSectionRect.x = WeaveRegion.x;
	CrossSectionRect.width = WeaveRegion.width;

	return CrossSectionRect;
}

int wxWeavePatternCtrl::GetNumberOfYarns(int i, int j)
{
	const CTextileWeave &WeavePattern = *m_WeavePattern;
	return (int)WeavePattern.GetCell(i, j).size();
}

wxBraidPatternCtrl::YARN_POS_DATA wxBraidPatternCtrl::GetWeftCoordinate(int iIndex)
{
	double dAccumulated = 0;
	double dWidth, dGapBefore, dGapAfter;
	double dCellMin, dYarnMin, dYarnMax, dCellMax;
	double dTotalSpacingWarp = 0;
	int i, iNumWarpYarns = m_BraidPattern->GetNumWarpYarns();
	for (i = 0; i<iNumWarpYarns; ++i)
	{
		dWidth = m_BraidPattern->GetWarpYarnWidth(i);
		//if (i>0)
			//dGapBefore = 0.5*m_WeavePattern->GetYYarnGapSize(i - 1);
		//else
			//dGapBefore = 0.5*m_WeavePattern->GetYYarnGapSize(iNumYYarns - 1);
		//dGapAfter = 0.5*m_WeavePattern->GetYYarnGapSize(i);
		if (iIndex == i)
		{
			dCellMin = dAccumulated;
			dYarnMin = dCellMin;// + dGapBefore;
			dYarnMax = dYarnMin + dWidth;
			dCellMax = dYarnMax; //+ dGapAfter;
		}
		dAccumulated += dWidth;// +dGapBefore + dGapAfter;
		dTotalSpacingWarp += m_BraidPattern->GetWarpYarnSpacing(i);
	}
	dCellMin /= dTotalSpacingWarp;
	dYarnMin /= dTotalSpacingWarp;
	dYarnMax /= dTotalSpacingWarp;
	dCellMax /= dTotalSpacingWarp;
	double dYarnCenter = 0.5*(dYarnMin + dYarnMax);
	wxRect BraidRegion = GetBraidRegion();
	YARN_POS_DATA YarnPosData;
	YarnPosData.iCellMin = BraidRegion.x + dCellMin * BraidRegion.width;
	YarnPosData.iYarnMin = BraidRegion.x + dYarnMin * BraidRegion.width;
	YarnPosData.iYarnCenter = BraidRegion.x + dYarnCenter * BraidRegion.width;
	YarnPosData.iYarnMax = BraidRegion.x + dYarnMax * BraidRegion.width;
	YarnPosData.iCellMax = BraidRegion.x + dCellMax * BraidRegion.width;
	return YarnPosData;
}

wxBraidPatternCtrl::YARN_POS_DATA wxBraidPatternCtrl::GetWarpCoordinate(int iIndex)
{
	double dAccumulated = 0;
	double dWidth, dGapBefore, dGapAfter;
	double dCellMin, dYarnMin, dYarnMax, dCellMax;
	double dTotalSpacingWeft = 0;
	int i, iNumXYarns = m_BraidPattern->GetNumWeftYarns();
	for (i = 0; i<iNumXYarns; ++i)
	{
		dWidth = m_BraidPattern->GetWeftYarnWidth(i);
		//if (i>0)
			//dGapBefore = 0.5*m_WeavePattern->GetXYarnGapSize(i - 1);
		//else
			//dGapBefore = 0.5*m_WeavePattern->GetXYarnGapSize(iNumXYarns - 1);
		//dGapAfter = 0.5*m_WeavePattern->GetXYarnGapSize(i);
		if (iIndex == i)
		{
			dCellMin = dAccumulated;
			dYarnMin = dCellMin;// +dGapBefore;
			dYarnMax = dYarnMin + dWidth;
			dCellMax = dYarnMax;// +dGapAfter;
		}
		dAccumulated += dWidth;// +dGapBefore + dGapAfter;
		dTotalSpacingWeft += m_BraidPattern->GetWeftYarnSpacing(i);
	}
	dCellMin /= dTotalSpacingWeft;
	dYarnMin /= dTotalSpacingWeft;
	dYarnMax /= dTotalSpacingWeft;
	dCellMax /= dTotalSpacingWeft;
	double dYarnCenter = 0.5*(dYarnMin + dYarnMax);
	wxRect BraidRegion = GetBraidRegion();
	YARN_POS_DATA YarnPosData;
	// Note: max and min are flipped because of Y flip
	YarnPosData.iCellMax = BraidRegion.y + (1 - dCellMin)*BraidRegion.height;
	YarnPosData.iYarnMax = BraidRegion.y + (1 - dYarnMin)*BraidRegion.height;
	YarnPosData.iYarnCenter = BraidRegion.y + (1 - dYarnCenter)*BraidRegion.height;
	YarnPosData.iYarnMin = BraidRegion.y + (1 - dYarnMax)*BraidRegion.height;
	YarnPosData.iCellMin = BraidRegion.y + (1 - dCellMax)*BraidRegion.height;
	return YarnPosData;
}


wxRect wxWeavePatternCtrl::GetCellRegion(int x, int y)
{
	YARN_POS_DATA XPosData = GetXCoordinate(x);
	YARN_POS_DATA YPosData = GetYCoordinate(y);
	wxRect Cell;
	Cell.x = XPosData.iCellMin;
	Cell.width = XPosData.iCellMax - XPosData.iCellMin;
	Cell.y = YPosData.iCellMin;
	Cell.height = YPosData.iCellMax - YPosData.iCellMin;
	return Cell;
}

pair<int, int> wxWeavePatternCtrl::GetCellIndices(wxPoint Position)
{
	wxRect WeaveRegion = GetWeaveRegion();
	if (!WeaveRegion.Contains(Position))
		return make_pair(-1, -1);

	YARN_POS_DATA PosData;
	int i;
	int x = -1, y = -1;
	int iNumYYarns = m_WeavePattern->GetNumYYarns();
	int iNumXYarns = m_WeavePattern->GetNumXYarns();
	for (i = 0; i<iNumYYarns; ++i)
	{
		PosData = GetXCoordinate(i);
		if (Position.x >= PosData.iCellMin &&
			Position.x < PosData.iCellMax)
			x = i;
	}
	for (i = 0; i<iNumXYarns; ++i)
	{
		PosData = GetYCoordinate(i);
		if (Position.y >= PosData.iCellMin &&
			Position.y < PosData.iCellMax)
			y = i;
	}

	if (x != -1 && y != -1)
		return make_pair(x, y);
	else
		return make_pair(-1, -1);
}

pair<int, int> wxWeavePatternCtrl::GetCellHeight(wxPoint Position)
{
	wxRect Region = GetCrossSectionRegion();
	if (!Region.Contains(Position))
		return make_pair(-1, -1);

	int iWidth = m_WeavePattern->GetNumYYarns();
	int iDivisionWidth = Region.width / iWidth;
	int i = (Position.x - Region.x) / iDivisionWidth;
	if (i<0)
		i = 0;
	if (i >= iWidth)
		i = iWidth - 1;

	int iHeight = GetNumberOfYarns(i, m_iCrossSectionIndex);
	int iDivisionHeight = Region.height / iHeight;
	int j = (Position.y - Region.y) / iDivisionHeight;
	j = iHeight - (j + 1);
	return make_pair(i, j);
}

void wxWeavePatternCtrl::OnMenu(wxCommandEvent& event)
{
	CTextileWeave3D* pWeave3D = dynamic_cast<CTextileWeave3D*>(&*m_WeavePattern);
	switch (event.GetId())
	{
	case MENUID_ADDLAYER:
		if (pWeave3D)
		{
			int i;
			for (i = 0; i<pWeave3D->GetNumYYarns(); ++i)
			{
				if (m_SelectedYYarns[i])
					pWeave3D->AddYLayers(i, 1);
			}
			for (i = 0; i<pWeave3D->GetNumXYarns(); ++i)
			{
				if (m_SelectedXYarns[i])
					pWeave3D->AddXLayers(i, 1);
			}
		}
		break;
	case MENUID_REMOVELAYER:
		if (pWeave3D)
		{
			int i;
			for (i = 0; i<pWeave3D->GetNumYYarns(); ++i)
			{
				if (m_SelectedYYarns[i])
					pWeave3D->DeleteYLayers(i, 1);
			}
			for (i = 0; i<pWeave3D->GetNumXYarns(); ++i)
			{
				if (m_SelectedXYarns[i])
					pWeave3D->DeleteXLayers(i, 1);
			}
		}
		break;
	case MENUID_SETYARNWIDTH:
	{
		wxString Val = wxGetTextFromUser(wxT("Please enter the yarn width"), wxT("Input value"), wxString() << m_dLastSelectedWidth, this);
		double dVal;
		if (Val.ToDouble(&dVal))
		{
			m_dLastSelectedWidth = dVal;
			int i;
			for (i = 0; i<m_WeavePattern->GetNumYYarns(); ++i)
			{
				if (m_SelectedYYarns[i])
					m_WeavePattern->SetYYarnWidths(i, dVal);
			}
			for (i = 0; i<m_WeavePattern->GetNumXYarns(); ++i)
			{
				if (m_SelectedXYarns[i])
					m_WeavePattern->SetXYarnWidths(i, dVal);
			}
		}
	}
	break;
	case MENUID_SETYARNHEIGHT:
	{
		wxString Val = wxGetTextFromUser(wxT("Please enter the yarn height"), wxT("Input value"), wxString() << m_dLastSelectedHeight, this);
		double dVal;
		if (Val.ToDouble(&dVal))
		{
			m_dLastSelectedHeight = dVal;
			int i;
			for (i = 0; i<m_WeavePattern->GetNumYYarns(); ++i)
			{
				if (m_SelectedYYarns[i])
					m_WeavePattern->SetYYarnHeights(i, dVal);
			}
			for (i = 0; i<m_WeavePattern->GetNumXYarns(); ++i)
			{
				if (m_SelectedXYarns[i])
					m_WeavePattern->SetXYarnHeights(i, dVal);
			}
		}
	}
	break;
	case MENUID_SETYARNSPACING:
	{
		wxString Val = wxGetTextFromUser(wxT("Please enter the yarn spacing"), wxT("Input value"), wxString() << m_dLastSelectedSpacing, this);
		double dVal;
		if (Val.ToDouble(&dVal))
		{
			m_dLastSelectedSpacing = dVal;
			int i;
			for (i = 0; i<m_WeavePattern->GetNumYYarns(); ++i)
			{
				if (m_SelectedYYarns[i])
					m_WeavePattern->SetYYarnSpacings(i, dVal);
			}
			for (i = 0; i<m_WeavePattern->GetNumXYarns(); ++i)
			{
				if (m_SelectedXYarns[i])
					m_WeavePattern->SetXYarnSpacings(i, dVal);
			}
		}
	}
	break;
	}
	Refresh();
}

void wxWeavePatternCtrl::OnMouseEvent(wxMouseEvent& event)
{
	m_MarqueeEnd = event.GetPosition();
	if (event.LeftDown() || event.LeftDClick())
	{
		if (!event.ShiftDown() && !event.ControlDown() && GetNumSelected() > 0)
		{
			fill(m_SelectedYYarns.begin(), m_SelectedYYarns.end(), false);
			fill(m_SelectedXYarns.begin(), m_SelectedXYarns.end(), false);
			RefreshRect(GetTopIconsRegion());
			RefreshRect(GetLeftIconsRegion());
		}
		wxPoint MousePos = event.GetPosition();
		pair<int, int> Indices = GetCellIndices(MousePos);
		if (!m_b3DMode)
		{
			if (Indices.first >= 0 && Indices.first < m_WeavePattern->GetNumYYarns() &&
				Indices.second >= 0 && Indices.second < m_WeavePattern->GetNumXYarns())
			{
				CTextileWeave2D* pWeave2D = dynamic_cast<CTextileWeave2D*>(&*m_WeavePattern);
				if (pWeave2D)
				{
					pWeave2D->SwapPosition(Indices.first, Indices.second);
					RefreshRect(GetCellRegion(Indices.first, Indices.second));
					RefreshRect(GetCrossSectionRegion());
				}
			}
			else
			{
				if (!HandleIconSelect(MousePos))
					m_MarqueeStart = MousePos;
			}
		}
		else
		{
			if (Indices.second >= 0 && Indices.second < m_WeavePattern->GetNumXYarns())
			{
				m_iCrossSectionIndex = Indices.second;
				Refresh();
			}
			else
			{
				Indices = GetCellHeight(MousePos);
				if (Indices.first >= 0 && Indices.first < m_WeavePattern->GetNumYYarns())
				{
					m_SelectedIndices = Indices;
				}
				else
				{
					if (!HandleIconSelect(MousePos))
						m_MarqueeStart = MousePos;
				}
			}
		}
	}
	if (event.RightDown())
	{
		pair<int, int> Indices = GetCellIndices(event.GetPosition());
		if (Indices.second >= 0 && Indices.second < m_WeavePattern->GetNumXYarns())
		{
			m_iCrossSectionIndex = Indices.second;
			Refresh();
		}
		else
		{
			if (GetNumSelected() == 0)
			{
				HandleIconSelect(event.GetPosition());
			}
			// Note that HandleIconSelect may change the state of GetNumSelected(), so no else if used here
			if (GetNumSelected() != 0)
			{
				wxMenu PopupMenu;
				if (m_b3DMode)
				{
					PopupMenu.Append(MENUID_ADDLAYER, wxT("Add layer"));
					PopupMenu.Append(MENUID_REMOVELAYER, wxT("Remove layer"));
				}
				PopupMenu.Append(MENUID_SETYARNWIDTH, wxT("Set yarn width..."));
				PopupMenu.Append(MENUID_SETYARNHEIGHT, wxT("Set yarn height..."));
				PopupMenu.Append(MENUID_SETYARNSPACING, wxT("Set yarn spacing..."));
				wxWindow::PopupMenu(&PopupMenu);
			}
		}
	}
	if (event.Dragging())
	{
		if (m_SelectedIndices.first != -1 && m_SelectedIndices.second != -1)
		{
			wxPoint MousePos = event.GetPosition();
			pair<int, int> Indices = GetCellHeight(MousePos);
			if (Indices.first == m_SelectedIndices.first &&
				Indices.second != m_SelectedIndices.second &&
				Indices.second != -1)
			{
				CTextileWeave3D* pWeave3D = dynamic_cast<CTextileWeave3D*>(&*m_WeavePattern);
				if (pWeave3D)
				{
					pWeave3D->SwapPosition(m_SelectedIndices.first, m_iCrossSectionIndex, m_SelectedIndices.second, Indices.second);
					m_SelectedIndices = Indices;
					Refresh();
				}
			}
		}
		if (m_MarqueeStart.x != -1 || m_MarqueeStart.y != -1)
		{
			m_MarqueeEnd = event.GetPosition();
			Refresh();
		}
	}
	if (event.LeftUp() || event.Leaving())
	{
		if (m_MarqueeStart.x != -1 || m_MarqueeStart.y != -1)
		{
			HandleIconSelect(GetMarqueeRect());
			Refresh();
		}
		m_SelectedIndices = make_pair(-1, -1);
		m_MarqueeStart.x = -1;
		m_MarqueeStart.y = -1;
	}
}

int wxWeavePatternCtrl::GetNumSelected()
{
	int iNum = 0;
	iNum += count(m_SelectedYYarns.begin(), m_SelectedYYarns.end(), true);
	iNum += count(m_SelectedXYarns.begin(), m_SelectedXYarns.end(), true);
	return iNum;
}

wxRect wxWeavePatternCtrl::GetMarqueeRect()
{
	wxRect MarqueeRect(m_MarqueeStart.x, m_MarqueeStart.y, m_MarqueeEnd.x - m_MarqueeStart.x, m_MarqueeEnd.y - m_MarqueeStart.y);
	if (MarqueeRect.width < 0)
	{
		MarqueeRect.width *= -1;
		MarqueeRect.x -= MarqueeRect.width;
	}
	if (MarqueeRect.height < 0)
	{
		MarqueeRect.height *= -1;
		MarqueeRect.y -= MarqueeRect.height;
	}
	return MarqueeRect;
}

void wxWeavePatternCtrl::SetWeaveSize(int iNumYYarns, int iNumXYarns, bool b3DMode)
{
	m_b3DMode = b3DMode;
	if (!b3DMode)
	{
		m_WeavePattern = CTextileWeave2D(iNumYYarns, iNumXYarns, 1, 1, false);
	}
	else
	{
		CTextileWeave3D Weave3D(iNumYYarns, iNumXYarns, 1, 1);
		Weave3D.AddYLayers(1);
		Weave3D.AddXLayers(1);
		m_WeavePattern = Weave3D;
	}
	m_SelectedYYarns.resize(m_WeavePattern->GetNumYYarns());
	m_SelectedXYarns.resize(m_WeavePattern->GetNumXYarns());
}

void wxWeavePatternCtrl::SetWeaveSize(const CTextileWeave &Weave)
{
	if (Weave.GetType() == "CTextileWeave3D")
		m_b3DMode = true;
	else
		m_b3DMode = false;
	m_WeavePattern = Weave;
	m_SelectedYYarns.resize(m_WeavePattern->GetNumYYarns());
	m_SelectedXYarns.resize(m_WeavePattern->GetNumXYarns());
}
