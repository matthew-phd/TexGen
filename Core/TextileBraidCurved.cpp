#include "PrecompiledHeaders.h"
#include "TextileBraidCurved.h"
#include "SectionEllipse.h"
#include "SectionRotated.h"
#include "SectionPolygon.h"

using namespace TexGen;

CTextileBraidCurved::CTextileBraidCurved(int iNumWeftYarns, int iNumWarpYarns, double dWidth,
	double dHeight, double dThickness,
	double dRadius, double dHornGearVelocity, int iNumHornGear,
	double dVelocity, bool bCurved, bool bRefine)
	:CTextileBraid(iNumWeftYarns, iNumWarpYarns, dWidth, dHeight,dThickness, dRadius, dHornGearVelocity,
		iNumHornGear, dVelocity, bRefine)
	,m_bCurved(true)
{
	
}

CTextileBraidCurved::~CTextileBraidCurved(void)
{

}

bool CTextileBraidCurved::BuildTextile() const
{
	m_Yarns.clear();
	m_WarpYarns.clear();
	m_WeftYarns.clear();

	m_WarpYarns.resize(m_iNumWarpYarns);
	m_WeftYarns.resize(m_iNumWeftYarns);

	vector<vector<RThetaZ> >WeftPolarCoor;
	vector<vector<RThetaZ> >WarpPolarCoor;

	TGLOGINDENT("Building braid\"" << GetName() << "\"");

	vector<int> Yarns;
	WeftPolarCoor.resize(m_iNumWeftYarns);
	for (int i = 0; i < m_iNumWeftYarns; i++)
	{
		WeftPolarCoor[i].resize(m_iNumWarpYarns + 1);
	}

	double r, theta, z;
	double x, y;

	// add weft yarns (x yarns)
	int i, j, k, iYarn;
	double startr = m_dMandrel_Rad;
	double startTheta = 0;
	double starty = 0;

	for (i = 0; i < m_iNumWeftYarns; i++)
	{

		theta = startTheta;
		y = starty;
		Yarns.clear();
		for (j = 0; j <= m_iNumWarpYarns; j++)
		{
			const vector<PATTERNBIAX> &Cell = GetCell(j%m_iNumWarpYarns, i);
			if (j == 0)
			{
				for (k = 0; k < (int)Cell.size(); k++)
				{
					if (Cell[k] == PATTERN_WEFTYARN)
					{
						Yarns.push_back(AddYarn(CYarn()));
					}
				}
			}
			m_WeftYarns[i] = Yarns;
			iYarn = 0;
			r = startr;
			for (k = 0; k < (int)Cell.size(); k++)
			{
				if (Cell[k] == PATTERN_WEFTYARN)
				{
					WeftPolarCoor[i][j] = RThetaZ(r, theta, y);
					z = r * cos(theta);
					x = r * sin(theta);
					m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(sin(m_dbraidAngle), cos(m_dbraidAngle), (-x)/sqrt((r*r)-(x*x)))));
					iYarn++;
				}
				r += m_dFabricThickness / Cell.size();
			}
			if (j < m_iNumWarpYarns)
			{
				y += m_WarpYarnData[j].dSpacing*cos(m_dbraidAngle);
				theta += (m_WarpYarnData[j].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI);
			}
		}
		starty += m_WeftYarnData[i].dSpacing*cos(m_dbraidAngle);
		startTheta += -1 * (m_WeftYarnData[i].dSpacing*sin(m_dbraidAngle) / (PI*m_dMandrel_Rad * 2))*(2 * PI);
	}


	// Add the Warp Yarns

	WarpPolarCoor.resize(m_iNumWarpYarns);
	for (int i = 0; i < m_iNumWarpYarns; i++)
	{
		WarpPolarCoor[i].resize(m_iNumWeftYarns + 1);
	}
	startr = m_dMandrel_Rad;
	startTheta = 0;
	starty = 0;
	for (i = 0; i < m_iNumWarpYarns; i++)
	{

		theta = startTheta;
		y = starty;
		Yarns.clear();
		for (j = 0; j <= m_iNumWeftYarns; j++)
		{
			const vector<PATTERNBIAX> &Cell = GetCell(j%m_iNumWeftYarns, i);
			if (j == 0)
			{
				for (k = 0; k < (int)Cell.size(); k++)
				{
					if (Cell[k] == PATTERN_WARPYARN)
					{
						Yarns.push_back(AddYarn(CYarn()));
					}
				}
			}
			m_WarpYarns[i] = Yarns;
			iYarn = 0;
			r = startr;
			for (k = 0; k < (int)Cell.size(); k++)
			{
				if (Cell[k] == PATTERN_WARPYARN)
				{
					WarpPolarCoor[i][j] = RThetaZ(r, theta, y);
					z = r * cos(theta);
					x = -1 * r * sin(theta);
					m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(-1 * sin(m_dbraidAngle), cos(m_dbraidAngle),-((-x) / sqrt((r*r) - (x*x))))));
					iYarn++;
				}
				r += m_dFabricThickness / Cell.size();
			}
			if (j < m_iNumWeftYarns)
			{
				y += m_WeftYarnData[j].dSpacing*cos(m_dbraidAngle);
				theta += (m_WeftYarnData[j].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI);
			}
		}
		starty += m_WarpYarnData[i].dSpacing*cos(m_dbraidAngle);
		startTheta += -1 * (m_WarpYarnData[i].dSpacing*sin(m_dbraidAngle) / (PI*m_dMandrel_Rad * 2))*(2 * PI);
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
	double dWidth, dHeight, position;
	for (i = 0; i < m_iNumWeftYarns; ++i)
	{
		dWidth = m_WeftYarnData[i].dWidth;
		dHeight = m_WeftYarnData[i].dHeight;
		CSectionEllipse Section(dWidth, dHeight);
		CYarnSectionInterpPosition AngledYarnSection(true, true);
		for (int j = 0; j <= m_iNumWarpYarns; j++)
		{
			position = ((double)j / m_iNumWarpYarns);
			AngledYarnSection.AddSection(position, CSectionRotated(Section, -WeftPolarCoor[i][j].theta));
		}
		if (m_pSectionMesh)
			Section.AssignSectionMesh(*m_pSectionMesh);
		for (itpYarn = m_WeftYarns[i].begin(); itpYarn != m_WeftYarns[i].end(); ++itpYarn)
		{
			m_Yarns[*itpYarn].AssignSection((AngledYarnSection));

		}

	}
	for (i = 0; i < m_iNumWarpYarns; ++i)
	{
		dWidth = m_WarpYarnData[i].dWidth;
		dHeight = m_WarpYarnData[i].dHeight;
		CSectionEllipse Section(dWidth, dHeight);
		CYarnSectionInterpPosition AngledYarnSection(true, true);
		for (int j = 0; j < m_iNumWeftYarns + 1; j++)
		{
			position = (double)j / m_iNumWeftYarns;
			AngledYarnSection.AddSection(position, CSectionRotated(Section, WarpPolarCoor[i][j].theta));
		}
		if (m_pSectionMesh)
			Section.AssignSectionMesh(*m_pSectionMesh);
		for (itpYarn = m_WarpYarns[i].begin(); itpYarn != m_WarpYarns[i].end(); ++itpYarn)
		{
			m_Yarns[*itpYarn].AssignSection(AngledYarnSection);
		}
	}
	// Add repeats and set interpolation
	vector<CYarn>::iterator itYarn;
	for (itYarn = m_Yarns.begin(); itYarn != m_Yarns.end(); ++itYarn)
	{
		itYarn->AssignInterpolation(CInterpolationBezier());
		itYarn->SetResolution(m_iResolution);
		//itYarn->AddRepeat(XYZ(dWidthWeft, -dHeightWeft, 0));
		//itYarn->AddRepeat(XYZ(dWidthWarp, dHeightWarp, 0));
	}


	return true;
}

