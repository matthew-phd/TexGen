#include "PrecompiledHeaders.h"
#include "TextileBraid.h"
#include "SectionEllipse.h"
#include "SectionRotated.h"
#include "SectionPolygon.h"

using namespace TexGen;


CTextileBraid::CTextileBraid(int iNumWeftYarns, int iNumWarpYarns, double dWidth, 
	double dHeight, double dSpacing, double dThickness, double dBraidAngle)
	: m_iNumWeftYarns(iNumWeftYarns)
	, m_iNumWarpYarns(iNumWarpYarns)
	, m_dGapSize(0)
	, m_dFabricThickness(dThickness)
	, m_iResolution(40)
	, m_dbraidAngle(dBraidAngle)
{
	m_Pattern.resize(iNumWeftYarns*iNumWarpYarns);
	YARNDATA YarnData;
	YarnData.dSpacing = dSpacing;
	YarnData.dWidth = dWidth;
	YarnData.dHeight = m_dFabricThickness / 2;
	m_WeftYarnData.resize(iNumWeftYarns, YarnData);
	m_WarpYarnData.resize(iNumWarpYarns, YarnData);

	vector<bool> Cell;
	Cell.push_back(PATTERN_WARPYARN);
	Cell.push_back(PATTERN_WEFTYARN);
	int i, j;
	for (i = 0; i<m_iNumWarpYarns; ++i)
	{
		for (j = 0; j<m_iNumWeftYarns; ++j)
		{
			GetCell(i, j) = Cell;
		}
	}
}


CTextileBraid::~CTextileBraid()
{
}

vector<PATTERNBIAX>& TexGen::CTextileBraid::GetCell(int x, int y)
{
	return m_Pattern[x + m_iNumWarpYarns * y];
}

