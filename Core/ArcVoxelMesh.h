
#pragma once
#include "VoxelMesh.h"

namespace TexGen
{
	using namespace std;

	class CTextile;

	/// Class used to generate voxel mesh for output to ABAQUS
	class CLASS_DECLSPEC CArcVoxelMesh : public CVoxelMesh
	{
	public:
		CArcVoxelMesh(string Type = "CShearedPeriodicBoundaries");
		virtual ~CArcVoxelMesh(void);
	
		bool CalculateVoxelSizes(CTextile &Textile);
		void OutputNodes(ostream &Output, CTextile &Textile, int Filetype = INP_EXPORT);

		RThetaZ m_ArcPolarSize;
		XYZ m_ArcVoxSize;
		XYZ m_StartPoint;
	};
};