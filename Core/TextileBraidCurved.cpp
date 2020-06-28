#include "PrecompiledHeaders.h"
#include "TextileBraidCurved.h"
#include "SectionEllipse.h"
#include "SectionRotated.h"
#include "SectionPolygon.h"
#include "SectionLenticular.h"
#include "DomainPrism.h"

using namespace TexGen;

CTextileBraidCurved::CTextileBraidCurved(int iNumWeftYarns, int iNumWarpYarns, double dWidth,
	double dHeight, double dThickness,
	double dRadius, double dHornGearVelocity, int iNumHornGear,
	double dVelocity, bool bCurved, bool bRefine, int iNumLayers)
	:CTextileBraid(iNumWeftYarns, iNumWarpYarns, dWidth, dHeight, dThickness, dRadius, dHornGearVelocity,
		iNumHornGear, dVelocity, bRefine, false)
	, m_bCurved(true)
	,iNumLayers(iNumLayers)
{
	m_YarnAngles.resize(m_iNumWarpYarns + m_iNumWeftYarns);
	for (int i = 0; i < m_YarnAngles.size(); i++)
	{
		m_YarnAngles[i].resize(m_iNumWeftYarns + 1);
	}

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



	TGLOGINDENT("Building braid\"" << GetName() << "\"");

	vector<int> Yarns;


	double r, theta, z;
	double x, y;

	// add weft yarns (x yarns)
	int i, j, k, iYarn;
	double startr = m_dMandrel_Rad;
	double startTheta = 0;
	double starty = 0;
	double a;

	PolarCoor.resize(m_iNumYarns);
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
					PolarCoor[i].push_back(RThetaZ(r, theta, y));
					z = r * cos(theta);
					x = r * sin(theta);
					a = (r*2.0) / (2.0*cos((PI / 2) - ((PI / 2) - m_dbraidAngle)));
					//if (x==0 || r==0)
						//m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(sin(m_dbraidAngle), cos(m_dbraidAngle), 0)));
					//m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(sin(m_dbraidAngle), cos(m_dbraidAngle), (-x) / sqrt((r*r) - (x*x)))));
					//m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(sin(m_dbraidAngle), cos(m_dbraidAngle),-1.0*((r*r)*x)/((a*a)*z))));
					m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), GetNodeTangents(i, j)));
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
		//CheckUpVectors(i, true);
		starty += m_WeftYarnData[i].dSpacing*cos(m_dbraidAngle);
		startTheta += -1 * (m_WeftYarnData[i].dSpacing*sin(m_dbraidAngle) / (PI*m_dMandrel_Rad * 2))*(2 * PI);
	}



	// Add the Warp Yarns


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
					PolarCoor[i + m_iNumWeftYarns].push_back(RThetaZ(r, theta, y));
					z = r * cos(theta);
					x = -1 * r * sin(theta);
					//if(x==0 || r==0)
						//m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(-sin(m_dbraidAngle), cos(m_dbraidAngle), 0)));
					//m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), XYZ(-sin(m_dbraidAngle), cos(m_dbraidAngle),((r*r)*x)/((a*a)*z))));
					m_Yarns[Yarns[iYarn]].AddNode(CNode(XYZ(x, y, z), GetNodeTangents(i + m_iNumWeftYarns, j)));

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
		//CheckUpVectors(i, false);
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
		CSectionLenticular Section(dWidth, dHeight);
		CYarnSectionInterpPosition AngledYarnSection(true, true);
		for (int j = 0; j <= m_iNumWeftYarns; j++)
		{
			position = ((double)j / m_iNumWeftYarns);
			double rotation = GetNodeRotation(i, j);
			m_YarnAngles[i][j] = rotation;
			AngledYarnSection.AddSection(position, CSectionRotated(Section, rotation));

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
		CSectionLenticular Section(dWidth, dHeight);
		CYarnSectionInterpPosition AngledYarnSection(true, true);
		for (int j = 0; j <= m_iNumWarpYarns; j++)
		{
			position = (double)j / m_iNumWarpYarns;
			double rotation = GetNodeRotation(i + m_iNumWeftYarns, j);
			m_YarnAngles[i + m_iNumWeftYarns][j] = rotation;
			AngledYarnSection.AddSection(position, CSectionRotated(Section, rotation));
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
		itYarn->AddRepeat(XYZ(0.0,0.0,m_dFabricThickness));
		//itYarn->AddRotationalRepeat(XYZ(dWidthWarp, dHeightWarp,0),0.45);
	}
	/*if (iNumLayers > 1)
		AddLayers(iNumLayers);*/

	if (!m_bRefine)
		return true;

	Refine();
	return true;


}

