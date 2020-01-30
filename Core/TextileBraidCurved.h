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
		virtual CDomainPlanes GetDefaultDomain(bool bSheared = false, bool bAddedHeight = true);

	
	protected:
		bool BuildTextile() const;
		bool m_bCurved;

		
	};
}