CDomainPlanes CTextileBraidCurved::GetDefaultDomain(bool bSheared, bool bAddedHeight)
{
	XYZ Min, Max;
	double dGap = 0.0;
	if (bAddedHeight)
		dGap = 0.05*m_dFabricThickness;
	RThetaZ PolarMin, PolarMax;
	//PolarMin.r = WeftPolarCoor[0][m_iNumWarpYarns].r;
	PolarMin.theta = -(m_WeftYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI)*m_iNumWeftYarns;
	//PolarMin.z =  WeftPolarCoor[0][0].z;
	//PolarMax.r = WarpPolarCoor[0][m_iNumWeftYarns].r;
	PolarMax.theta = (m_WarpYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI)*m_iNumWarpYarns;
	//PolarMax.z = (WarpPolarCoor[1][1].z- WeftPolarCoor[0][0].z)*m_iNumWarpYarns;
	Min.y = 0;
	Max.y = m_WeftYarnData[0].dSpacing*cos(m_dbraidAngle) * 2* m_iNumWeftYarns;
	Min.x = m_dRadius*1000*sin(PolarMin.theta);
	Max.x = m_dRadius*1000*sin(PolarMax.theta);
	
	Min.z = m_dRadius*1000*cos(PolarMin.theta);
	Max.z = m_dRadius*1000+m_dFabricThickness +dGap;

	
	return CDomainPlanes(Min, Max);
}