double CTextileBraidCurved::DomainAngle()
{
	RThetaZ Min, Max;
	Min.theta = 0.0;
	//Min.theta = -(m_WeftYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI)*((m_iNumWeftYarns - 1) / 2);
	//Max.theta = (m_WarpYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI)*((m_iNumWarpYarns - 1) / 2);
	Max.theta = 0.159355;
	double angle = Max.theta - Min.theta;
	if (fmod((2 * PI / angle), 1.0) == 0) return angle;
	else
	{
		double roundAngle = (2 * PI) / round((2 * PI) / angle);
		return roundAngle;
	}

	//return angle; 


}

CDomainPrism CTextileBraidCurved::GetDefaultCurvedDomain(bool bAddedHeight)
{
	vector<XY> points;
	double dGap = 0.0;
	if (bAddedHeight)
		dGap = 0.05*m_dFabricThickness;
	double angle = DomainAngle() / 20;
	double theta;

	theta = 0.0;
	//theta = -(angle * 10);
	//theta = -(m_WeftYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI)*(m_iNumWeftYarns / 2);
	double radius = 1000 * m_dRadius - dGap;
	for (int i = 0; i < 21; i++)
	{
		points.push_back(XY((sin(theta)*radius), (cos(theta)*radius)));
		theta = theta + angle;

	}
	radius = 1000 * m_dRadius + (iNumLayers*m_dFabricThickness) + dGap;
	theta = theta - angle;
	for (int i = 0; i < 21; i++)
	{
		points.push_back(XY((sin(theta)*radius), (cos(theta)*radius)));
		theta = theta - angle;

	}
	double MinY = m_WeftYarnData[0].dSpacing*cos(m_dbraidAngle) * 2 * (m_iNumWeftYarns / 4);
	double MaxY = MinY + m_WeftYarnData[0].dSpacing*cos(m_dbraidAngle) *  (m_iNumWeftYarns / 2);
	return CDomainPrism(points, XYZ(0, MinY, 0), XYZ(0, MaxY, 0));
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

	//CDomainPlanes Domain = CDomainPlanes(XYZ(-20, -5, 0), XYZ(20, 40, 17));
	//AssignDomain(Domain);
	CDomainPrism Domain = GetDefaultCurvedDomain(bAddedHeight);
	Domain.GeneratePlanes();
	AssignDomain(Domain);

	TGLOG("Domain Volume: " << Domain.GetVolume());
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

	XYZ NodePos = Nodes[NodeIndex].GetPosition();

	double X, Y, Z;
	X = NodePos.x;
	Y = NodePos.y;
	Z = NodePos.z;

	Nodes[NodeIndex].ProjectUp();
	XYZ upVector = Nodes[NodeIndex].GetUp();
	Nodes[NodeIndex].SetUp(upVector);

	double a, b; // constants from ellipse

	b = PolarCoor[YarnIndex][NodeIndex].r;
	a = (b*2.0) / (2.0*cos((PI / 2) - ((PI / 2) - m_dbraidAngle)));

	double TanGradient = -((b*b)*X) / ((a*a)*Z);

	double rotation = atan(TanGradient);
	m_Yarns[YarnIndex].SetNodes(Nodes);

	return rotation;
}

