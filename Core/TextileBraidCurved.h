#pragma once 
#include "TextileBraid.h"

namespace TexGen
{
	class CSectionEllipse;
	using namespace std;

	class CLASS_DECLSPEC CTextileBraidCurved : public CTextileBraid
	{
	public:
		CTextileBraidCurved(int iNumWeftYarns, int iNumWarpYarns, double dWidth,
			double dHeight, double dThickness,
			double dRadius, double dHornGearVelocity, int iNumHornGear,
			double dVelocity, bool bCurved, bool bRefine, int iNumLayers);
		virtual ~CTextileBraidCurved(void);
		CTextile* Copy() const { return new CTextileBraidCurved(*this);}
		string GetType() const { return "CTextileBraidCurved"; }
		double GetBraidAngle() const { return m_dbraidAngle; }
		virtual CDomainPrism GetDefaultCurvedDomain(bool bAddedHeight = true);
		double DomainAngle();
		void AssignDefaultDomain(bool bAddedHeight);
		void CheckUpVectors(int WarpIndex, bool Yarn = PATTERN_WEFTYARN) const;
		double GetNodeRotation(int YarnIndex, int NodeIndex) const;
		void AddAdditionalNodes() const; 
		mutable vector<vector<RThetaZ> >PolarCoor;
		

	protected:
		bool BuildTextile() const;
		bool m_bCurved;
		void Refine(bool bCorrectWidths = true, bool bPeriodic = true) const;
		void CorrectBraidYarnWidths() const;
		bool AdjustSectionsForRotation(bool bPeriodic) const;
		void CorrectInterference() const;
		XYZ GetNodeTangents(int YarnIndex, int NodeIndex) const;
		XYZ GetUpVector(XYZ CheckNodes[], bool bYarn) const; 
		void SaveCrossSection(string& FileName, int YarnIndex, int NodeIndex) const;
		double ReturnNodeRotation(int YarnIndex, int NodeIndex) const;
		mutable vector<vector<double>> m_YarnAngles;
		int iNumLayers;

		
		
	};
}