bool CTextileBraid::BuildTextile() const
{
	m_Yarns.clear();
	m_WarpYarns.clear();
	m_WeftYarns.clear();

	m_WarpYarns.resize(m_iNumWarpYarns);
	m_WeftYarns.resize(m_iNumWeftYarns);

	TGLOGINDENT("Building braid \"" << GetName() << "\"");

	vector<int> Yarns;

	double x, y, z;


	// Add x yarns (yarns parallel to the x axis)
	int i, j, k, iYarn;
	double startx = 0;
	double starty = 0;
	for (i = 0; i < m_iNumWeftYarns; ++i)
	{
		x = startx;
		y = starty;
		Yarns.clear();
		for (j = 0; j <= m_iNumWarpYarns; ++j)
		{
			const vector<PATTERNBIAX> &Cell = GetCell(j%m_iNumWarpYarns, i);
			if (j == 0)
			{
				for (k = 0; k < (int)Cell.size(); ++k)
				{
					if (Cell[k] == PATTERN_WEFTYARN)
					{
						Yarns.push_back(AddYarn(CYarn()));
					}
				}
			}
			m_WeftYarns[i] = Yarns;
			iYarn = 0;
			z = m_dFabricThickness / (2 * Cell.size());
			for (k = 0; k < (int)Cell.size(); ++k)
			{
				if (Cell[k] == PATTERN_WEFTYARN)
				{
					m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(sin(m_dbraidAngle), cos(m_dbraidAngle), 0)));
					++iYarn;
				}
				z += m_dFabricThickness / Cell.size();
			}
			if (j < m_iNumWarpYarns)
			{
				x += m_WarpYarnData[j].dSpacing*sin(m_dbraidAngle);
				y += m_WarpYarnData[j].dSpacing*cos(m_dbraidAngle);
				
			}
			
		}
		startx += m_WeftYarnData[i].dSpacing*sin(m_dbraidAngle);
		starty += m_WeftYarnData[i].dSpacing*cos(m_dbraidAngle)*-1;
		
	}

	// Add y yarns (yarns parallel to the y axis)
	startx = 0;
	starty = 0;
	for (j = 0; j < m_iNumWarpYarns; ++j)
	{
		x = startx;
		y = starty;
		Yarns.clear();
		for (i = 0; i <= m_iNumWeftYarns; ++i)
		{
			const vector<PATTERNBIAX> &Cell = GetCell(j, i%m_iNumWeftYarns);
			if (i == 0)
			{
				for (k = 0; k < (int)Cell.size(); ++k)
				{
					if (Cell[k] == PATTERN_WARPYARN)
					{
						Yarns.push_back(AddYarn(CYarn()));
					}
				}
			}
			m_WarpYarns[j] = Yarns;
			iYarn = 0;
			z = m_dFabricThickness / (2 * Cell.size());
			for (k = 0; k < (int)Cell.size(); ++k)
			{
				if (Cell[k] == PATTERN_WARPYARN)
				{
					m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(sin(m_dbraidAngle), -1*cos(m_dbraidAngle), 0)));
					++iYarn;
				}
				z += m_dFabricThickness / Cell.size();
			}
			if (i < m_iNumWeftYarns)
			{
				y -= m_WeftYarnData[i].dSpacing*cos(m_dbraidAngle);
				x += m_WeftYarnData[i].dSpacing*sin(m_dbraidAngle);
				
			}
			
		}
		startx += m_WarpYarnData[j].dSpacing*sin(m_dbraidAngle);
		starty += m_WarpYarnData[j].dSpacing*cos(m_dbraidAngle);
		
	}


	// Assign sections and interpolation to the yarns

	double dWidthWeft;
	double dHeightWeft;
	double dWidthWarp;
	double dHeightWarp;
	dWidthWeft = GetWidthWeft();
	dHeightWeft = GetHeightWeft();
	dWidthWarp = GetWidthWarp();
	dHeightWarp = GetHeightWarp();

	vector<int>::iterator itpYarn;
	double dWidth, dHeight;
	for (i = 0; i < m_iNumWeftYarns; ++i)
	{
		dWidth = m_WeftYarnData[i].dWidth;
		dHeight = m_WeftYarnData[i].dHeight;
		CSectionEllipse Section(dWidth, dHeight);
		if (m_pSectionMesh)
			Section.AssignSectionMesh(*m_pSectionMesh);
		for (itpYarn = m_WeftYarns[i].begin(); itpYarn != m_WeftYarns[i].end(); ++itpYarn)
		{
			m_Yarns[*itpYarn].AssignSection(CYarnSectionConstant(Section));	
			
		}
	}
	for (i = 0; i < m_iNumWarpYarns; ++i)
	{
		dWidth = m_WarpYarnData[i].dWidth;
		dHeight = m_WarpYarnData[i].dHeight;
		CSectionEllipse Section(dWidth, dHeight);
		if (m_pSectionMesh)
			Section.AssignSectionMesh(*m_pSectionMesh);
		for (itpYarn = m_WarpYarns[i].begin(); itpYarn != m_WarpYarns[i].end(); ++itpYarn)
		{
			m_Yarns[*itpYarn].AssignSection(CYarnSectionConstant(Section));
		}
	}
	// Add repeats and set interpolation
	
	vector<CYarn>::iterator itYarn;
	for (itYarn = m_Yarns.begin(); itYarn != m_Yarns.end(); ++itYarn)
	{
		itYarn->AssignInterpolation(CInterpolationBezier());
		itYarn->SetResolution(m_iResolution);
		itYarn->AddRepeat(XYZ(dWidthWeft, -dHeightWeft, 0));
		itYarn->AddRepeat(XYZ(dWidthWarp, dHeightWarp, 0));
	}

	return true;
	
}

const vector<PATTERNBIAX>& TexGen::CTextileBraid::GetCell(int x, int y) const
{
	return m_Pattern[x + m_iNumWarpYarns * y];
}

double CTextileBraid::GetWidthWarp() const {
	double dWidthWarp = 0;
	for (int i = 0; i < m_iNumWarpYarns; i++) {
		dWidthWarp += m_WarpYarnData[i].dSpacing*sin(m_dbraidAngle);
	}
	return dWidthWarp;
}

double CTextileBraid::GetHeightWarp() const {
	double dHeightWarp = 0;
	for (int i = 0; i < m_iNumWarpYarns; i++) {
		dHeightWarp += m_WarpYarnData[i].dSpacing*cos(m_dbraidAngle);
	}
	return dHeightWarp;
}

double CTextileBraid::GetWidthWeft() const {
	double dWidthWeft = 0;
	for (int i = 0; i < m_iNumWeftYarns; i++) {
		dWidthWeft += m_WeftYarnData[i].dSpacing*sin(m_dbraidAngle);
	}
	return dWidthWeft;
}