void CTextileBraidCurved::AddAdditionalNodes() const
{
	// add nodes at the start of weft
	for (int i = 0; i < 4; i++)
	{
		vector<CNode> Nodes = m_Yarns[i].GetMasterNodes();
		CNode Node = Nodes[0];
		XYZ NodePos = Node.GetPosition();
		double x, y, z;
		x = NodePos.x;
		y = NodePos.y;
		z = NodePos.z;
		double r = m_dMandrel_Rad;
		double theta = asin(x / r);
		for (int j = 0; j < m_iNumWeftYarns; j++)
		{
			theta -= (m_WarpYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI);
			y -= m_WarpYarnData[j].dSpacing*cos(m_dbraidAngle);
			x = r * sin(theta);
			z = r * cos(theta);
			m_Yarns[i].InsertNode(CNode(XYZ(x, y, z), XYZ(sin(m_dbraidAngle), cos(m_dbraidAngle), (-x) / sqrt((r*r) - (x*x)))), 0);
		}

	}
	// add nodes to the end of weft 
	for (int i = 0; i < 4; i++)
	{
		vector<CNode> Nodes = m_Yarns[i].GetMasterNodes();
		CNode Node = Nodes[8];
		XYZ NodePos = Node.GetPosition();
		double x, y, z;
		x = NodePos.x;
		y = NodePos.y;
		z = NodePos.z;
		double r = m_dMandrel_Rad;
		double theta = asin(x / r);
		for (int j = 0; j < m_iNumWeftYarns; j++)
		{
			theta += (m_WarpYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI);
			y += m_WarpYarnData[j].dSpacing*cos(m_dbraidAngle);
			x = r * sin(theta);
			z = r * cos(theta);
			m_Yarns[i].AddNode(CNode(XYZ(x, y, z), XYZ(sin(m_dbraidAngle), cos(m_dbraidAngle), (-x) / sqrt((r*r) - (x*x)))));
		}
	}
	// add nodes to the end of warp
	for (int i = 0; i < 4; i++)
	{
		vector<CNode> Nodes = m_Yarns[i + 4].GetMasterNodes();
		CNode Node = Nodes[0];
		XYZ NodePos = Node.GetPosition();
		double x, y, z;
		x = NodePos.x;
		y = NodePos.y;
		z = NodePos.z;
		double r = m_dMandrel_Rad;
		double theta = asin(x / r);
		for (int j = 0; j < m_iNumWeftYarns; j++)
		{
			theta += (m_WeftYarnData[j].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI);
			y -= m_WeftYarnData[j].dSpacing*cos(m_dbraidAngle);
			x = r * sin(theta);
			z = r * cos(theta);
			m_Yarns[i + 4].InsertNode(CNode(XYZ(x, y, z), XYZ(-1 * sin(m_dbraidAngle), cos(m_dbraidAngle), -(-x) / sqrt((r*r) - (x*x)))), 0);
		}

	}
	for (int i = 0; i < 4; i++)
	{
		vector<CNode> Nodes = m_Yarns[i + 4].GetMasterNodes();
		CNode Node = Nodes[8];
		XYZ NodePos = Node.GetPosition();
		double x, y, z;
		x = NodePos.x;
		y = NodePos.y;
		z = NodePos.z;
		double r = m_dMandrel_Rad;
		double theta = asin(x / r);
		for (int j = 0; j < m_iNumWeftYarns; j++)
		{
			theta -= (m_WeftYarnData[j].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI);
			y += m_WeftYarnData[j].dSpacing*cos(m_dbraidAngle);
			x = r * sin(theta);
			z = r * cos(theta);
			m_Yarns[i + 4].AddNode(CNode(XYZ(x, y, z), XYZ(-1 * sin(m_dbraidAngle), cos(m_dbraidAngle), -(-x) / sqrt((r*r) - (x*x)))));
		}

	}

}

void CTextileBraidCurved::Refine(bool bCorrectWidths, bool bPeriodic) const
{
	//string FileName = "Cross_Section_4_4";
	//SaveCrossSection(FileName, 4, 4);
	//CorrectBraidYarnWidths();
	//CorrectInterference();
	//FileName = "Cross_Section_4_4_Correct_Interference";
	//SaveCrossSection(FileName, 4, 4);
	AdjustSectionsForRotation(bPeriodic);
	//FileName = "Cross_Section_4_4_Rotation";
	//SaveCrossSection(FileName, 4, 4);
	CorrectInterference();
	//FileName = "Cross_Section_4_4_refine";
	//(FileName, 4, 4);
}

