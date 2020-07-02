#include "PrecompiledHeaders.h"
#include "TexGen.h"
#include "ArcPeriodicBoundaries.h"

using namespace TexGen;
using namespace std;

CArcPeriodicBoundaries::CArcPeriodicBoundaries(void)
{

}

CArcPeriodicBoundaries::~CArcPeriodicBoundaries(void)
{

}

void CArcPeriodicBoundaries::OutputFaceSets(ostream& Output)
{
	vector<int> FullFaceA;
	
	for (int i = 0; i < m_FaceA.first.size(); i++){ FullFaceA.push_back(m_FaceA.first[i]); }
	for (int i = 0; i < m_Edges[1].size(); i++) { FullFaceA.push_back(m_Edges[1][i]); }
	for (int i = 0; i < m_Edges[2].size(); i++) { FullFaceA.push_back(m_Edges[2][i]); }
	for (int i = 0; i < m_Edges[5].size(); i++) { FullFaceA.push_back(m_Edges[5][i]); }
	for (int i = 0; i < m_Edges[6].size(); i++) { FullFaceA.push_back(m_Edges[6][i]); }
	FullFaceA.push_back(m_Vertices[1]);
	FullFaceA.push_back(m_Vertices[2]);
	FullFaceA.push_back(m_Vertices[5]);
	FullFaceA.push_back(m_Vertices[6]);

	vector<int> FullFaceB;

	for (int i = 0; i < m_FaceA.second.size(); i++) { FullFaceB.push_back(m_FaceA.second[i]); }
	for (int i = 0; i < m_Edges[0].size(); i++) { FullFaceB.push_back(m_Edges[0][i]); }
	for (int i = 0; i < m_Edges[3].size(); i++) { FullFaceB.push_back(m_Edges[3][i]); }
	for (int i = 0; i < m_Edges[4].size(); i++) { FullFaceB.push_back(m_Edges[4][i]); }
	for (int i = 0; i < m_Edges[7].size(); i++) { FullFaceB.push_back(m_Edges[7][i]); }
	FullFaceB.push_back(m_Vertices[0]);
	FullFaceB.push_back(m_Vertices[3]);
	FullFaceB.push_back(m_Vertices[4]);
	FullFaceB.push_back(m_Vertices[7]);

	vector<int> FullFaceC;

	for (int i = 0; i < m_FaceB.first.size(); i++) { FullFaceC.push_back(m_FaceB.first[i]); }
	for (int i = 0; i < m_Edges[2].size(); i++) { FullFaceC.push_back(m_Edges[2][i]); }
	for (int i = 0; i < m_Edges[3].size(); i++) { FullFaceC.push_back(m_Edges[3][i]); }
	for (int i = 0; i < m_Edges[9].size(); i++) { FullFaceC.push_back(m_Edges[9][i]); }
	for (int i = 0; i < m_Edges[10].size(); i++) { FullFaceC.push_back(m_Edges[10][i]); }
	FullFaceC.push_back(m_Vertices[2]);
	FullFaceC.push_back(m_Vertices[3]);
	FullFaceC.push_back(m_Vertices[6]);
	FullFaceC.push_back(m_Vertices[7]);

	vector<int> FullFaceD;

	for (int i = 0; i < m_FaceB.second.size(); i++) { FullFaceD.push_back(m_FaceB.second[i]); }
	for (int i = 0; i < m_Edges[0].size(); i++) { FullFaceD.push_back(m_Edges[0][i]); }
	for (int i = 0; i < m_Edges[1].size(); i++) { FullFaceD.push_back(m_Edges[1][i]); }
	for (int i = 0; i < m_Edges[8].size(); i++) { FullFaceD.push_back(m_Edges[8][i]); }
	for (int i = 0; i < m_Edges[11].size(); i++) { FullFaceD.push_back(m_Edges[11][i]); }
	FullFaceD.push_back(m_Vertices[0]);
	FullFaceD.push_back(m_Vertices[1]);
	FullFaceD.push_back(m_Vertices[4]);
	FullFaceD.push_back(m_Vertices[5]);


	OutputSets(Output, FullFaceA, "FaceA");
	OutputSets(Output, FullFaceB, "FaceB");
	OutputSets(Output, FullFaceC, "FaceC");
	OutputSets(Output, FullFaceD, "FaceD");
	OutputSets(Output, m_FaceC.first, "FaceE");
	OutputSets(Output, m_FaceC.second, "FaceF");
}
void CArcPeriodicBoundaries::SetDomainSize(const CMesh& Mesh)
{
	m_dMeshVol = Mesh.CalculateVolume();
	XYZ StartPoint = Mesh.GetNode(0);
	
	XYZ Node2 = Mesh.GetNode(Mesh.GetNumNodes()-1);

	m_DomSize.y = GetLength(StartPoint, Node2);
}

void CArcPeriodicBoundaries::OutputEquations(ostream& Output, int iBoundaryConditions)
{
	Output << "***************************" << endl;
	Output << "*** BOUNDARY CONDITIONS ***" << endl;
	Output << "***************************" << endl;

	Output << "*** Name: Translation stop Vertex 1 Type: Displacement/Rotation" << endl;
	Output << "*Boundary" << endl;
	Output << "FaceB, 3, 3" << endl;
	Output << "FaceD, 2, 2" << endl;
	Output << endl;

	Output << "*****************" << endl;
	Output << "*** EQUATIONS ***" << endl;
	Output << "*****************" << endl;
	Output << "*Equation" << endl;
	Output << "2" << endl;
	Output << "FaceC, 2, 1.0, ConstraintsDriver1, 1, -" << m_DomSize.y << endl;
	Output << "*Equation" << endl;
	Output << "2" << endl;
	Output << "FaceA, 1, -0.16041128, FaceA, 3, 0.9870502" << endl;
}

void CArcPeriodicBoundaries::OutputLoadCase(ostream &Output, int iCase)
{
	Output << "*Load Case, name=Load" << endl;
	Output << "*Cload" << endl;
	Output << "ConstraintsDriver2, 1, " << m_dMeshVol << endl;
	Output << "*End Load Case" << endl;
	Output << endl;
}

void CArcPeriodicBoundaries::OutputStep(ostream& Output, int iBoundaryConditions)
{
	Output << "*******************" << endl;
	Output << "*** CREATE STEP ***" << endl;
	Output << "*******************" << endl;

	Output << "*** PREDEFINED FIELDS ***" << endl;
	Output << "*** Name: Initial temperature 0ºC all cells   Type: Temperature ***" << endl;
	Output << "*Initial Conditions, type=TEMPERATURE" << endl;
	Output << "AllNodes, 0." << endl;
	Output << endl;
	Output << "*Step, Name=Isothermal linear perturbation step, perturbation" << endl;
	Output << "Elastic material property computation" << endl;
	Output << "*Static" << endl;
	Output << endl;
	Output << "***********************" << endl;
	Output << "*** OUTPUT REQUESTS ***" << endl;
	Output << "***********************" << endl;
	Output << "*Output, field" << endl;

	
	Output << "*Element Output, directions=YES" << endl << "S," << endl;
	

	Output << "*** FIELD OUTPUT: Output Request Fy ***" << endl;
	Output << "*Node Output, nset=ConstraintsDriver1" << endl << "U," << endl;
	

	Output << endl;

	Output << "******************" << endl;
	Output << "*** LOAD CASES ***" << endl;
	Output << "******************" << endl;

	OutputLoadCase(Output, 1);
	

	Output << endl;

	Output << "*End Step" << endl;
	Output << endl;


}