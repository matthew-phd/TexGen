#pragma once

namespace TexGen
{
	using namespace std;

	/// Class used to generate Abaqus input file for periodic boundary conditions for CTextilebraidCurved
	class CLASS_DECLSPEC CArcPeriodicBoundaries : public CPeriodicBoundaries
	{
	public:
		CArcPeriodicBoundaries(void);
		~CArcPeriodicBoundaries(void);
	};
}