void CTextileBraidCurved::CorrectBraidYarnWidths() const
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

	//RepeatLimits.resize(2, pair<int, int>(-1, 0));
	RepeatLimits.resize(0);
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
	double position;
	for (i = 0; i < m_iNumWeftYarns; ++i)
	{
		dWidth = m_WeftYarnData[i].dWidth;
		dHeight = m_WeftYarnData[i].dHeight;
		CSectionLenticular Section(dWidth, dHeight);
		CYarnSectionInterpPosition AngledYarnSection(true, true);
		for (int j = 0; j <= m_iNumWeftYarns; j++)
		{
			position = ((double)j / m_iNumWeftYarns);
			double rotation = GetNodeRotation(i, j);
			AngledYarnSection.AddSection(position, CSectionRotated(Section, rotation));

		}
		if (m_pSectionMesh)
			Section.AssignSectionMesh(*m_pSectionMesh);
		for (itpYarn = m_WeftYarns[i].begin(); itpYarn != m_WeftYarns[i].end(); ++itpYarn)
		{
			m_Yarns[*itpYarn].AssignSection((AngledYarnSection));

		}

	}
	/*for (int i = 0; i < m_iNumWeftYarns; i++)
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
	}*/
	for (i = 0; i < m_iNumWarpYarns; ++i)
	{
		dWidth = m_WarpYarnData[i].dWidth;
		dHeight = m_WarpYarnData[i].dHeight;
		CSectionLenticular Section(dWidth, dHeight);
		CYarnSectionInterpPosition AngledYarnSection(true, true);
		for (int j = 0; j <= m_iNumWarpYarns; j++)
		{
			position = (double)j / m_iNumWarpYarns;
			double rotation = GetNodeRotation(i + m_iNumWeftYarns, j);
			AngledYarnSection.AddSection(position, CSectionRotated(Section, rotation));
		}
		if (m_pSectionMesh)
			Section.AssignSectionMesh(*m_pSectionMesh);
		for (itpYarn = m_WarpYarns[i].begin(); itpYarn != m_WarpYarns[i].end(); ++itpYarn)
		{
			m_Yarns[*itpYarn].AssignSection(AngledYarnSection);
		}
	}
	/*for (int i = 0; i < m_iNumWarpYarns; i++)
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
	}*/
}

