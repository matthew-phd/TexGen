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
	, m_dbraidAngle((PI/2)-dBraidAngle)
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
					m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(cos(m_dbraidAngle), sin(m_dbraidAngle), 0)));
					++iYarn;
				}
				z += m_dFabricThickness / Cell.size();
			}
			if (j < m_iNumWarpYarns)
			{
				x += m_WarpYarnData[j].dSpacing*cos(m_dbraidAngle);
				y += m_WeftYarnData[j].dSpacing*sin(m_dbraidAngle);
				
			}
			
		}
		startx += m_WarpYarnData[i].dSpacing*cos(m_dbraidAngle);
		starty += m_WarpYarnData[i].dSpacing*sin(m_dbraidAngle)*-1;
		
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
					m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(cos(m_dbraidAngle), -1*sin(m_dbraidAngle), 0)));
					++iYarn;
				}
				z += m_dFabricThickness / Cell.size();
			}
			if (i < m_iNumWeftYarns)
			{
				y -= m_WeftYarnData[i].dSpacing*sin(m_dbraidAngle);
				x += m_WarpYarnData[i].dSpacing*cos(m_dbraidAngle);
				
			}
			
		}
		startx += m_WeftYarnData[j].dSpacing*cos(m_dbraidAngle);
		starty += m_WeftYarnData[j].dSpacing*sin(m_dbraidAngle);
		
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
		itYarn->AddRepeat(XYZ(dWidthWarp, -dHeightWarp, 0));
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
		dWidthWarp += m_WarpYarnData[i].dSpacing*cos(m_dbraidAngle);
	}
	return dWidthWarp;
}

double CTextileBraid::GetHeightWarp() const {
	double dHeightWarp = 0;
	for (int i = 0; i < m_iNumWeftYarns; i++) {
		dHeightWarp += m_WeftYarnData[i].dSpacing*sin(m_dbraidAngle);
	}
	return dHeightWarp;
}

double CTextileBraid::GetWidthWeft() const {
	double dWidthWeft = 0;
	for (int i = 0; i < m_iNumWeftYarns; i++) {
		dWidthWeft += m_WeftYarnData[i].dSpacing*cos(m_dbraidAngle);
	}
	return dWidthWeft;
}

double CTextileBraid::GetHeightWeft() const {
	double dHeightWeft = 0;
	for (int i = 0; i < m_iNumWeftYarns; i++) {
		dHeightWeft += m_WarpYarnData[i].dSpacing*sin(m_dbraidAngle);
	}
	return dHeightWeft;
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
	Min.x = -10;
	Min.y = -10;
	Min.z = -5;
	Max.x = 10;
	Max.y = 10;
	Max.z = 5;

	//Min.x = -0.5*m_WarpYarnData[m_iNumWarpYarns - 1].dSpacing;
	//Min.y = -0.5*m_WeftYarnData[m_iNumWeftYarns - 1].dSpacing;
	//	Min.x = m_YYarnData[0].dSpacing;
	//	Min.y = m_XYarnData[0].dSpacing;
	//Min.z = -dGap;
	//Max.x = Min.x + GetWidth();
	//Max.y = Min.y + GetHeight();
	//Max.z = m_dFabricThickness + dGap;
	return CDomainPlanes(Min, Max);
}

void CTextileBraid::AssignDefaultDomain(bool bSheared, bool bAddedHeight)
{
	CDomainPlanes Domain = GetDefaultDomain(bSheared, bAddedHeight);
	AssignDomain(Domain);
}
