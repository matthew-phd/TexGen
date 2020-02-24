/*=============================================================================
TexGen: Geometric textile modeller.
Copyright (C) 2006 Martin Sherburn

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
=============================================================================*/

#include "PrecompiledHeaders.h"
#include "DomainPrism.h"
#include "Yarn.h"
#include "SectionPolygon.h"
using namespace TexGen;

CDomainPrism::CDomainPrism(const vector<XY> &Points, XYZ &start, XYZ &end)
{
	CNode Node = CNode(start);
	m_Yarn.AddNode(Node);
	Node = CNode(end);
	m_Yarn.AddNode(Node);
	m_Yarn.AssignSection(CYarnSectionConstant(CSectionPolygon(Points,false, true)));
	m_Yarn.SetResolution(2, Points.size());  // Need better definition of resolution
	BuildMesh();
}

CDomainPrism::CDomainPrism()
{

}

CDomainPrism::~CDomainPrism(void)
{
}

CDomainPrism::CDomainPrism(TiXmlElement &Element)
	: CDomain(Element)
{
	TiXmlElement* pYarn = Element.FirstChildElement("Yarn");
	m_Yarn = CYarn(*pYarn);

	if (m_Mesh.GetNumNodes() == 0)
	{
		BuildMesh();
	}
}

void CDomainPrism::PopulateTiXmlElement(TiXmlElement &Element, OUTPUT_TYPE OutputType) const
{
	CDomain::PopulateTiXmlElement(Element, OutputType);
	TiXmlElement Yarn("Yarn");
	m_Yarn.PopulateTiXmlElement(Yarn, OutputType);
	Element.InsertEndChild(Yarn);
}

void CDomainPrism::BuildMesh()
{
	m_Mesh.Clear();
	m_Yarn.AddSurfaceToMesh( m_Mesh );
	m_Mesh.MergeNodes();
	m_Mesh.SaveToVTK("DomainMesh");
}

void CDomainPrism::GeneratePlanes()
{
	if (m_Mesh.GetNumNodes() == 0)
	{
		BuildMesh();
	}

	list<int>::iterator itInt;
	list<int>::iterator itStart;
	XYZ points[4];
	m_ElementPlanes.clear();

	list<int> &QuadIndices = m_Mesh.GetIndices(CMesh::QUAD);
	for (itInt = QuadIndices.begin(); itInt != QuadIndices.end(); )
	{
		itStart = itInt;

		points[0] = m_Mesh.GetNode(*(itInt++));
		points[1] = m_Mesh.GetNode(*(itInt++));
		points[2] = m_Mesh.GetNode(*(itInt++));
		points[3] = m_Mesh.GetNode(*(itInt++));

		PLANE ElementPlane;
		if (GetPlane( points, ElementPlane))
			m_ElementPlanes.push_back(ElementPlane);
	}

	list<int> &TriIndices = m_Mesh.GetIndices(CMesh::TRI);
	for (itInt = TriIndices.begin(); itInt != TriIndices.end(); )
	{
		itStart = itInt;

		points[0] = m_Mesh.GetNode(*(itInt++));
		points[1] = m_Mesh.GetNode(*(itInt++));
		points[2] = m_Mesh.GetNode(*(itInt++));

		PLANE ElementPlane;
		if (GetPlane(points, ElementPlane))
			m_ElementPlanes.push_back(ElementPlane);
	}

	list<int> &Indices = m_Mesh.GetIndices(CMesh::POLYGON);
	list<int>::iterator itIndices;
	int StartIndex;
	list<int>::iterator itStartIndex;

	for (itIndices = Indices.begin(); itIndices != Indices.end(); )
	{
		StartIndex = (*(itIndices));
		itStartIndex = itIndices;
		vector<int> PolygonIndex;
		XYZ p;

		int i = 0, iPoints = 0;
		
		do {
			if (i < 3)
			{
				points[i] = m_Mesh.GetNode(*(itIndices));
			}
			++i;
			++itIndices;
			if (i == 3)
			{
				PLANE ElementPlane;
				if (GetPlane(points, ElementPlane))
					m_ElementPlanes.push_back(ElementPlane);
			}
		} while ((*itIndices) != StartIndex);
		++itIndices;
	}
	RemoveDuplicatePlanes();
}

bool CDomainPrism::GetPlane(XYZ *p, PLANE &plane)
{
	plane.Normal = CrossProduct(p[2] - p[0], p[1] - p[0]);

	if (GetLength(plane.Normal))
	{
		Normalise(plane.Normal);
		plane.d = DotProduct(plane.Normal, p[0] - XYZ(0, 0, 0));
		return(true);
	}
	return(false);
}

void CDomainPrism::RemoveDuplicatePlanes()
{
	vector<PLANE>::iterator itPlanes1, itPlanes2;

	for (itPlanes1 = m_ElementPlanes.begin(); itPlanes1 != m_ElementPlanes.end(); ++itPlanes1)
	{
		for (itPlanes2 = itPlanes1 + 1; itPlanes2 != m_ElementPlanes.end(); )
		{
			if (PlaneEqual(*itPlanes1, *itPlanes2))
			{
				itPlanes2 = m_ElementPlanes.erase(itPlanes2);
			}
			else
				++itPlanes2;
		}
	}
}