bool CTextileBraidCurved::AdjustSectionsForRotation(bool bPeriodic) const
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
	double dRotation;
	double k;
	int iNumYarns, iYarnLength;
	int iDirection;
	int x, y;

	// First loop for Y yarns, second loop for X yarns
	for (iDirection = 0; iDirection < 2; ++iDirection)
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
			for (j = 0; j < iYarnLength; ++j)
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

				CSectionLenticular* LenticularSection = NULL;
				string YarnSection = pYarnSection->GetNodeSection(j).GetType();
				if (pYarnSection->GetNodeSection(j).GetType() == "CSectionLenticular")
					LenticularSection = (CSectionLenticular*)pYarnSection->GetNodeSection(j).Copy();
				else
					LenticularSection = (CSectionLenticular*)DefaultEllipseSection.Copy();
				if (iDirection == 0)
				{
					dRotation = GetNodeRotation(i + iNumYarns, j);
					k = i + iNumYarns;
				}
				else
				{
					dRotation = GetNodeRotation(i, j);
					k = i;
				}
				double dRot;
				// Assign section based on the rotation it should have
				switch (iRot)
				{
				case 0:
					pYarnSection->ReplaceSection(j, CSectionRotated(*LenticularSection, dRotation));
					m_YarnAngles[k][j] = dRotation;
					break;
				case -1:
					dRot = dRotation - dAngle;
					pYarnSection->ReplaceSection(j, CSectionRotated(*LenticularSection, dRot));
					m_YarnAngles[k][j] = dRot;
					break;
				case 1:
					dRot = dRotation + dAngle;
					pYarnSection->ReplaceSection(j, CSectionRotated(*LenticularSection, dRot));
					m_YarnAngles[k][j] = dRot;
					break;
				}
				delete LenticularSection;
			}

			// Assign the same section to the end as at the start (periodic yarns)
			if (bPeriodic)
				j = 0;
			if (iDirection == 0)
			{
				y = iPrevYarny = iNextYarny = j;
			}
			else
			{
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
			string YarnSection = pYarnSection->GetNodeSection(j).GetType();
			if (pYarnSection->GetNodeSection(iYarnLength).GetType() == "CSectionLenticular")
				EllipseSection = (CSectionLenticular*)pYarnSection->GetNodeSection(j).Copy();
			else
				EllipseSection = (CSectionLenticular*)DefaultEllipseSection.Copy();
			if (iDirection == 0)
			{
				dRotation = GetNodeRotation(i + iNumYarns, iYarnLength);
				k = i + iNumYarns;
			}
			else
			{
				dRotation = GetNodeRotation(i, iYarnLength);
				k = i;
			}
			double dRot;
			// Assign section based on the rotation it should have
			switch (iRot)
			{

			case 0:
				pYarnSection->ReplaceSection(iYarnLength, CSectionRotated(*EllipseSection, dRotation));
				m_YarnAngles[k][j] = dRotation;
				break;
			case -1:
				dRot = dRotation - dAngle;
				pYarnSection->ReplaceSection(iYarnLength, CSectionRotated(*EllipseSection, dRot));
				m_YarnAngles[k][j] = dRot;
				break;
			case 1:
				dRot = dRotation + dAngle;
				pYarnSection->ReplaceSection(iYarnLength, CSectionRotated(*EllipseSection, dRot));
				m_YarnAngles[k][j] = dRot;
				break;
			}

			pYarn->AssignSection(*pYarnSection);
			delete pYarnSection;
		}
	}

	return true;
}

