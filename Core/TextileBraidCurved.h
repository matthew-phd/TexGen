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
			double dVelocity, bool bCurved, bool bRefine);
		virtual ~CTextileBraidCurved(void);
		CTextile* Copy() const { return new CTextileBraidCurved(*this);}
		string GetType() const { return "CTextileBraidCurved"; }
		double GetBraidAngle() const { return m_dbraidAngle; }
		virtual CDomainPrism GetDefaultCurvedDomain(bool bAddedHeight = true);
		double Angle();
		void AssignDefaultDomain(bool bAddedHeight);
		void CheckUpVectors(int WarpIndex, bool Yarn = PATTERN_WEFTYARN) const;
		double GetNodeRotation(int YarnIndex, int NodeIndex) const;

	
	protected:
		bool BuildTextile() const;
		bool m_bCurved;

		XYZ GetUpVector(XYZ CheckNodes[], bool bYarn) const;

		
	};
}