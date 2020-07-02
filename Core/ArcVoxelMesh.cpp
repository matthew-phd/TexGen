#include "PrecompiledHeaders.h"
#include "ArcVoxelMesh.h"
#include "TexGen.h"
#include "ShearedPeriodicBoundaries.h"
#include <iterator>

using namespace TexGen;

TexGen::CArcVoxelMesh::CArcVoxelMesh(string Type)
	:CVoxelMesh(Type)
{
}

TexGen::CArcVoxelMesh::~CArcVoxelMesh(void)
{
}

bool CArcVoxelMesh::CalculateVoxelSizes(CTextile &Textile)
{
	vector<XY> Points = Textile.GetDomain()->GetPrismDomain()->GetPoints();
	m_StartPoint.x = Points[0].x;
	m_StartPoint.z = Points[0].y;
	XYZ Node2, Node4;
	Node2.x = Points[20].x;
	Node2.z = Points[20].y;
	Node4.x = Points[41].x;
	Node4.z = Points[41].y;
	double TotalTheta, R, l, theta, z;
	l = sqrt(((Node2.x - m_StartPoint.x)*(Node2.x - m_StartPoint.x)) + ((Node2.z - m_StartPoint.z)*(Node2.z - m_StartPoint.z)));
	R = m_StartPoint.z;

	// Calculate total theta of domain
	TotalTheta = 2 * asin(l / (2*R));
	// Calculate change in theta for one voxel;
	theta = TotalTheta / m_XVoxels;
	m_ArcPolarSize.theta = theta;
	//Calculate total change in r
	z = Node4.z - m_StartPoint.z;
	// Change in z per voxel
	m_ArcPolarSize.r = z / m_ZVoxels;

	m_StartPoint.y = Textile.GetDomain()->GetPrismDomain()->GetStart().y;
	double End = Textile.GetDomain()->GetPrismDomain()->GetEnd().y;

	//Calculate total distance in y
	double y = End - m_StartPoint.y;
	// Calculate change in y per voxel
	m_ArcPolarSize.z = y / m_YVoxels;

	m_dCosAngle = cos(TotalTheta);
	m_dSinAngle = sin(TotalTheta);

	return true;
}

void CArcVoxelMesh::OutputNodes(ostream &Output, CTextile &Textile, int Filetype)
{
	int x, y, z;
	int iNodeIndex = 1;
	vector<XYZ> CentrePoints;
	vector<POINT_INFO> RowInfo;
	XYZ StartPoint = m_StartPoint;
	double dRadius;

	for (int z = 0; z <= m_ZVoxels; z++)
	{
		dRadius = m_StartPoint.z + m_ArcPolarSize.r*z;
		StartPoint.x = m_StartPoint.x;
		StartPoint.z = dRadius;
		for (y = 0; y <= m_YVoxels; y++)
		{
			StartPoint.y = m_StartPoint.y + m_ArcPolarSize.z*y;

			for (x = 0; x <= m_XVoxels; x++)
			{
				XYZ Point;
				Point.x = dRadius * sin(0 + m_ArcPolarSize.theta*x);
				Point.y = StartPoint.y;
				Point.z = dRadius * cos(0 + m_ArcPolarSize.theta*x);

				Output << iNodeIndex << ", ";
				Output << Point << "\n";

				if (x < m_XVoxels && y < m_YVoxels && z < m_ZVoxels)
				{
					Point.x += (0.5*m_ArcPolarSize.r) *sin(0.5*m_ArcPolarSize.theta);
					Point.y += 0.5*m_ArcPolarSize.z;
					Point.z += (0.5*m_ArcPolarSize.r) *sin(0.5*m_ArcPolarSize.theta);
					CentrePoints.push_back(Point);
				}

				iNodeIndex++;
			}
		}
		RowInfo.clear();   // Changed to do layer at a time instead of row to optimise
		Textile.GetPointInformation(CentrePoints, RowInfo);
		m_ElementsInfo.insert(m_ElementsInfo.end(), RowInfo.begin(), RowInfo.end());
		CentrePoints.clear();
	}

	Output << "*PARAMETER" << endl;
	Output << "SS=" << m_dSinAngle << "; CC=" << m_dCosAngle << endl;
}