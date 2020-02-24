#include "PrecompiledHeaders.h"
#include "TextileBraidCurved.h"
#include "SectionEllipse.h"
#include "SectionRotated.h"
#include "SectionPolygon.h"
#include "DomainPrism.h"

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
		CheckUpVectors(i);
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
		CheckUpVectors(i+m_iNumWeftYarns);
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
			double rotation = GetNodeRotation(i, j);
			AngledYarnSection.AddSection(position, CSectionRotated(Section, -rotation));
		
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
			double rotation = GetNodeRotation(i + m_iNumWeftYarns, j);
			AngledYarnSection.AddSection(position, CSectionRotated(Section, -rotation));
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
		//itYarn->AddRepeat(XYZ(dWidthWarp, dHeightWarp, ));
		//itYarn->AddRotationalRepeat(XYZ(dWidthWarp, dHeightWarp,0),0.45);
	}


	return true;
}

double CTextileBraidCurved::Angle()
{
	RThetaZ Min, Max;
	Min.theta = -(m_WeftYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI)*m_iNumWeftYarns;
	Max.theta = (m_WarpYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI)*m_iNumWarpYarns;

	double angle = Max.theta - Min.theta;

	return angle; 

}

CDomainPrism CTextileBraidCurved::GetDefaultCurvedDomain( bool bAddedHeight)
{
	vector<XY> points;
	double dGap = 0.0;
	if (bAddedHeight)
		dGap = 0.2*m_dFabricThickness;
	double angle = Angle()/21;
	double theta;
	
	theta = -(m_WeftYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI)*m_iNumWeftYarns;
	double radius =1000* m_dRadius-dGap;
	for (int i = 0; i < 21; i++)
	{
		points.push_back(XY((sin(theta)*radius), (cos(theta)*radius)));
		theta = theta + angle;

	}
	radius = (1000*m_dRadius) + m_dFabricThickness + dGap;
	theta = theta - angle;
	for (int i = 0; i < 21; i++)
	{
		points.push_back(XY((sin(theta)*radius), (cos(theta)*radius)));
		theta = theta - angle;

	}

	double MaxY = m_WeftYarnData[0].dSpacing*cos(m_dbraidAngle) * 2 * m_iNumWeftYarns;
	return CDomainPrism(points,XYZ(0, 0, 0), XYZ(0, MaxY, 0));
	/*vector<XY> points;
	points.push_back(XY(-10, -10));
	points.push_back(XY(20, -10));
	points.push_back(XY(20, 20));
	points.push_back(XY(-10, 20));
	//points.push_back(XY(1, -1))
	return CDomainPrism(points, XYZ(0, 0, 0), XYZ(0, 30, 0));*/
}

void CTextileBraidCurved::AssignDefaultDomain(bool bAddedHeight)
{
	CDomainPrism Domain = GetDefaultCurvedDomain(bAddedHeight);
	Domain.GeneratePlanes();
	AssignDomain(Domain);
}

XYZ CTextileBraidCurved::GetUpVector(XYZ CheckNodes[], bool bYarn) const
{
	double dHorz, dz;
	dz = CheckNodes[2].z - CheckNodes[0].z;

	if (bYarn == PATTERN_WEFTYARN)
		dHorz = CheckNodes[2].x - CheckNodes[0].x;
	else
		dHorz = CheckNodes[2].y - CheckNodes[0].y;
	XYZ UpVector;

	if (fabs(dz) > fabs(dHorz))
	{
		if (dz >= 0.0)
		{
			if (bYarn == PATTERN_WEFTYARN)
				UpVector.x = -1;
			else
				UpVector.y = -1;
		}
		else
		{
			if (bYarn == PATTERN_WEFTYARN)
				UpVector.x = 1;
			else
				UpVector.y = 1;

		}
	}
	else
	{
		if (dHorz >= 0.0)
		{
			UpVector.z = 1;
		}
		else
		{
			UpVector.z = -1;
		}
	}
	return UpVector;
	
}

void CTextileBraidCurved::CheckUpVectors(int Index, bool bYarn) const
{
	BuildTextileIfNeeded();
	int YarnIndex = 0;
	if (bYarn == PATTERN_WEFTYARN)
	{
		if (Index > m_iNumWeftYarns - 1)
			return;
		YarnIndex = m_WeftYarns[Index][0];
	}
	else
	{
		if (Index > m_iNumWarpYarns - 1)
			return;
		YarnIndex = m_WarpYarns[Index][0];
	}

	vector<CNode> Nodes = m_Yarns[YarnIndex].GetMasterNodes();
	vector<CNode>::iterator itNodes;

	XYZ CheckNodes[3];
	XYZ UpVector, StartUp;

	int size = Nodes.size();
	for (int i = 0; i < size - 1; ++i)
	{
		CheckNodes[1] = Nodes[i].GetPosition();
		if (i == 0)
		{
			CheckNodes[0] = CheckNodes[1];
			CheckNodes[2] = Nodes[i + 1].GetPosition();
		}
		else
		{
			CheckNodes[2] = Nodes[i + 1].GetPosition();
		}
		UpVector = GetUpVector(CheckNodes, bYarn);
		Nodes[i].SetUp(UpVector);
		if (i == 0)
			Nodes[size - 1].SetUp(UpVector);
		CheckNodes[0] = CheckNodes[1];
	}
	m_Yarns[YarnIndex].SetNodes(Nodes);
}

double CTextileBraidCurved::GetNodeRotation(int YarnIndex, int NodeIndex) const
{
	vector<CNode> Nodes = m_Yarns[YarnIndex].GetMasterNodes();
	CNode Node = Nodes[NodeIndex];
	XYZ NodePos = Node.GetPosition();

	double X,Y,Z;
	X = NodePos.x;
	Y = NodePos.y;
	Z = NodePos.z;

	double R, b, a;
	R = sqrt((X*X) + (Z*Z));
	b = R;
	
	
	XYZ centre;
	centre.x = 0;
	centre.y = NodeIndex * m_WeftYarnData[0].dSpacing*cos(m_dbraidAngle) * 2;
	double r, theta;
	r = sqrt(((X - centre.x)*(X - centre.x)) + ((Y - centre.y)*(Y - centre.y)) + ((Z - centre.z)*(Z - centre.z)));
	theta = ( acos(sqrt((1 - ((b / r)*(b / r))) / ((exp(1))*(exp(1))))));
	a = R * (1 / cos((PI/2)-m_dbraidAngle));
	if (X == 0 && Y == 0)
	{
		return 0.0;
	}
	double slope = (((b*b)*X) / ((a*a)*Z));
	double rotation = atan2(slope,1);

	return rotation;

}