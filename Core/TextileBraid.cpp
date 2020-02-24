#include "PrecompiledHeaders.h"
#include "TextileBraid.h"
#include "SectionEllipse.h"
#include "SectionRotated.h"
#include "SectionPolygon.h"
#include "SectionLenticular.h"
#include <random>

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
	, m_dMandrel_Rad(dRadius*1000+(dThickness/4)) // radius of the mandrel braided onto + 1
{
	// calculate the braid angle based on machine inputs
	m_dbraidAngle = atan((2 * m_dHornGearVelocity * m_dRadius) / (m_iNumHornGear*m_dVelocity));
	double vol_fraction = (1 - (((dWidth / 1000)*(2*iNumHornGear)) / (4 * PI*dRadius*cos(m_dbraidAngle))));
	m_dCoverFactor = 1 - pow(vol_fraction, 2.0);
	if (m_dCoverFactor > 1)
	{
		TGERROR("Cover Factor greater than one");
	}

	// Solve the quadractic equation to calculate the distance between yarns
	double a = m_dCoverFactor;
	double b = (2 * m_dCoverFactor*(dWidth / 1000.0)) - (2 * (dWidth / 1000.0));
	double c = (m_dCoverFactor - 1)*((dWidth / 1000.0)*(dWidth / 1000.0));

	double gap, dSpacing;
	gap = ((-b) + sqrt((b*b) - (4 * a*c))) / (2 * a);
	dSpacing = ((gap * 1000.0)/ cos(m_dbraidAngle - 0.7853)) + (dWidth/cos(m_dbraidAngle-0.7853));

	m_Pattern.resize(iNumWeftYarns*iNumWarpYarns);
	YARNDATA YarnData;
	YarnData.dSpacing = dSpacing;
	YarnData.dWidth = dWidth;
	YarnData.dHeight = m_dFabricThickness / 2.0;
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

		// Calculate the z values for a curved 

		// double dUnit_Cell_Width = (m_WeftYarnData[m_iNumWeftYarns - 1].dSpacing*sin(m_dbraidAngle)*m_iNumWarpYarns) + (m_WeftYarnData[m_iNumWeftYarns - 1].dSpacing*sin(m_dbraidAngle)*(m_iNumWeftYarns));
		// double dMandrel_Circum = PI * (2 * m_dMandrel_Rad);
		// double theta = (dUnit_Cell_Width / dMandrel_Circum) * (2 * PI);
		// double a = dUnit_Cell_Width / 2;

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
				//z = sqrt((m_dMandrel_Rad*m_dMandrel_Rad) - ((x - a)*(x - a)));
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
				//z = sqrt((m_dMandrel_Rad*m_dMandrel_Rad) - ((x - a)*(x - a)));
				for (k = 0; k < (int)Cell.size(); ++k)
				{
					if (Cell[k] == PATTERN_WARPYARN)
					{
						m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(sin(m_dbraidAngle), -1 * cos(m_dbraidAngle), 0)));
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
			CSectionLenticular Section(dWidth, dHeight);
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
			CSectionLenticular Section(dWidth, dHeight);
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
		{
			//AdjustSpacing();
			return true;
		}

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

CDomainPlanes CTextileBraid::GetDefaultDomain( bool bAddedHeight)
{
	
		XYZ Min, Max;
		double dGap = 0.0;
		if (bAddedHeight)
			dGap = 0.05*m_dFabricThickness;

		Min.x = 0;
		Min.y = -1 * m_WarpYarnData[m_iNumWarpYarns - 1].dSpacing*cos(m_dbraidAngle)*m_iNumWeftYarns;
		Min.z = -dGap;
		// Min.z = 11;
		Max.x = (m_WeftYarnData[m_iNumWeftYarns - 1].dSpacing*sin(m_dbraidAngle)*m_iNumWarpYarns) + (m_WeftYarnData[m_iNumWeftYarns - 1].dSpacing*sin(m_dbraidAngle)*(m_iNumWeftYarns));
		Max.y = m_WarpYarnData[m_iNumWarpYarns - 1].dSpacing*cos(m_dbraidAngle)*m_iNumWarpYarns;
		Max.z = m_dFabricThickness + dGap;
		//Max.z = 16.6;
		return CDomainPlanes(Min, Max);
	
}

void CTextileBraid::AssignDefaultDomain(bool bSheared, bool bAddedHeight)
{
	CDomainPlanes Domain = GetDefaultDomain(bAddedHeight);
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
		CSectionLenticular Section(dWidth, dHeight);
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
		CSectionLenticular Section(dWidth, dHeight);
		if (m_pSectionMesh)
			Section.AssignSectionMesh(*m_pSectionMesh);
		for (itpYarn = m_WarpYarns[i].begin(); itpYarn != m_WarpYarns[i].end(); itpYarn++)
		{
			m_Yarns[*itpYarn].AssignSection(CYarnSectionConstant(Section));
		}
	}
}