double CTextileBraid::GetHeightWeft() const {
	double dHeightWeft = 0;
	for (int i = 0; i < m_iNumWeftYarns; i++) {
		dHeightWeft += m_WeftYarnData[i].dSpacing*cos(m_dbraidAngle);
	}
	return dHeightWeft;
}

double TexGen::CTextileBraid::GetWidth() const
{
	double dWidth;
	dWidth = m_WarpYarnData[m_iNumWarpYarns - 1].dSpacing*cos(m_dbraidAngle)*m_iNumWarpYarns;
	return dWidth;
}

double TexGen::CTextileBraid::GetHeight() const
{
	double dHeight;
	dHeight = (m_WarpYarnData[m_iNumWarpYarns - 1].dSpacing*cos(m_dbraidAngle)*m_iNumWeftYarns)
		+ (m_WarpYarnData[m_iNumWarpYarns - 1].dSpacing*cos(m_dbraidAngle)*m_iNumWeftYarns);
	return dHeight;
}

/*void CTextileBraid::GetWarpSpacings() const
{
	m_WarpSpacing.clear();
	double sinAngle = sin(m_dbraidAngle);
	double cosAngle = cos(m_dbraidAngle);

	for (int i = 0; i < m_iNumWeftYarns; ++i)
	{
		XY WarpSpacing;
		WarpSpacing.x = m_WeftYarnData[i].dSpacing * cosAngle;
		WarpSpacing.y = m_WeftYarnData[i].dSpacing * sinAngle;
		m_WarpSpacing.push_back(WarpSpacing);
	}
}

void CTextileBraid::GetWeftSpacings() const
{
	m_WeftSpacing.clear();
	double sinAngle = sin(m_dbraidAngle);
	double cosAngle = cos(m_dbraidAngle);

	for (int i = 0; i < m_iNumWarpYarns; ++i)
	{
		XY WeftSpacing;
		WeftSpacing.x = m_WarpYarnData[i].dSpacing * cosAngle;
		WeftSpacing.y = m_WarpYarnData[i].dSpacing * sinAngle;
		m_WeftSpacing.push_back(WeftSpacing);
	}
}
*/
void CTextileBraid::SwapPosition(int x, int y)
{
	if (x<0 || x >= m_iNumWeftYarns || y<0 || y >= m_iNumWarpYarns)
	{
		TGERROR("Unable to swap positions, index out of range: " << x << ", " << y);
		return;
	}
	vector<bool> &Cell = GetCell(x, y);
	assert(Cell.size() == 2);
	Cell[0] = !Cell[0];
	Cell[1] = !Cell[1];
	m_bNeedsBuilding = true;
}

CDomainPlanes CTextileBraid::GetDefaultDomain(bool bSheared, bool bAddedHeight)
{
	XYZ Min, Max;
	double dGap = 0.0;
	if (bAddedHeight)
		dGap = 0.05*m_dFabricThickness;
	//Min.x = -10;
	//Min.y = -10;
	//Min.z = -5;
	//Max.x = 10;
	//Max.y = 10;
	//Max.z = 5;

	Min.x = 0;
	Min.y = -1*m_WarpYarnData[m_iNumWarpYarns - 1].dSpacing*cos(m_dbraidAngle)*m_iNumWeftYarns;
	//	Min.x = m_YYarnData[0].dSpacing;
	//	Min.y = m_XYarnData[0].dSpacing;
	Min.z = -dGap;
	Max.x = (m_WeftYarnData[m_iNumWeftYarns -1].dSpacing*sin(m_dbraidAngle)*m_iNumWarpYarns)+(m_WeftYarnData[m_iNumWeftYarns - 1].dSpacing*sin(m_dbraidAngle)*(m_iNumWeftYarns-1));
	Max.y =m_WarpYarnData[m_iNumWarpYarns - 1].dSpacing*cos(m_dbraidAngle)*m_iNumWeftYarns;
	Max.z = m_dFabricThickness + dGap;
	return CDomainPlanes(Min, Max);
}

void CTextileBraid::AssignDefaultDomain(bool bSheared, bool bAddedHeight)
{
	CDomainPlanes Domain = GetDefaultDomain(bSheared, bAddedHeight);
	AssignDomain(Domain);
}