bool CDomainPrism::PlaneEqual(PLANE Plane1, PLANE Plane2)
{
	if (fabs(GetLength(Plane1.Normal, Plane2.Normal)) > 1e-9)
		return false;
	if (Plane1.d != Plane2.d)
		return false;
	return true;
}

void CDomainPrism::ClipIntersectMeshToDomain(CMesh &Mesh, bool bFillGaps) const
{
	const double TOL = 1e-9;

	TGLOGINDENT("Clipping mesh to domain");

	vector<XYZ> NewTriangles;
	vector<bool> NewTrianglesFlipped;
	vector<XYZ>::iterator itXYZ;
	vector<XYZ> NewQuads;

	vector<PLANE>::const_iterator itPlane;
	list<int>::iterator itStart;
	list<int>::iterator itInt;
	const XYZ *p1, *p2, *p3, *p4;
	double d1, d2, d3, d4;	// d represents the distance of the point to the plane (i.e. +ve inside, -ve outside, 0 on top)
	double dTemp;
	const XYZ *pTemp;
	double u;
	int i;
	int iLastNodeIndex;
	bool bFlipped;

	// Deal with surface elements
	for (itPlane = m_ElementPlanes.begin(); itPlane != m_ElementPlanes.end(); ++itPlane)
	{
		list<int> &QuadIndices = Mesh.GetIndices(CMesh::QUAD);
		for (itInt = QuadIndices.begin(); itInt != QuadIndices.end(); )
		{
			itStart = itInt;

			p1 = &Mesh.GetNode(*(itInt++));
			p2 = &Mesh.GetNode(*(itInt++));
			p3 = &Mesh.GetNode(*(itInt++));
			p4 = &Mesh.GetNode(*(itInt++));

			d1 = DotProduct(itPlane->Normal, *p1) - itPlane->d;
			d2 = DotProduct(itPlane->Normal, *p2) - itPlane->d;
			d3 = DotProduct(itPlane->Normal, *p3) - itPlane->d;
			d4 = DotProduct(itPlane->Normal, *p4) - itPlane->d;

			if (d1 <= TOL && d2 <= TOL && d3 <= TOL && d4 <= TOL) // The quad lies completely on or outside the plane
			{
				//itInt = QuadIndices.erase(itStart, itInt); // Delete the quad
			}
			else if (d1 >= -TOL && d2 >= -TOL && d3 >= -TOL && d4 >= -TOL) // The quad lies completely inside the plane
			{
				// Do nothing
			}
			//else if (d1 < TOL && d2 < TOL && d3 > TOL && d4 > TOL) // Points 1 & 2 outside plane
			else if (d1 < TOL && d2 < TOL && d3 > TOL && d4 > TOL) // Points 1 & 2 outside plane
			{
				itInt = QuadIndices.erase(itStart, itInt); // Delete the quad
				u = d4 / (d4 - d1);
				NewQuads.push_back(*p4 + (*p1 - *p4) * u);
				u = d3 / (d3 - d2);
				NewQuads.push_back(*p3 + (*p2 - *p3) * u);
				NewQuads.push_back(*p3);
				NewQuads.push_back(*p4);
			}
			else if (d2 < TOL && d3 < TOL && d4 > TOL && d1 > TOL) // Points 2 & 3 outside plane
			{
				itInt = QuadIndices.erase(itStart, itInt); // Delete the quad
				NewQuads.push_back(*p1);
				u = d1 / (d1 - d2);
				NewQuads.push_back(*p1 + (*p2 - *p1) * u);
				u = d4 / (d4 - d3);
				NewQuads.push_back(*p4 + (*p3 - *p4) * u);
				NewQuads.push_back(*p4);
			}
			else if (d3 < TOL && d4 < TOL && d1 > TOL && d2 > TOL)  // Points 3 & 4 outside plane
			{
				itInt = QuadIndices.erase(itStart, itInt); // Delete the quad
				NewQuads.push_back(*p1);
				NewQuads.push_back(*p2);
				u = d2 / (d2 - d3);
				NewQuads.push_back(*p2 + (*p3 - *p2) * u);
				u = d1 / (d1 - d4);
				NewQuads.push_back(*p1 + (*p4 - *p1) * u);
			}
			else if (d4 < TOL && d1 < TOL && d2 > TOL && d3 > TOL)  // Points 4 & 1 outside plane
			{
				itInt = QuadIndices.erase(itStart, itInt); // Delete the quad
				u = d2 / (d2 - d1);
				NewQuads.push_back(*p2 + (*p1 - *p2) * u);
				NewQuads.push_back(*p2);
				NewQuads.push_back(*p3);
				u = d3 / (d3 - d4);
				NewQuads.push_back(*p3 + (*p4 - *p3) * u);

			}
			else // Convert the quad to a triangle for trimming if 1 or 3 points inside plane
			{
				itInt = Mesh.ConvertQuadtoTriangles(itStart);
			}
		}

		// Add the new quads to the mesh, and clear the new quad list
		iLastNodeIndex = int(Mesh.GetNumNodes());
		for (i = 0; i < int(NewQuads.size() / 4); ++i)
		{

			QuadIndices.push_back(4 * i + iLastNodeIndex);
			QuadIndices.push_back(4 * i + 1 + iLastNodeIndex);
			QuadIndices.push_back(4 * i + 2 + iLastNodeIndex);
			QuadIndices.push_back(4 * i + 3 + iLastNodeIndex);
		}
		for (itXYZ = NewQuads.begin(); itXYZ != NewQuads.end(); ++itXYZ)
		{
			Mesh.AddNode(*itXYZ);
		}
		NewQuads.clear();

		list<int> &TriIndices = Mesh.GetIndices(CMesh::TRI);
		for (itInt = TriIndices.begin(); itInt != TriIndices.end(); )
		{
			itStart = itInt;

			p1 = &Mesh.GetNode(*(itInt++));
			p2 = &Mesh.GetNode(*(itInt++));
			p3 = &Mesh.GetNode(*(itInt++));

			d1 = DotProduct(itPlane->Normal, *p1) - itPlane->d;
			d2 = DotProduct(itPlane->Normal, *p2) - itPlane->d;
			d3 = DotProduct(itPlane->Normal, *p3) - itPlane->d;

			if (d1 <= TOL && d2 <= TOL && d3 <= TOL) // The triangle lies completely outside or on the plane
			{
				itInt = TriIndices.erase(itStart, itInt); // Delete the triangle
			}
			else if (d1 >= -TOL && d2 >= -TOL && d3 >= -TOL) // The triangle lies completely inside the plane
			{
				// Do nothing
			}
			else
			{
				itInt = TriIndices.erase(itStart, itInt); // Delete the triangle, will need to be seperated into smaller ones
														  // Order points such that d1 >= d2 >= d3
				bFlipped = false; // Keep track of whether the triangle is flipped or not
				if (d2 > d1)
				{
					dTemp = d2; d2 = d1; d1 = dTemp;
					pTemp = p2; p2 = p1; p1 = pTemp;
					bFlipped = !bFlipped;
				}
				if (d3 > d2)
				{
					dTemp = d3; d3 = d2; d2 = dTemp;
					pTemp = p3; p3 = p2; p2 = pTemp;
					bFlipped = !bFlipped;
					if (d2 > d1)
					{
						dTemp = d2; d2 = d1; d1 = dTemp;
						pTemp = p2; p2 = p1; p1 = pTemp;
						bFlipped = !bFlipped;
					}
				}

				if (d2 <= TOL) // One point of the triangle lies inside the plane, the other two are outside or on the plane
				{
					NewTriangles.push_back(*p1);
					u = d1 / (d1 - d2);
					NewTriangles.push_back(*p1 + (*p2 - *p1) * u);
					u = d1 / (d1 - d3);
					NewTriangles.push_back(*p1 + (*p3 - *p1) * u);
					NewTrianglesFlipped.push_back(bFlipped);
				}
				else if (d3 <= TOL) // Two points of the triangle lie inside the plane, the other is outside or on the plane
				{
					NewTriangles.push_back(*p1);
					NewTriangles.push_back(*p2);
					u = d2 / (d2 - d3);
					NewTriangles.push_back(*p2 + (*p3 - *p2) * u);
					NewTrianglesFlipped.push_back(bFlipped);

					NewTriangles.push_back(*p2 + (*p3 - *p2) * u);
					u = d1 / (d1 - d3);
					NewTriangles.push_back(*p1 + (*p3 - *p1) * u);
					NewTriangles.push_back(*p1);
					NewTrianglesFlipped.push_back(bFlipped);
				}
			}
		}

		// Add the new triangles to the mesh, and clear the new triangles list
		iLastNodeIndex = int(Mesh.GetNumNodes());
		for (i = 0; i < int(NewTriangles.size() / 3); ++i)
		{
			// The order of the vertices determines the direction of the normal,
			// the normal should always point outwards from the yarn. Thus if the
			// normal was flipped we must make sure the indices are swapped so
			// the normal is flipped back to its original state.
			if (!NewTrianglesFlipped[i])
			{
				TriIndices.push_back(3 * i + iLastNodeIndex);
				TriIndices.push_back(3 * i + 1 + iLastNodeIndex);
				TriIndices.push_back(3 * i + 2 + iLastNodeIndex);
			}
			else
			{
				TriIndices.push_back(3 * i + iLastNodeIndex);
				TriIndices.push_back(3 * i + 2 + iLastNodeIndex);
				TriIndices.push_back(3 * i + 1 + iLastNodeIndex);
			}
		}
		for (itXYZ = NewTriangles.begin(); itXYZ != NewTriangles.end(); ++itXYZ)
		{
			Mesh.AddNode(*itXYZ);
		}
		NewTriangles.clear();
		NewTrianglesFlipped.clear();

		vector<int> ClosedLoop;
		if (bFillGaps)
			FillGaps(Mesh, *itPlane, ClosedLoop);
	}

	// Deal with volume elements
	int iNumNodes;
	double d;
	vector<CMesh::ELEMENT_TYPE> VolumeElements;
	vector<CMesh::ELEMENT_TYPE>::iterator itElementType;
	VolumeElements.push_back(CMesh::TET);
	VolumeElements.push_back(CMesh::PYRAMID);
	VolumeElements.push_back(CMesh::WEDGE);
	VolumeElements.push_back(CMesh::HEX);
	VolumeElements.push_back(CMesh::QUADRATIC_TET);
	for (itPlane = m_ElementPlanes.begin(); itPlane != m_ElementPlanes.end(); ++itPlane)
	{
		for (itElementType = VolumeElements.begin(); itElementType != VolumeElements.end(); ++itElementType)
		{
			list<int> &Indices = Mesh.GetIndices(*itElementType);
			for (itInt = Indices.begin(); itInt != Indices.end(); )
			{
				iNumNodes = CMesh::GetNumNodes(*itElementType);
				itStart = itInt;

				XYZ Center;
				for (i = 0; i<iNumNodes; ++i)
				{
					Center += Mesh.GetNode(*(itInt++));
				}
				Center /= iNumNodes;

				d = DotProduct(itPlane->Normal, Center) - itPlane->d;
				if (d < 0)
					itInt = Indices.erase(itStart, itInt); // Delete the volume element
			}
		}
	}

	for (itPlane = m_ElementPlanes.begin(); itPlane != m_ElementPlanes.end(); ++itPlane)
	{
		list<int> &Indices = Mesh.GetIndices(CMesh::POLYGON);
		list<int>::iterator itIndices;
		int StartIndex, NewStartIndex;
		bool bResetStart = true;
		list<int>::iterator itStartIndex;
		bool bDelete = true;

		for (itIndices = Indices.begin(); itIndices != Indices.end(); )
		{
			bDelete = true;
			StartIndex = NewStartIndex = (*(itIndices));
			itStartIndex = itIndices;
			vector<int> PolygonIndex;
			XYZ p;
			bResetStart = true;
			int i = 0, iPoints = 0;
			do {
				p = Mesh.GetNode(*(itIndices));
				d = DotProduct(itPlane->Normal, p) - itPlane->d;
				if (d < TOL)
				{
					//if ( *itIndices == NewStartIndex )
					//bResetStart = false;  // Keeping 1st so don't need to reset
					//bDelete = false;
					++iPoints;
				}
				++i;
				++itIndices;
			} while ((*itIndices) != StartIndex);
			++itIndices;
			//if ( bDelete )
			if (iPoints == i) // All points outside plane
			{
				itIndices = Indices.erase(itStartIndex, itIndices);
			}
			/*if ( NewStartIndex != StartIndex ) // Must have deleted start so reset to new start value
			*(itIndices) = NewStartIndex;
			if ( NewStartIndex == StartIndex && bResetStart )  // Deleted whole loop so need to delete repeated start index
			itIndices = Indices.erase(itIndices);
			else
			itIndices++;*/
		}
	}

	Mesh.RemoveUnreferencedNodes();
}