void CTextileBraid::CorrectInterference() const
{
	TGLOGINDENT("Correcting interference");

	vector<vector<int> > *pTransverseYarns;
	vector<vector<int> > *pLongitudinalYarns;
	int iTransverseNum;
	int iLongitudinalNum;
	int iDir;
	int i, j, k;
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
	vector<double> Modifiers;
	vector<vector<vector<double> > > YarnSectionModifiers;
	YarnSectionModifiers.resize(m_Yarns.size());

	for (iDir = 0; iDir < 2; ++iDir)
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
		for (i = 0; i < iTransverseNum; ++i)
		{
			TransverseYarnsMesh.Clear();
			for (itpYarn = (*pTransverseYarns)[i].begin(); itpYarn != (*pTransverseYarns)[i].end(); ++itpYarn)
			{
				m_Yarns[*itpYarn].AddSurfaceToMesh(TransverseYarnsMesh, RepeatLimits);
			}
			TransverseYarnsMesh.Convert3Dto2D();
			TransverseYarnsMesh.ConvertQuadstoTriangles();
			//			CElementsOctree Octree;
			//			TransverseYarnsMesh.BuildElementOctree(CMesh::TRI, Octree);
			for (j = 0; j < iLongitudinalNum; ++j)
			{
				for (itpYarn = (*pLongitudinalYarns)[j].begin(); itpYarn != (*pLongitudinalYarns)[j].end(); ++itpYarn)
				{
					YarnPosInfo.iSection = i;
					YarnPosInfo.dSectionPosition = 0;
					YarnPosInfo.SectionLengths = m_Yarns[*itpYarn].GetYarnSectionLengths();

					pInterpolation = m_Yarns[*itpYarn].GetInterpolation();
					Node = pInterpolation->GetNode(m_Yarns[*itpYarn].GetMasterNodes(), i, 0);
					Up = Node.GetUp();
					Side = CrossProduct(Node.GetTangent(), Up);

					pYarnSection = m_Yarns[*itpYarn].GetYarnSection();
					vector<XY> Points = pYarnSection->GetSection(YarnPosInfo, m_Yarns[*itpYarn].GetNumSectionPoints());
					Centre = m_Yarns[*itpYarn].GetMasterNodes()[i].GetPosition();
					vector<XY>::iterator itPoint;
					Modifiers.clear();
					for (itPoint = Points.begin(); itPoint != Points.end(); itPoint++) {
						P = itPoint->x * Side + itPoint->y * Up + Centre;
						if (TransverseYarnsMesh.IntersectLine(Centre, P, Intersections, make_pair(true, false)))
						{
							double dU = Intersections[0].first;
							XYZ Normal = Intersections[0].second;
							double dProjectedGap = m_dGapSize / DotProduct(Normal, Centre - P);
							dU -= 0.5 * dProjectedGap;
							if (dU > 1)
								dU = 1;
							if (dU < 0)
								dU = 0;
							Modifiers.push_back(dU);
						}
						else
							Modifiers.push_back(1);
					}
					YarnSectionModifiers[*itpYarn].push_back(Modifiers);
				}
			}
		}
	}
	vector<XY> Points;
	for (i = 0; i < (int)m_Yarns.size(); i++) 
	{
		CYarnSectionInterpNode NewYarnSection(false, true);
		pYarnSection = m_Yarns[i].GetYarnSection();
		YarnPosInfo.SectionLengths = m_Yarns[i].GetYarnSectionLengths();
		// Add Sections at the nodes
		YarnPosInfo.dSectionPosition = 0;
		for (j = 0; j < (int)YarnSectionModifiers[i].size(); j++)
		{
			YarnPosInfo.iSection = j;
			Points = pYarnSection->GetSection(YarnPosInfo, YarnSectionModifiers[i][j].size());
			for (k = 0; k < (int)Points.size(); k++)
			{
				Points[k] *= YarnSectionModifiers[i][j][k];
			}
			CSectionPolygon Section(Points);
			if (m_pSectionMesh)
				Section.AssignSectionMesh(*m_pSectionMesh);
			NewYarnSection.AddSection(Section);
		}
		NewYarnSection.AddSection(NewYarnSection.GetNodeSection(0));

		// Add Sections between the nodes (Not necessary for sections that dont cross)
		YarnPosInfo.dSectionPosition = 0.5;
		for (j = 0; j<(int)YarnSectionModifiers[i].size(); ++j)
		{
			if (NeedsMidSection(i, j))
			{
				YarnPosInfo.iSection = j;

				Points = pYarnSection->GetSection(YarnPosInfo, YarnSectionModifiers[i][j].size());
				for (k = 0; k<(int)Points.size(); ++k)
				{
					Points[k] *= min(YarnSectionModifiers[i][j][k], YarnSectionModifiers[i][(j + 1) % YarnSectionModifiers[i].size()][k]);
				}
				CSectionPolygon Section(Points);
				if (m_pSectionMesh)
					Section.AssignSectionMesh(*m_pSectionMesh);
				NewYarnSection.InsertSection(j, 0.5, Section);
			}
		}
		m_Yarns[i].AssignSection(NewYarnSection);

	}


}

