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


	return true;
}

void CArcVoxelMesh::OutputNodes(ostream &Output, CTextile &Textile, int Filetype)
{

}