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

		void SetDomainSize(const CMesh& Mesh);
		void OutputLoadCase(ostream &Output, int iCase);
		void OutputEdgeSets(ostream& Output);
		void OutputStep(ostream& Output, int iBoundaryConditions);
	private:
		void OutputEquations(ostream& Output, int iBoundaryConitions);
		void OutputFaceSets(ostream& Ouput);
		
		
		double m_dMeshVol;
	};
}