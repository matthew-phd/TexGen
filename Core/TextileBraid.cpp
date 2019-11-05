#include "PrecompiledHeaders.h"
#include "TextileBraid.h"
#include "SectionEllipse.h"
#include "SectionRotated.h"
#include "SectionPolygon.h"

using namespace TexGen;


CTextileBraid::CTextileBraid(int iNumWeftYarns, int iNumWarpYarns, double dWidth, 
	double dHeight, double dThickness, double dRadius,
	double dHornGearVelocity, int iNumHornGear, double dVelocity, bool bRefine)
	: m_iNumWeftYarns(iNumWeftYarns)
	, m_iNumWarpYarns(iNumWarpYarns)
	, m_dGapSize(0)
	, m_dFabricThickness(dThickness)
	, m_iResolution(40)
	, m_dRadius(dRadius)
	, m_dHornGearVelocity(dHornGearVelocity)
	, m_iNumHornGear(iNumHornGear)
	, m_dVelocity(dVelocity)
	, m_bRefine(bRefine)
{
	// calculate the braid angle based on machine inputs
	m_dbraidAngle = atan((2 * m_dHornGearVelocity * m_dRadius) / (m_iNumHornGear*m_dVelocity));
	double vol_fraction = (1 - (((dWidth / 1000)*iNumHornGear) / (4 * PI*dRadius*cos(m_dbraidAngle))));
	m_dCoverFactor = 1 - pow(vol_fraction, 2.0);
	if (m_dCoverFactor > 1)
	{
		TGERROR("Cover Factor greater than one");
	}

	// Solve the quadractic equation to calculate the distance between yarns
	double a = m_dCoverFactor;
	double b = (2 * m_dCoverFactor*(dWidth / 1000)) - (2 * (dWidth / 1000));
	double c = (m_dCoverFactor - 1)*((dWidth / 1000)*(dWidth / 1000));

	double gap, dSpacing;
	gap = ((-b) + sqrt((b*b) - (4 * a*c))) / (2 * a);
	dSpacing = ((gap * 1000)/ cos(m_dbraidAngle - 0.7853)) + (dWidth/cos(m_dbraidAngle-0.7853));

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
	if (!m_bRefine)
		return true;
	Refine(true); 

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

void CTextileBraid::SetGapSize(double dGapSize)
{
	m_dGapSize = dGapSize;
	m_bNeedsBuilding = true;
}

void CTextileBraid::CorrectBraidYarnWidths() const
{

	TGLOGINDENT("Adjusting yarn wdiths for \"" << GetName() << "\" with gap size of " << m_dGapSize); 
	
	vector<vector<int> > *pTransverseYarns;
	vector<vector<int> > *pLongitudinalYarns;
	int iTransverseNum;
	int iLongitudinalNum;
	int iDir;
	int i, j;
	CMesh TransverseYarnsMesh;
	vector<int>::iterator itpYarn;
	vector<pair<int, int> > RepeatLimits;
	vector<pair<double, XYZ> > Intersections;
	XYZ Centre, P;
	const CYarnSection* pYarnSection;
	const CInterpolation* pInterpolation;
	CSlaveNode Node;
	XYZ Side, Up;
	YARN_POSITION_INFORMATION YarnPosInfo;

	RepeatLimits.resize(2, pair<int, int>(-1, 0));
	vector<double> YarnMaxWidth;
	YarnMaxWidth.resize(m_Yarns.size(), -1);

	// Find the max width of the yarns....
	for (iDir = 0; iDir < 2; iDir++)
	{
		switch (iDir)
		{
		case 0:
			pTransverseYarns = &m_WarpYarns;
			pLongitudinalYarns = &m_WeftYarns;
			iTransverseNum = m_iNumWarpYarns;
			iLongitudinalNum = m_iNumWeftYarns;
			break;
		case 1:
			pTransverseYarns = &m_WeftYarns;
			pLongitudinalYarns = &m_WarpYarns;
			iTransverseNum = m_iNumWeftYarns;
			iLongitudinalNum = m_iNumWarpYarns;
			break;
		}
		for (i = 0; i < iTransverseNum; i++)
		{
			TransverseYarnsMesh.Clear();
			for (itpYarn = (*pTransverseYarns)[i].begin(); itpYarn != (*pTransverseYarns)[i].end(); itpYarn++)
			{
				m_Yarns[*itpYarn].AddSurfaceToMesh(TransverseYarnsMesh, RepeatLimits);
			}
			TransverseYarnsMesh.Convert3Dto2D();
			TransverseYarnsMesh.ConvertQuadstoTriangles();
			for (j = 0; j < iLongitudinalNum; j++)
			{
				for (itpYarn = (*pLongitudinalYarns)[j].begin(); itpYarn != (*pLongitudinalYarns)[j].end(); itpYarn++)
				{
					YarnPosInfo.iSection = i;
					YarnPosInfo.dSectionPosition = 0;
					YarnPosInfo.SectionLengths = m_Yarns[*itpYarn].GetYarnSectionLengths();
					pInterpolation = m_Yarns[*itpYarn].GetInterpolation();
					Node = pInterpolation->GetNode(m_Yarns[*itpYarn].GetMasterNodes(), i, 0);
					Up = Node.GetUp();
					Side = CrossProduct(Node.GetTangent(), Up);

					pYarnSection = m_Yarns[*itpYarn].GetYarnSection();

					vector<XY> Points = pYarnSection->GetSection(YarnPosInfo, 2);
					Centre = m_Yarns[*itpYarn].GetMasterNodes()[i].GetPosition();
					vector<XY>::iterator itPoint;
					for (itPoint = Points.begin(); itPoint != Points.end(); itPoint++)
					{
						P = itPoint->x *Side + itPoint->y*Up + Centre;
						// Find intersection of the side points of longitudinal yarn with transvery yarn meshes
						if (TransverseYarnsMesh.IntersectLine(Centre, P, Intersections, make_pair(true, false)))
						{
							double dU = Intersections[0].first;
							XYZ Normal = Intersections[0].second;
							double dProjectedGap = m_dGapSize / DotProduct(Normal, Centre - P);
							dU -= 0.5*dProjectedGap;
							if (dU < 0)
								dU = 0;
							if (dU < 1)
							{
								double dWidth = 2 * GetLength(*itPoint) * dU;
								if (YarnMaxWidth[*itpYarn] < 0 || dWidth < YarnMaxWidth[*itpYarn])
								{
									YarnMaxWidth[*itpYarn] = dWidth;
								}
							}
						}
					}
				}
			}
		}
	}
	// Adjust Yarn Widths
	double dWidth, dHeight;
	for (int i = 0; i < m_iNumWeftYarns; i++)
	{
		for (itpYarn = m_WeftYarns[i].begin(); itpYarn != m_WeftYarns[i].end(); itpYarn++)
		{
			if (YarnMaxWidth[*itpYarn] >= 0)
			{
				if (m_WeftYarnData[i].dWidth > YarnMaxWidth[*itpYarn])
				{
					TGLOG("Changing Weft Yarn wdith " << i << " from " << m_WeftYarnData[i].dWidth << " to " << YarnMaxWidth[*itpYarn]);
					m_WeftYarnData[i].dWidth = YarnMaxWidth[*itpYarn];
				}
			}
		}
	}
	for (int i = 0; i < m_iNumWarpYarns; i++)
	{
		for (itpYarn = m_WarpYarns[i].begin(); itpYarn != m_WarpYarns[i].end(); itpYarn++)
		{
			if (YarnMaxWidth[*itpYarn] >= 0)
			{
				if (m_WarpYarnData[i].dWidth > YarnMaxWidth[*itpYarn])
				{
					TGLOG("Changing Warp Yarn wdith " << i << " from " << m_WarpYarnData[i].dWidth << " to " << YarnMaxWidth[*itpYarn]);
					m_WarpYarnData[i].dWidth = YarnMaxWidth[*itpYarn];
				}
			}
		}
	}
	// Assign new sections to yarns
	for (int i = 0; i < m_iNumWeftYarns; i++)
	{
		dWidth = m_WeftYarnData[i].dWidth;
		dHeight = m_WeftYarnData[i].dHeight;
		CSectionEllipse Section(dWidth, dHeight);
		if (m_pSectionMesh)
			Section.AssignSectionMesh(*m_pSectionMesh);
		for (itpYarn = m_WeftYarns[i].begin(); itpYarn != m_WeftYarns[i].end(); itpYarn++)
		{
			m_Yarns[*itpYarn].AssignSection(CYarnSectionConstant(Section));
		}
	}
	for (int i = 0; i < m_iNumWarpYarns; i++)
	{
		dWidth = m_WarpYarnData[i].dWidth;
		dHeight = m_WarpYarnData[i].dHeight;
		CSectionEllipse Section(dWidth, dHeight);
		if (m_pSectionMesh)
			Section.AssignSectionMesh(*m_pSectionMesh);
		for (itpYarn = m_WarpYarns[i].begin(); itpYarn != m_WarpYarns[i].end(); itpYarn++)
		{
			m_Yarns[*itpYarn].AssignSection(CYarnSectionConstant(Section));
		}
	}
}

void CTextileBraid::Refine(bool bCorrectWidths) const
{
	if (bCorrectWidths)
	{
		CorrectBraidYarnWidths();
	}
}