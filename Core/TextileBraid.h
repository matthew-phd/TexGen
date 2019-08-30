#include "Textile.h"
#include "DomainPlanes.h"
#include "mymath.h"


#pragma once
namespace TexGen
{
	using namespace std;
	class CPatternDraft;

	enum
	{
		PATTERN_WEFTYARN = 1,
		PATTERN_WARPYARN = 0,
	};

	typedef bool PATTERNBIAX;
	/// Represents a biaxal braided textile unit cell 
	class CLASS_DECLSPEC CTextileBraid: public CTextile
	{
	public:
		CTextileBraid(int iNumWeftYarns, int iNumWarpYarns, double dWidth, double dHeight, double dSpacing, double dThickness, double dBraidAngle);
		virtual ~CTextileBraid();
		virtual CTextile* Copy() const { return new CTextileBraid(*this); }
		const vector<PATTERNBIAX> &GetCell(int x, int y) const;
		double GetWidthWarp() const;
		double GetHeightWarp() const;
		double GetWidthWeft() const;
		double GetHeightWeft() const;
		//void GetWarpSpacings() const;
		//void GetWeftSpacings() const;
		void SwapPosition(int x, int y);
		virtual CDomainPlanes GetDefaultDomain(bool bSheared = false, bool bAddedHeight = true);
		void AssignDefaultDomain(bool bSheared, bool bAddedHeight);
		int GetNumWeftYarns() const { return m_iNumWeftYarns; }
		int GetNumWarpYarns() const { return m_iNumWarpYarns; }
		double GetYarnSpacing() const; 
		double GetYarnWidth() const;
		double GetFabricThickness() const { return m_dFabricThickness; }
		
	protected:
		vector<PATTERNBIAX> &GetCell(int x, int y);
		bool BuildTextile() const;

		struct YARNDATA
		{
			double dWidth;
			double dHeight;
			double dSpacing;
		};

		int m_iNumWeftYarns, m_iNumWarpYarns;
		vector<vector<PATTERNBIAX> > m_Pattern;
		double m_dGapSize;
		double m_dFabricThickness;
		double m_dbraidAngle;
		int m_iResolution;
		CObjectContainer<CSectionMesh> m_pSectionMesh;

		mutable vector<YARNDATA> m_WeftYarnData;
		mutable vector<YARNDATA> m_WarpYarnData;
		mutable vector<vector<int> > m_WeftYarns;
		mutable vector<vector<int> > m_WarpYarns;
	};

}