double CTextileBraid::GetYarnSpacing() const
{
	double dSpacing = 0; 
	for (int i = 0; i < m_iNumWeftYarns; i++) {
		dSpacing += m_WeftYarnData[i].dSpacing;
	}
	for (int i = 0; i < m_iNumWarpYarns; i++) {
		dSpacing += m_WarpYarnData[i].dSpacing;
	}
	return dSpacing = dSpacing / (m_iNumWarpYarns + m_iNumWeftYarns);
	
}

double CTextileBraid::GetYarnWidth() const
{
	double dWidth = 0;
	for (int i = 0; i < m_iNumWeftYarns; i++) {
		dWidth += m_WeftYarnData[i].dWidth;
	}
	for (int i = 0; i < m_iNumWarpYarns; i++) {
		dWidth += m_WarpYarnData[i].dWidth;
	}
	return dWidth = dWidth / (m_iNumWarpYarns + m_iNumWeftYarns);
}

double CTextileBraid::GetWeftYarnWidth(int iIndex) const
{
	if (iIndex < 0 || iIndex >= m_iNumWeftYarns)
	{
		TGERROR("Unable to get yarn width, index out of range: " << iIndex);
		return 0;
	}
	return m_WeftYarnData[iIndex].dWidth;
}

double CTextileBraid::GetWarpYarnWidth(int iIndex) const
{
	if (iIndex < 0 || iIndex >= m_iNumWarpYarns)
	{
		TGERROR("Unable to get the yarn width, index out of range: " << iIndex);
		return 0;
	}
	return m_WarpYarnData[iIndex].dWidth;
}

double CTextileBraid::GetWeftYarnHeight(int iIndex) const
{
	if (iIndex < 0 || iIndex >= m_iNumWeftYarns)
	{
		TGERROR("Unable to get the yarn height, index out of range: " << iIndex);
	}
	return m_WeftYarnData[iIndex].dHeight;
}

double CTextileBraid::GetWarpYarnHeight(int iIndex) const
{
	if (iIndex < 0 || iIndex >= m_iNumWarpYarns)
	{
		TGERROR("Unable to get the yarn height, index out of range: " << iIndex);
	}
	return m_WarpYarnData[iIndex].dHeight;
}

double CTextileBraid::GetWeftYarnSpacing(int iIndex) const
{
	if (iIndex < 0 || iIndex >= m_iNumWeftYarns)
	{
		TGERROR("Unable to get the yarn height, index out of range: " << iIndex);
	}
	return m_WeftYarnData[iIndex].dSpacing;
}

double CTextileBraid::GetWarpYarnSpacing(int iIndex) const
{
	if (iIndex < 0 || iIndex >= m_iNumWarpYarns)
	{
		TGERROR("Unable to get the yarn height, index out of range: " << iIndex);
	}
	return m_WarpYarnData[iIndex].dSpacing;
}

double CTextileBraid::GetWeftGapSize(int iIndex) const
{
	if (iIndex < 0 || iIndex >= m_iNumWeftYarns)
	{
		TGERROR("Unable to get yarn spacing, index out of range: " << iIndex);
		return 0;
	}
	double dGapSize = m_WeftYarnData[iIndex].dSpacing;
	dGapSize -= 0.5*m_WeftYarnData[iIndex].dWidth;
	if (iIndex == m_iNumWeftYarns - 1)
		dGapSize -= 0.5*m_WeftYarnData[0].dWidth;
	else dGapSize -= 0.5*m_WeftYarnData[iIndex + 1].dWidth;
	return dGapSize;
}

double CTextileBraid::GetWarpGapSize(int iIndex) const
{
	if (iIndex < 0 || iIndex >= m_iNumWarpYarns)
	{
		TGERROR("Unable to get yarn spacing, index out of range: " << iIndex);
		return 0;
	}
	double dGapSize = m_WarpYarnData[iIndex].dSpacing;
	dGapSize -= 0.5*m_WarpYarnData[iIndex].dWidth;
	if (iIndex == m_iNumWarpYarns - 1)
		dGapSize -= 0.5*m_WarpYarnData[0].dWidth;
	else dGapSize -= 0.5*m_WarpYarnData[iIndex + 1].dWidth;
	return dGapSize;
}

