#pragma once

class CBraidWizard : public wxWizard
{
public:
	CBraidWizard(wxWindow* parent, wxWindowID id = wxID_ANY);
	CBraidWizard(void);
	bool RunIt();
	string getCreateTextileCommand(string ExisitingTextile = "");
	void LoadSettings(const CTextileBraid& Braid);

protected:
	void BuildPages();
	wxWizardPageSimple* BuildFirstPage();
	//wxDialog* BuildBraidPatternDialog();
	void OnWidthChanged(wxCommandEvent& event) { m_bWidthChanged = true; }
	void OnSpacingChanged(wxCommandEvent& event) { m_bSpacingChanged = true; }
	void OnThicknessChanged(wxCommandEvent& event) { m_bThicknessChanged = true; }
	void OnInit(wxInitDialogEvent& event) { m_bWidthChanged = m_bSpacingChanged = m_bThicknessChanged = false; }

	wxSpinCtrl *m_pWeftYarnsSpin;
	wxSpinCtrl *m_pWarpYarnsSpin;
	// wxBraidPatternCtrl *m_pBraidPatternCtrl;
	wxString m_YarnSpacing;
	wxString m_YarnWidth;
	wxString m_FabricThickness;
	//wxString m_GapSize;
	wxString m_BraidAngle;

	bool m_bCreateDomain;
	//bool m_bAddedDomainHeight;

	bool m_bWidthChanged;
	bool m_bSpacingChanged;
	bool m_bThicknessChanged;

	wxWizardPageSimple *m_pFirstPage;
	//wxDialog *m_pBraidPatternDialog; 

	DECLARE_EVENT_TABLE()
};