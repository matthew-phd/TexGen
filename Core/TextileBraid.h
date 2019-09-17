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
		string GetType() const { return "CTextileBraid"; }
		const vector<PATTERNBIAX> &GetCell(int x, int y) const;
		double GetWidthWarp() const;
		double GetHeightWarp() const;
		double GetWidthWeft() const;
		double GetHeightWeft() const;
		double GetWidth() const; // Calculates the width of the unit cell
		double GetHeight() const; // Calculates the height of the unit cell
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

		double GetWeftYarnWidth(int iIndex) const;
		double GetWarpYarnWidth(int iIndex) const;
		double GetWeftYarnHeight(int iIndex) const;
		double GetWarpYarnHeight(int iIndex) const;
		double GetWeftYarnSpacing(int iIndex) const;
		double GetWarpYarnSpacing(int iIndex) const;
		double GetWeftGapSize(int iIndex) const;
		double GetWarpGapSize(int iIndex) const;

		void SetWarpYarnWidths(int iIndex, double dWidth);
		void SetWeftYarnWidths(int iIndex, double dWidht);
		void SetWarpYarnHeights(int iIndex, double dHeight);
		void SetWeftYarnHeights(int iIndex, double dHeight);
		void SetWarpYarnSpacings(int iIndex, double dSpacing);
		void SetWeftYarnSpacings(int iIndex, double dSpacing);

		void SetWeftYarnWidths(double dWdidth);
		void SetWarpYarnWidths(double dWdidth);
		void SetWeftYarnHeights(double dHeight);
		void SetWarpYarnHeights(double dHeight);
		void SetWeftYarnSpacings(double dSpacing);
		void SetWarpYarnSpacings(double dSpacing);

		void SetYarnWidths(double dWidth);
		void SetYarnHeights(double dHeight);
		void SetYarnSpacings(double dSpacing);

		void SetThickness(double dThickness);
		void SetResolution(int iResolution);


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