void CTextileBraid::SetWarpYarnWidths(int iIndex, double dWidth)
{
	if (iIndex < 0 || iIndex >= m_iNumWarpYarns)
	{
		TGERROR("Unable to set yarn widths, index out of range: " << iIndex);
		return;
	}
	m_WarpYarnData[iIndex].dWidth = dWidth;
	m_bNeedsBuilding = true;
}

void CTextileBraid::SetWeftYarnWidths(int iIndex, double dWidth)
{
	if (iIndex < 0 || iIndex >= m_iNumWeftYarns)
	{
		TGERROR("Unable to set yarn widths, index out of range: " << iIndex);
		return;
	}
	m_WeftYarnData[iIndex].dWidth = dWidth;
	m_bNeedsBuilding = true;
}

void CTextileBraid::SetWarpYarnHeights(int iIndex, double dHeight)
{
	if (iIndex < 0 || iIndex >= m_iNumWarpYarns)
	{
		TGERROR("Unable to set yarn heights, index out of range: " << iIndex);
		return;
	}
	m_WarpYarnData[iIndex].dHeight = dHeight;
	m_bNeedsBuilding = true;
}

void CTextileBraid::SetWeftYarnHeights(int iIndex, double dHeight)
{
	if (iIndex < 0 || iIndex >= m_iNumWeftYarns)
	{
		TGERROR("Unable to set yarn heights, index out of range: " << iIndex);
		return;
	}
	m_WeftYarnData[iIndex].dHeight = dHeight;
	m_bNeedsBuilding = true;
}

void CTextileBraid::SetWarpYarnSpacings(int iIndex, double dSpacing)
{
	if (iIndex < 0 || iIndex >= m_iNumWarpYarns)
	{
		TGERROR("Unable to set yarn Spacing, index out of range: " << iIndex);
		return;
	}
	m_WarpYarnData[iIndex].dSpacing = dSpacing;
	m_bNeedsBuilding = true;
}

void CTextileBraid::SetWeftYarnSpacings(int iIndex, double dSpacing)
{
	if (iIndex < 0 || iIndex >= m_iNumWeftYarns)
	{
		TGERROR("Unable to set yarn Spacing, index out of range: " << iIndex);
		return;
	}
	m_WeftYarnData[iIndex].dSpacing = dSpacing;
	m_bNeedsBuilding = true;
}

void CTextileBraid::SetWeftYarnWidths(double dWidth)
{
	for (int i = 0; i < m_iNumWeftYarns; i++)
	{
		SetWeftYarnWidths(i, dWidth);
	}
}

void CTextileBraid::SetWarpYarnWidths(double dWidth)
{
	for (int i = 0; i < m_iNumWarpYarns; i++)
	{
		SetWarpYarnWidths(i, dWidth);
	}
}

void CTextileBraid::SetWeftYarnHeights(double dHeight)
{
	for (int i = 0; i < m_iNumWeftYarns; i++)
	{
		SetWeftYarnHeights(i, dHeight);
	}
}

void CTextileBraid::SetWarpYarnHeights(double dHeight)
{
	for (int i = 0; i < m_iNumWarpYarns; i++)
	{
		SetWarpYarnHeights(i, dHeight);
	}
}

void CTextileBraid::SetWeftYarnSpacings(double dSpacing)
{
	for (int i = 0; i < m_iNumWeftYarns; i++)
	{
		SetWeftYarnSpacings(i, dSpacing);
	}
}

void CTextileBraid::SetWarpYarnSpacings(double dSpacing)
{
	for (int i = 0; i < m_iNumWarpYarns; i++)
	{
		SetWarpYarnSpacings(i, dSpacing);
	}
}

void CTextileBraid::SetYarnWidths(double dWidth)
{
	SetWeftYarnWidths(dWidth);
	SetWarpYarnWidths(dWidth);
}

void CTextileBraid::SetYarnHeights(double dHeight)
{
	SetWeftYarnHeights(dHeight);
	SetWarpYarnHeights(dHeight);
}

void CTextileBraid::SetYarnSpacings(double dSpacing)
{
	SetWeftYarnSpacings(dSpacing);
	SetWarpYarnSpacings(dSpacing);
}

void CTextileBraid::SetThickness(double dThickness)
{
	m_dFabricThickness = dThickness;
	m_bNeedsBuilding = true;
}

void CTextileBraid::SetResolution(int iResolution)
{
	m_iResolution = iResolution;
	m_bNeedsBuilding = true;
}