bool CTextileBraid::NeedsMidSection(int iYarn, int iSection) const
{
	int i, j;
	for (i = 0; i<(int)m_WeftYarns.size(); ++i)
	{
		for (j = 0; j<(int)m_WeftYarns[i].size(); ++j)  // Steps through layers - why does it need to do this if only used for 2D weaves?
		{
			if (m_WeftYarns[i][j] == iYarn)
			{
				int iNextSection = (iSection + 1) % m_WarpYarns.size();
				return !(GetCell(iSection, i) == GetCell(iNextSection, i));
			}
		}
	}
	for (i = 0; i<(int)m_WarpYarns.size(); ++i)
	{
		for (j = 0; j<(int)m_WarpYarns[i].size(); ++j)
		{
			if (m_WarpYarns[i][j] == iYarn)
			{
				int iNextSection = (iSection + 1) % m_WeftYarns.size();
				return !(GetCell(i, iSection) == GetCell(i, iNextSection));
			}
		}
	}
	return true;
}

bool CTextileBraid::AdjustSectionsForRotation(bool bPeriodic) const
{
	int i, j;

	CYarn *pYarn;
	XYZ PrevXPos, NextXPos;
	XYZ PrevYPos, NextYPos;
	XYZ Up;
	CNode NewNode;

	// Assign more adequate cross sections
	double dWidth, dHeight;
	int iPrevYarnx, iPrevYarny;
	int iNextYarnx, iNextYarny;
	//	int iNextCrossx, iNextCrossy;

	double dAngle;
	int iNumYarns, iYarnLength;
	int iDirection;
	int x, y;

	// First loop for Y yarns, second loop for X yarns
	for (iDirection = 0; iDirection<2; ++iDirection)
	{
		if (iDirection == 0)
		{
			iNumYarns = m_iNumWarpYarns;
			iYarnLength = m_iNumWeftYarns;
		}
		else
		{
			iNumYarns = m_iNumWeftYarns;
			iYarnLength = m_iNumWarpYarns;
		}
		int start = 0;
		if (!bPeriodic)
		{
			start = 1;
		}

		for (i = start; i < iNumYarns; ++i)
		{
			if (iDirection == 0)
			{
				iPrevYarnx = i - 1;
				if (iPrevYarnx < 0)
					iPrevYarnx += iNumYarns;
				if (bPeriodic)
					iNextYarnx = (i + 1) % iNumYarns;
				else
					iNextYarnx = i + 1;  //  Assumes that one extra node than number of yarns

										 // The angle is the maximum rotation angle to apply to the yarn at points where it needs
										 // rotating, specified in radians.
				dAngle = atan2(0.5*m_dFabricThickness, m_WarpYarnData[iPrevYarnx].dSpacing + m_WarpYarnData[i].dSpacing);
				// Get the yarn width and height for this X yarn
				dWidth = m_WarpYarnData[i].dWidth;
				dHeight = m_WarpYarnData[i].dHeight;
				// Get a pointer to the current yarn
				pYarn = &m_Yarns[m_WarpYarns[i][0]];
				// Used for getting the cell coordinates
				x = i;
			}
			else
			{
				iPrevYarny = i - 1;
				if (iPrevYarny < 0)
					iPrevYarny += iNumYarns;
				if (bPeriodic)
					iNextYarny = (i + 1) % iNumYarns;  //  Assumes that one extra node than number of yarns
				else
					iNextYarny = i + 1;

				// The angle is the maximum rotation angle to apply to the yarn at points where it needs
				// rotating, specified in radians.
				dAngle = atan2(0.5*m_dFabricThickness, m_WeftYarnData[iPrevYarny].dSpacing + m_WeftYarnData[i].dSpacing);
				// Get the yarn width and height for this X yarn
				dWidth = m_WeftYarnData[i].dWidth;
				dHeight = m_WeftYarnData[i].dHeight;
				// Get a pointer to the current yarn
				pYarn = &m_Yarns[m_WeftYarns[i][0]];
				// Used for getting the cell coordinates
				y = i;
			}

			CSectionLenticular DefaultEllipseSection(dWidth, dHeight);

			// Get a copy of the yarn sections that is applied to the nodes
			if (pYarn->GetYarnSection()->GetType() != "CYarnSectionInterpNode")
				return false;
			CYarnSectionInterpNode* pYarnSection = (CYarnSectionInterpNode*)pYarn->GetYarnSection()->Copy();
			//			CYarnSectionInterpNode YarnSection(true, true);
			int iRot;		// Should have 1 of three values (-1, 0 or 1). -1 (rotation to the right), 0 (no rotation), 1 (rotation to the left)
			for (j = 0; j<iYarnLength; ++j)
			{
				if (iDirection == 0)
				{
					// Set the parameters which will be used by GetCell for traversing an X yarn
					y = iPrevYarny = iNextYarny = j;
				}
				else
				{
					// Set the parameters which will be used by GetCell for traversing a Y yarn
					x = iPrevYarnx = iNextYarnx = j;
				}
				// If the yarns on either side are the same then no rotation should occur
				if (GetCell(iPrevYarnx, iPrevYarny) == GetCell(iNextYarnx, iNextYarny))
					iRot = 0;
				else if (GetCell(iPrevYarnx, iPrevYarny)[0] == PATTERN_WARPYARN)
					iRot = -1;	// Rotate to the right
				else
					iRot = 1;	// Rotate to the left

				CSectionLenticular* EllipseSection = NULL;
				if (pYarnSection->GetNodeSection(j).GetType() == "CSectionEllipse")
					EllipseSection = (CSectionLenticular*)pYarnSection->GetNodeSection(j).Copy();
				else
					EllipseSection = (CSectionLenticular*)DefaultEllipseSection.Copy();
				// Assign section based on the rotation it should have
				switch (iRot)
				{
				case 0:
					//					pYarnSection->ReplaceSection(j, EllipseSection);
					break;
				case -1:
					pYarnSection->ReplaceSection(j, CSectionRotated(*EllipseSection, dAngle));
					break;
				case 1:
					pYarnSection->ReplaceSection(j, CSectionRotated(*EllipseSection, -dAngle));
					break;
				}
				delete EllipseSection;
			}

			// Assign the same section to the end as at the start (periodic yarns)
			if (bPeriodic)
				pYarnSection->ReplaceSection(j, pYarnSection->GetNodeSection(0));

			/*			// Now additional sections will be added between cross overs
			for (j=0; j<iYarnLength; ++j)
			{
			if (iDirection == 0)
			{
			// Set the parameters which will be used by GetCell for traversing an X yarn
			y = iPrevYarny = iNextYarny = j;
			iNextCrossx = x;
			iNextCrossy = (y+1)%iYarnLength;
			}
			else
			{
			// Set the parameters which will be used by GetCell for traversing a Y yarn
			x = iPrevYarnx = iNextYarnx = j;
			iNextCrossx = (x+1)%iYarnLength;
			iNextCrossy = y;
			}
			// If the yarn is going from the top to the bottom or vice versa, an additional section
			// is placed half way between the two
			if (GetCell(x, y) != GetCell(iNextCrossx, iNextCrossy))
			{
			YarnSection.InsertSection(j, 0.5, EllipseSection);
			}
			}*/

			pYarn->AssignSection(*pYarnSection);
			delete pYarnSection;
		}
	}

	return true;
}

void CTextileBraid::Refine(bool bCorrectWidths, bool bPeriodic) const
{
	if (bCorrectWidths)
	{
		CorrectBraidYarnWidths();
		CorrectInterference();
		AdjustSectionsForRotation(bPeriodic);
		CorrectInterference();
	}
}

/*void CTextileBraid::AdjustSpacing() const
{
	// from study to show equation for average yarn spacing-std y=0.1877x+0.1928
	double std = (0.1877*m_WeftYarnData[0].dSpacing) + 0.1928;
	default_random_engine generator;
	normal_distribution<double> distribution(m_WeftYarnData[0].dSpacing, std);
	CYarn *pYarn;
	pYarn = &m_Yarns[m_WeftYarns[0][0]];
	CNode* Nodes = (CNode*)pYarn->GetNode(0);
	vector<double> spacing;
	spacing.resize(10);

	for (int i = 0; i < 10; i++)
	{
		double number = distribution(generator);
		if (number > m_WeftYarnData[0].dWidth)
			spacing[i] = number;

	}

}*/

