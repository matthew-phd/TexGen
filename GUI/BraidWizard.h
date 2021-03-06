#pragma once

class wxBraidPatternCtrl;

class CBraidWizard : public wxWizard
{
public:
	CBraidWizard(wxWindow* parent, wxWindowID id = wxID_ANY);
	~CBraidWizard(void);
	bool RunIt();
	string GetCreateTextileCommand(string ExisitingTextile = "");
	void LoadSettings(const CTextileBraid& Braid);

protected:
	void OnWizardPageChanging(wxWizardEvent& event);
	void BuildPages();
	wxWizardPageSimple* BuildFirstPage();
	wxDialog* BuildBraidPatternDialog();
	bool RebuildBraidPatternCtrl();
	//wxWizardPageSimple* BuildPatternPage();
	//wxDialog* BuildBraidPatternDialog();
	void OnWidthChanged(wxCommandEvent& event) { m_bWidthChanged = true; }
	void OnSpacingChanged(wxCommandEvent& event) { m_bSpacingChanged = true; }
	void OnThicknessChanged(wxCommandEvent& event) { m_bThicknessChanged = true; }
	void OnInit(wxInitDialogEvent& event) { m_bWidthChanged = m_bSpacingChanged = m_bThicknessChanged = false; }
	bool GetPatternCell(int i, int y);
	wxSpinCtrl *m_pWeftYarnsSpin;
	wxSpinCtrl *m_pWarpYarnsSpin;
	wxBraidPatternCtrl *m_pBraidPatternCtrl;
	wxString m_YarnSpacing;
	wxString m_YarnWidth;
	wxString m_FabricThickness;
	wxString m_BraidAngle;
	wxString m_Radius;
	wxString m_HornGearVelocity;
	wxSpinCtrl *m_pHornGearSpin;
	wxString m_Velocity;
	//wxString m_BraidPattern;
	wxChoice* pBraidPattern;
	bool m_bCreateDomain;
	bool m_bRefine;
	bool m_bCurved;
	bool m_bAdjustSpacing;
	bool m_bLayers;
	
	
	bool m_bWidthChanged;
	bool m_bSpacingChanged;
	bool m_bThicknessChanged;

	wxWizardPageSimple *m_pFirstPage;
	wxDialog *m_pBraidPatternDialog;
	//wxWizardPageSimple *m_pBraidPatternPage;
	//wxDialog *m_pBraidPatternDialog; 

	void OnCurved(wxCommandEvent& event);
	void OnLayers(wxCommandEvent& event);
	void RefreshGapTextBox();

	wxSpinCtrl *m_pLayersSpin;


	DECLARE_EVENT_TABLE()
};