void CDomainPrism::ClipMeshToDomain(CMesh &Mesh, bool bFillGaps) const
{
	const double TOL = 1e-9;

	TGLOGINDENT("Clipping mesh to domain");

	vector<XYZ> NewTriangles;
	
	vector<XYZ>::iterator itXYZ;
	vector<XYZ> NewQuads;

	vector<PLANE>::const_iterator itPlane;
	list<int>::iterator itStart;
	list<int>::iterator itInt;
	const XYZ *p1, *p2, *p3, *p4;
	//XYZ points[4];
	//double d1, d2, d3, d4;	// d represents the distance of the point to the plane (i.e. +ve inside, -ve outside, 0 on top)
	//double dist[4];
	double dTemp;
	const XYZ *pTemp;
	double u;
	int i;
	int iLastNodeIndex;
	bool bFlipped;
	bool b1, b2, b3, b4;

	CMesh IntersectMesh;
	vector<XYZ> IntersectQuads;

	// Deal with surface elements

	list<int> &QuadIndices = Mesh.GetIndices(CMesh::QUAD);
	for (itInt = QuadIndices.begin(); itInt != QuadIndices.end(); )
	{
		itStart = itInt;

		double d1= 0, d2=0, d3=0, d4 = 0;
		p1 = &Mesh.GetNode(*(itInt++));
		p2 = &Mesh.GetNode(*(itInt++));
		p3 = &Mesh.GetNode(*(itInt++));
		p4 = &Mesh.GetNode(*(itInt++));

		b1 = m_Yarn.PointInsideYarn(*p1, NULL, NULL, NULL, &d1);
		b2 = m_Yarn.PointInsideYarn(*p2, NULL, NULL, NULL, &d2);
		b3 = m_Yarn.PointInsideYarn(*p3, NULL, NULL, NULL, &d3);
		b4 = m_Yarn.PointInsideYarn(*p4, NULL, NULL, NULL, &d4);


		if (!b1 && !b2 && !b3 && !b4) // The quad lies completely on or outside the plane
		{
			itInt = QuadIndices.erase(itStart, itInt); // Delete the quad
		}
		else if (b1 && b2 && b3 && b4) // The quad lies completely inside the plane
		{
			// Do nothing
		}
		else  // quad to intersections
		{
			itInt = QuadIndices.erase(itStart, itInt); // Delete the quad
			IntersectQuads.push_back(*p1);
			IntersectQuads.push_back(*p2);
			IntersectQuads.push_back(*p3);
			IntersectQuads.push_back(*p4);
		}
		//else if (!b1 && !b2 && b3 && b4) // Points 1 & 2 outside plane
		//{
		//	itInt = QuadIndices.erase(itStart, itInt); // Delete the quad
		//	//u = -d4 / GetLength(*p1, *p4);
		//	//
		//	////u = d4 / (d4 - d1);
		//	//NewQuads.push_back(*p4 + (*p1 - *p4) * u);
		//	//u = -d3 / GetLength(*p2, *p3);
		//	////u = d3 / (d3 - d2);
		//	//NewQuads.push_back(*p3 + (*p2 - *p3) * u);
		//	//NewQuads.push_back(*p3);
		//	//NewQuads.push_back(*p4);
		//}
		//else if (!b2 && !b3 && b4 && b1) // Points 2 & 3 outside plane
		//{
		//	itInt = QuadIndices.erase(itStart, itInt); // Delete the quad
		//	//NewQuads.push_back(*p1);
		//	//u = -d1 / GetLength(*p1, *p2);
		//	////u = d1 / (d1 - d2);
		//	//NewQuads.push_back(*p1 + (*p2 - *p1) * u);
		//	//u = -d4 / GetLength(*p4, *p3);
		//	////u = d4 / (d4 - d3);
		//	//NewQuads.push_back(*p4 + (*p3 - *p4) * u);
		//	//NewQuads.push_back(*p4);
		//}
		//else if (!b3 && !b4 && b1 && b2)  // Points 3 & 4 outside plane
		//{
		//	itInt = QuadIndices.erase(itStart, itInt); // Delete the quad
		//	//NewQuads.push_back(*p1);
		//	//NewQuads.push_back(*p2);
		//	//u = -d2 / GetLength(*p2, *p3);
		//	////u = d2 / (d2 - d3);
		//	//NewQuads.push_back(*p2 + (*p3 - *p2) * u);
		//	//u = -d1 / (*p1, *p4);
		//	////u = d1 / (d1 - d4);
		//	//NewQuads.push_back(*p1 + (*p4 - *p1) * u);
		//}
		//else if (!b4 && !b1 && b2 && b3)  // Points 4 & 1 outside plane
		//{
		//	itInt = QuadIndices.erase(itStart, itInt); // Delete the quad
		//	//u = -d2 / GetLength(*p2, *p1);
		//	////u = d2 / (d2 - d1);
		//	//NewQuads.push_back(*p2 + (*p1 - *p2) * u);
		//	//NewQuads.push_back(*p2);
		//	//NewQuads.push_back(*p3);
		//	//u = -d3 / GetLength(*p4, *p3);
		//	////u = d3 / (d3 - d4);
		//	//NewQuads.push_back(*p3 + (*p4 - *p3) * u);

		//}
		//else // Convert the quad to a triangle for trimming if 1 or 3 points inside plane
		//{
		//	//itInt = Mesh.ConvertQuadtoTriangles(itStart);
		//}
	}

	// Add the new quads to the mesh, and clear the new quad list
	/*iLastNodeIndex = int(Mesh.GetNumNodes());
	for (i = 0; i < int(NewQuads.size() / 4); ++i)
	{

		QuadIndices.push_back(4 * i + iLastNodeIndex);
		QuadIndices.push_back(4 * i + 1 + iLastNodeIndex);
		QuadIndices.push_back(4 * i + 2 + iLastNodeIndex);
		QuadIndices.push_back(4 * i + 3 + iLastNodeIndex);
	}
	for (itXYZ = NewQuads.begin(); itXYZ != NewQuads.end(); ++itXYZ)
	{
		Mesh.AddNode(*itXYZ);
	}
	NewQuads.clear();*/

	list<int> &IntersectQuadIndices = IntersectMesh.GetIndices(CMesh::QUAD);
	for (i = 0; i < int(IntersectQuads.size() / 4); ++i)
	{
		IntersectQuadIndices.push_back(4 * i);
		IntersectQuadIndices.push_back(4 * i + 1);
		IntersectQuadIndices.push_back(4 * i + 2);
		IntersectQuadIndices.push_back(4 * i + 3);
	}
	for (itXYZ = IntersectQuads.begin(); itXYZ != IntersectQuads.end(); ++itXYZ)
	{
		IntersectMesh.AddNode(*itXYZ);
	}
	IntersectQuads.clear();

	vector<XYZ> IntersectTriangles;

	list<int> &TriIndices = Mesh.GetIndices(CMesh::TRI);
	for (itInt = TriIndices.begin(); itInt != TriIndices.end(); )
	{
		itStart = itInt;

		double d1 = 0, d2 = 0, d3 = 0, d4 = 0;
		p1 = &Mesh.GetNode(*(itInt++));
		p2 = &Mesh.GetNode(*(itInt++));
		p3 = &Mesh.GetNode(*(itInt++));

		b1 = m_Yarn.PointInsideYarn(*p1, NULL, NULL, NULL, &d1);
		b2 = m_Yarn.PointInsideYarn(*p2, NULL, NULL, NULL, &d2);
		b3 = m_Yarn.PointInsideYarn(*p3, NULL, NULL, NULL, &d3);

		if (!b1 && !b2 && !b3) // The triangle lies completely outside or on the plane
		{
			itInt = TriIndices.erase(itStart, itInt); // Delete the triangle
		}
		else if (b1 && b2 && b3) // The triangle lies completely inside the plane
		{
			// Do nothing
		}
		else
		{
			itInt = TriIndices.erase(itStart, itInt);
			IntersectTriangles.push_back(*p1);
			IntersectTriangles.push_back(*p2);
			IntersectTriangles.push_back(*p3);
		}
	/*	else
		{
			itInt = TriIndices.erase(itStart, itInt); // Delete the triangle, will need to be seperated into smaller ones
														// Order points such that d1 >= d2 >= d3
			
			if (b1 && !b2 && !b3)
			{
				NewTriangles.push_back(*p1);
				u = -d1 / GetLength(*p1, *p2);
				NewTriangles.push_back(*p1 + (*p2 - *p1) * u);
				u = -d1 / GetLength(*p1, *p3);
				NewTriangles.push_back(*p1 + (*p3 - *p1) * u);
			}
			else if (b2 && !b1 && !b2)
			{
				NewTriangles.push_back(*p2);
				u = -d2 / GetLength(*p2, *p3);
				NewTriangles.push_back(*p2 + (*p3 - *p2) * u);
				u = -d2 / GetLength(*p1, *p2);
				NewTriangles.push_back(*p2 + (*p1 - *p2) * u);
			}
			else if (b3 && !b1 && !b2)
			{
				NewTriangles.push_back(*p3);
				u = -d3 / GetLength(*p1, *p3);
				NewTriangles.push_back(*p3 + (*p1 - *p3) * u);
				u = -d3 / GetLength(*p2, *p3);
				NewTriangles.push_back(*p3 + (*p2 - *p3) * u);
			}
			else if (b1 && b2)
			{
				NewTriangles.push_back(*p1);
				NewTriangles.push_back(*p2);
				u = -d2 / GetLength(*p2, *p3);
				NewTriangles.push_back(*p2 + (*p3 - *p2) * u);

				NewTriangles.push_back(*p2 + (*p3 - *p2) * u);
				u = -d1 / GetLength(*p1, *p3);
				NewTriangles.push_back(*p1 + (*p3 - *p1) * u);
				NewTriangles.push_back(*p1);
			}
			else if (b1 && b3)
			{
				NewTriangles.push_back(*p1);
				u = -d1 / GetLength(*p2, *p1);
				NewTriangles.push_back(*p1 + (*p2 - *p1) * u);
				u = -d3 / GetLength(*p3, *p2);
				NewTriangles.push_back(*p3 + (*p2 - *p3) * u);

				NewTriangles.push_back(*p3 + (*p2 - *p3) * u);
				NewTriangles.push_back(*p3);
				NewTriangles.push_back(*p1);
			}
			else  // b2 && b3
			{
				NewTriangles.push_back(*p2);
				NewTriangles.push_back(*p3);
				u = -d3 / GetLength(*p3, *p1);
				NewTriangles.push_back(*p3 + (*p1 - *p3) * u);

				NewTriangles.push_back(*p3 + (*p1 - *p3) * u);
				u = -d2 / GetLength(*p2, *p1);
				NewTriangles.push_back(*p2 + (*p1 - *p2) * u);
				NewTriangles.push_back(*p2);
			}
		}*/
	}

	list<int> &IntersectTriangleIndices = IntersectMesh.GetIndices(CMesh::TRI);
	iLastNodeIndex = IntersectMesh.GetNumNodes();
	for (i = 0; i < int(IntersectQuads.size() / 4); ++i)
	{
		IntersectTriangleIndices.push_back(4 * i + iLastNodeIndex); 
		IntersectTriangleIndices.push_back(4 * i + 1 + iLastNodeIndex);
		IntersectTriangleIndices.push_back(4 * i + 2 + iLastNodeIndex);
		IntersectTriangleIndices.push_back(4 * i + 3 + iLastNodeIndex);
	}
	for (itXYZ = IntersectTriangles.begin(); itXYZ != IntersectTriangles.end(); ++itXYZ)
	{
		IntersectMesh.AddNode(*itXYZ);
	}
	IntersectTriangles.clear();


	IntersectMesh.SaveToVTK("IntersectionMesh");

	ClipIntersectMeshToDomain(IntersectMesh);
	Mesh.InsertMesh(IntersectMesh);
	// Add the new triangles to the mesh, and clear the new triangles list
/*	iLastNodeIndex = int(Mesh.GetNumNodes());
	for (i = 0; i < int(NewTriangles.size() / 3); ++i)
	{
		TriIndices.push_back(3 * i + iLastNodeIndex);
		TriIndices.push_back(3 * i + 1 + iLastNodeIndex);
		TriIndices.push_back(3 * i + 2 + iLastNodeIndex);
	}

	for (itXYZ = NewTriangles.begin(); itXYZ != NewTriangles.end(); ++itXYZ)
	{
		Mesh.AddNode(*itXYZ);
	}
	NewTriangles.clear();*/

//	vector<int> ClosedLoop;
//	if (bFillGaps)
//		FillGaps(Mesh, *itPlane, ClosedLoop);
	

	// Deal with volume elements
/*	int iNumNodes;
	double d;
	vector<CMesh::ELEMENT_TYPE> VolumeElements;
	vector<CMesh::ELEMENT_TYPE>::iterator itElementType;
	VolumeElements.push_back(CMesh::TET);
	VolumeElements.push_back(CMesh::PYRAMID);
	VolumeElements.push_back(CMesh::WEDGE);
	VolumeElements.push_back(CMesh::HEX);
	VolumeElements.push_back(CMesh::QUADRATIC_TET);
	for (itPlane = m_ElementPlanes.begin(); itPlane != m_ElementPlanes.end(); ++itPlane)
	{
		for (itElementType = VolumeElements.begin(); itElementType != VolumeElements.end(); ++itElementType)
		{
			list<int> &Indices = Mesh.GetIndices(*itElementType);
			for (itInt = Indices.begin(); itInt != Indices.end(); )
			{
				iNumNodes = CMesh::GetNumNodes(*itElementType);
				itStart = itInt;

				XYZ Center;
				for (i = 0; i<iNumNodes; ++i)
				{
					Center += Mesh.GetNode(*(itInt++));
				}
				Center /= iNumNodes;

				d = DotProduct(itPlane->Normal, Center) - itPlane->d;
				if (d < 0)
					itInt = Indices.erase(itStart, itInt); // Delete the volume element
			}
		}
	}

	for (itPlane = m_ElementPlanes.begin(); itPlane != m_ElementPlanes.end(); ++itPlane)
	{
		list<int> &Indices = Mesh.GetIndices(CMesh::POLYGON);
		list<int>::iterator itIndices;
		int StartIndex, NewStartIndex;
		bool bResetStart = true;
		list<int>::iterator itStartIndex;
		bool bDelete = true;

		for (itIndices = Indices.begin(); itIndices != Indices.end(); )
		{
			bDelete = true;
			StartIndex = NewStartIndex = (*(itIndices));
			itStartIndex = itIndices;
			vector<int> PolygonIndex;
			XYZ p;
			bResetStart = true;
			int i = 0, iPoints = 0;
			do {
				p = Mesh.GetNode(*(itIndices));
				d = DotProduct(itPlane->Normal, p) - itPlane->d;
				if (d < TOL)
				{
					//if ( *itIndices == NewStartIndex )
					//bResetStart = false;  // Keeping 1st so don't need to reset
					//bDelete = false;
					++iPoints;
				}
				++i;
				++itIndices;
			} while ((*itIndices) != StartIndex);
			++itIndices;
			//if ( bDelete )
			if (iPoints == i) // All points outside plane
			{
				itIndices = Indices.erase(itStartIndex, itIndices);
			}
			/*if ( NewStartIndex != StartIndex ) // Must have deleted start so reset to new start value
			*(itIndices) = NewStartIndex;
			if ( NewStartIndex == StartIndex && bResetStart )  // Deleted whole loop so need to delete repeated start index
			itIndices = Indices.erase(itIndices);
			else
			itIndices++;*/
/*		}
	}

	Mesh.RemoveUnreferencedNodes();*/
}