void CTextileBraidCurved::CorrectInterference() const
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
	//RepeatLimits.resize(2, pair<int, int>(-1, 0));
	RepeatLimits.resize(0);
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
			/*if (i == 4 && j == 4)
			{
				ofstream fileX;
				fileX.open("yarn_cross_section_x.txt");
				for (int q = 0; q < Points.size(); q++)
				{
					fileX << Points[q][0];
					fileX << endl;
				}
				fileX.close();
				ofstream fileY;
				fileY.open("yarn_cross_section_y.txt");
				for (int q = 0; q < Points.size(); q++)
				{
					fileY << Points[q][1];
					fileY << endl;
				}
				fileY.close();
			}*/
			for (k = 0; k < (int)Points.size(); k++)
			{
				Points[k] *= YarnSectionModifiers[i][j][k];
			}
			/*if (i == 4 && j == 4)
			{
				ofstream file1X;
				file1X.open("yarn_cross_section_refine_x.txt");
				for (int q = 0; q < Points.size(); q++)
				{
					file1X << Points[q][0];
					file1X << endl;
				}
				file1X.close();
				ofstream file1Y;
				file1Y.open("yarn_cross_section_refine_y.txt");
				for (int q = 0; q < Points.size(); q++)
				{
					file1Y << Points[q][1];
					file1Y << endl;
				}
				file1Y.close();
			}*/
			CSectionPolygon Section(Points);
			if (m_pSectionMesh)
				Section.AssignSectionMesh(*m_pSectionMesh);
			NewYarnSection.AddSection(Section);
		}
		double RotationAngle = -m_YarnAngles[i][0] + m_YarnAngles[i][m_YarnAngles[i].size() - 1];
		NewYarnSection.AddSection(CSectionRotated(NewYarnSection.GetNodeSection(0), RotationAngle));

		// Add Sections between the nodes (Not necessary for sections that dont cross)
		YarnPosInfo.dSectionPosition = 0.5;
		for (j = 0; j < (int)YarnSectionModifiers[i].size(); ++j)
		{
			if (NeedsMidSection(i, j))
			{
				YarnPosInfo.iSection = j;

				Points = pYarnSection->GetSection(YarnPosInfo, YarnSectionModifiers[i][j].size());
				for (k = 0; k < (int)Points.size(); ++k)
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

XYZ CTextileBraidCurved::GetNodeTangents(int YarnIndex, int NodeIndex) const
{
	double r0, theta0, y0;
	double r1, theta1, y1;
	double x0, z0;
	double x1, z1;
	if (YarnIndex < m_iNumWeftYarns)
	{

		r0 = PolarCoor[YarnIndex][NodeIndex].r;
		theta0 = PolarCoor[YarnIndex][NodeIndex].theta;
		y0 = PolarCoor[YarnIndex][NodeIndex].z;

		r1 = r0;
		theta1 = theta0 + ((m_WarpYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI));
		y1 = y0 + m_WeftYarnData[0].dSpacing*cos(m_dbraidAngle);

		x0 = r0 * sin(theta0);
		z0 = r0 * cos(theta0);

		x1 = r1 * sin(theta1);
		z1 = r1 * cos(theta1);

		XYZ dirVector = XYZ(x1 - x0, y1 - y0, z1 - z0);
		double Magnitude = sqrt((dirVector.x*dirVector.x) + (dirVector.y*dirVector.y) + (dirVector.z*dirVector.z));

		XYZ Tangent = XYZ((dirVector.x / Magnitude), (dirVector.y / Magnitude), (dirVector.z / Magnitude));

		return Tangent;

	}

	if (YarnIndex >= m_iNumWeftYarns)
	{
		r0 = PolarCoor[YarnIndex][NodeIndex].r;
		theta0 = PolarCoor[YarnIndex][NodeIndex].theta;
		y0 = PolarCoor[YarnIndex][NodeIndex].z;

		r1 = r0;
		theta1 = theta0 + ((m_WeftYarnData[0].dSpacing*sin(m_dbraidAngle) / (PI*(m_dMandrel_Rad * 2)))*(2 * PI));
		y1 = y0 + m_WarpYarnData[0].dSpacing*cos(m_dbraidAngle);

		x0 = -1.0*r0 * sin(theta0);
		z0 = r0 * cos(theta0);

		x1 = -1.0* r1 * sin(theta1);
		z1 = r1 * cos(theta1);

		XYZ dirVector = XYZ(x1 - x0, y1 - y0, z1 - z0);
		double Magnitude = sqrt((dirVector.x*dirVector.x) + (dirVector.y*dirVector.y) + (dirVector.z*dirVector.z));

		XYZ Tangent = XYZ((dirVector.x / Magnitude), (dirVector.y / Magnitude), (dirVector.z / Magnitude));

		return Tangent;
	}




}

void CTextileBraidCurved::SaveCrossSection(string& FileName, int YarnIndex, int NodeIndex) const
{
	const CYarnSection* pYarnSection;
	YARN_POSITION_INFORMATION YarnPosInfo;
	CYarnSectionInterpNode NewYarnSection(false, true);
	vector<XY> Points;


	pYarnSection = m_Yarns[YarnIndex].GetYarnSection();
	YarnPosInfo.SectionLengths = m_Yarns[YarnIndex].GetYarnSectionLengths();
	Points = pYarnSection->GetSection(YarnPosInfo, 40);
	ofstream file1X;
	file1X.open(FileName + "_x.txt");
	for (int q = 0; q < Points.size(); q++)
	{
		file1X << Points[q][0];
		file1X << endl;
	}
	file1X.close();
	ofstream file1Y;
	file1Y.open(FileName + "_y.txt");
	for (int q = 0; q < Points.size(); q++)
	{
		file1Y << Points[q][1];
		file1Y << endl;
	}
	file1Y.close();
}

double CTextileBraidCurved::ReturnNodeRotation(int YarnIndex, int NodeIndex) const
{
	return m_YarnAngles[YarnIndex][NodeIndex];
}

double CTextileBraidCurved::ReturnNodeTheta(int YarnIndex, int NodeIndex) const
{
	TGLOG("Theta: " << PolarCoor[YarnIndex][NodeIndex].theta);
	return PolarCoor[YarnIndex][NodeIndex].theta;
}