bool CDomainPrism::FillGaps(CMesh &Mesh, const PLANE &Plane, vector<int> &Polygon, bool bMeshGaps)
{
	const double TOL = 1e-9;

	int i1, i2, i3, i4;
	const XYZ *p1, *p2, *p3, *p4;
	double d1, d2, d3, d4;	// d represents the distance of the point to the plane (i.e. +ve inside, -ve outside, 0 on top)

	vector<pair<int, int> > Segments;

	// Merge the nodes together and remove degenerate triangles
	Mesh.MergeNodes();
	Mesh.RemoveDegenerateTriangles();

	// Build a list of segments which lie on the plane
	list<int>::iterator itInt;
	// Check each quad to see if any of the edges lie on the plane
	list<int> &QuadIndices = Mesh.GetIndices(CMesh::QUAD);
	for (itInt = QuadIndices.begin(); itInt != QuadIndices.end(); )
	{
		i1 = *(itInt++);
		i2 = *(itInt++);
		i3 = *(itInt++);
		i4 = *(itInt++);

		p1 = &Mesh.GetNode(i1);
		p2 = &Mesh.GetNode(i2);
		p3 = &Mesh.GetNode(i3);
		p4 = &Mesh.GetNode(i4);

		d1 = DotProduct(Plane.Normal, *p1) - Plane.d;
		d2 = DotProduct(Plane.Normal, *p2) - Plane.d;
		d3 = DotProduct(Plane.Normal, *p3) - Plane.d;
		d4 = DotProduct(Plane.Normal, *p4) - Plane.d;

		d1 = abs(d1);
		d2 = abs(d2);
		d3 = abs(d3);
		d4 = abs(d4);

		// Add the segments which lie on the plane
		// The order of the segment indices is important, it tells us
		// which side is inside and which side is outside
		// Here the segments are added clockwise when viewed along the plane normal
		if (d1 <= TOL && d2 <= TOL)
		{
			Segments.push_back(pair<int, int>(i1, i2));
		}
		if (d2 <= TOL && d3 <= TOL)
		{
			Segments.push_back(pair<int, int>(i2, i3));
		}
		if (d3 <= TOL && d4 <= TOL)
		{
			Segments.push_back(pair<int, int>(i3, i4));
		}
		if (d4 <= TOL && d1 <= TOL)
		{
			Segments.push_back(pair<int, int>(i4, i1));
		}
	}
	// Check each triangle to find edges that lie on the plane
	list<int> &TriIndices = Mesh.GetIndices(CMesh::TRI);
	for (itInt = TriIndices.begin(); itInt != TriIndices.end(); )
	{
		i1 = *(itInt++);
		i2 = *(itInt++);
		i3 = *(itInt++);

		p1 = &Mesh.GetNode(i1);
		p2 = &Mesh.GetNode(i2);
		p3 = &Mesh.GetNode(i3);

		d1 = DotProduct(Plane.Normal, *p1) - Plane.d;
		d2 = DotProduct(Plane.Normal, *p2) - Plane.d;
		d3 = DotProduct(Plane.Normal, *p3) - Plane.d;

		d1 = abs(d1);
		d2 = abs(d2);
		d3 = abs(d3);

		// Add the segments which lie on the plane
		// The order of the segment indices is important, it tells us
		// which side is inside and which side is outside
		// Here the segments are added clockwise when viewed along the plane normal
		if (d1 <= TOL && d2 <= TOL)
		{
			Segments.push_back(pair<int, int>(i1, i2));
		}
		if (d2 <= TOL && d3 <= TOL)
		{
			Segments.push_back(pair<int, int>(i2, i3));
		}
		if (d3 <= TOL && d1 <= TOL)
		{
			Segments.push_back(pair<int, int>(i3, i1));
		}
	}

	vector<pair<int, int> >::iterator itSegment;

	// Find closed loops
	int iIndex, iFirstIndex;
	bool bFound;

#ifdef _DEBUG
	// This code will help to find out where the source of the problem comes from.
	// It basically checks how many times each node has been referenced by the segments.
	// Each node should be referenced twice, otherwise there will be a problem.

	map<int, int> Indices;
	map<int, int>::iterator itIndex;

	for (itSegment = Segments.begin(); itSegment != Segments.end(); ++itSegment)
	{
		Indices[itSegment->first]++;
		Indices[itSegment->second]++;
	}
#endif

	while (Segments.size())
	{
		vector<int> ClosedLoop;
		// Start at a random index and go round counterclockwise until a full circle is done
		// Indices are added to the ClosedLoop list with each index specified only once.
		// As segments are followed they are removed from the segments list.
		iFirstIndex = iIndex = Segments.begin()->first;
		do
		{
			bFound = false;
			for (itSegment = Segments.begin(); itSegment != Segments.end(); ++itSegment)
			{
				// Follow segments counter-clockwise only
				if (itSegment->second == iIndex)
				{
					// Adjust the index to go follow the segment
					iIndex = itSegment->first;
					// Delete the segment, it is no longer needed
					itSegment = Segments.erase(itSegment);
					// Found the segment, now stop searching
					bFound = true;
					break;
				}
				// This part is commented because we only want to go counter-clockwise
				/*				else if (itSegment->first == iIndex)
				{
				iIndex = itSegment->second;
				itSegment = Segments.erase(itSegment);
				bFound = true;
				break;
				}*/
			}
			if (bFound)
			{
				// Add the index to the loop
				ClosedLoop.push_back(iIndex);
				// If the index is the same as the first index then a full circle
				// has been made, its time to exit the loop
				if (iFirstIndex == iIndex)
					break;
			}
		} while (bFound);

		// If a dead end was reached the bFound will be false. This means that the segments
		// do not form a fully closed loop. This can occur if nodes are not merged together
		// correctly (two nodes may occupy the same position where each one is only referenced
		// once which will cause a break in the loop).
		if (!bFound)
		{
			// Report the error
			TGERROR("Unable to fill gaps satisfactorily");

#ifdef _DEBUG
			// Print out the total number of nodes
			TGLOG("Number of nodes: " << Indices.size());

			// Print out the number of nodes referenced more than once
			//			for (itIndex = Indices.begin(); itIndex != Indices.end(); ++itIndex)
			//			{
			//				if (itIndex->second > 1)
			//					cout << "Node " << itIndex->first << " referenced " << itIndex->second << " times. (" << Mesh.m_Nodes[itIndex->first] << ")" << endl;
			//			}
			// Print out the number of nodes referenced only once
			for (itIndex = Indices.begin(); itIndex != Indices.end(); ++itIndex)
			{
				if (itIndex->second != 2)
					TGLOG("Node " << itIndex->first << " referenced " << itIndex->second << " times. (" << Mesh.GetNode(itIndex->first) << ")");
			}

			//assert(false);
#endif
			return false;
		}

		// Check for two points next to each other which are the same
		// Happens on issue #2 - would be better to find cause
		vector<int>::iterator itCurrent = ClosedLoop.begin();
		vector<int>::iterator itNext = itCurrent + 1;
		while (itNext != ClosedLoop.end())
		{
			if (*itCurrent == *itNext)
			{
				itNext = ClosedLoop.erase(itNext);
			}
			else
			{
				++itCurrent;
				++itNext;
			}
		}

		// Check for spike in loop where points two apart are the same
		// Would be better to find the root cause of this happening

		vector<int>::iterator itPrev = ClosedLoop.begin();
		itCurrent = itPrev + 1;
		itNext = itCurrent + 1;

		while (itNext != ClosedLoop.end())
		{
			if (*itPrev == *itNext)
			{
				ClosedLoop.erase(itCurrent, itNext + 1);
				if (itPrev != ClosedLoop.begin())
				{
					itCurrent = itPrev;
					--itPrev;
					itNext = itCurrent + 1;
				}
			}
			else
			{
				// Go to the next set of 3 points
				++itPrev;
				++itCurrent;
				++itNext;
				if (itPrev == ClosedLoop.end())
					itPrev = ClosedLoop.begin();
			}
		}

		Polygon.insert(Polygon.begin(), ClosedLoop.begin(), ClosedLoop.end());
		// Mesh the closed loop
		if (bMeshGaps)  // If just saving closed loop don't want to fill in end
			Mesh.MeshClosedLoop(Plane.Normal, ClosedLoop);
	};

	return true;

}