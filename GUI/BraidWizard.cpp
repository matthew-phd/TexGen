#include "PrecompiledHeaders.h"
#include "BraidWizard.h"
#include "WindowIDs.h"
#include "PythonConverter.h"
#include "Wizard.xpm"
#include "RangeValidator.h"
#include "BraidPatternCtrl.h"
//#include "BraidPatternCtrl.h"

BEGIN_EVENT_TABLE(CBraidWizard, wxWizard)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CBraidWizard::OnWizardPageChanging)
	EVT_TEXT(ID_Width, CBraidWizard::OnWidthChanged)
	EVT_TEXT(ID_Spacing, CBraidWizard::OnSpacingChanged)
	EVT_TEXT(ID_Thickness, CBraidWizard::OnThicknessChanged)
	EVT_INIT_DIALOG(CBraidWizard::OnInit)
END_EVENT_TABLE()

CBraidWizard::CBraidWizard(wxWindow* parent, wxWindowID id)
	: wxWizard(parent, id, wxT("Braid Wizard"), wxBitmap(Wizard_xpm))
	, m_pFirstPage(NULL)
	//, m_pBraidPatternDialog(NULL)
	//, m_pBraidPatternCtrl(NULL)
	, m_pWeftYarnsSpin(NULL)
	, m_pWarpYarnsSpin(NULL)
	, m_YarnSpacing(wxT("3.5"))
	, m_YarnWidth(wxT("3"))
	, m_FabricThickness(wxT("1.0"))
	, m_BraidAngle(wxT("55"))
	, m_bCreateDomain(true)
	, m_bWidthChanged(false)
	, m_bSpacingChanged(false)
	, m_bThicknessChanged(false)
	//, m_bAddedDomainHeight(true)
{
	BuildPages();
	GetPageAreaSizer()->Add(m_pFirstPage);
}

CBraidWizard::CBraidWizard(void)
{
	if (m_pBraidPatternDialog)
		m_pBraidPatternDialog->Destroy(); 
}

bool CBraidWizard::RunIt()
{
	return RunWizard(m_pFirstPage);
}

void CBraidWizard::BuildPages()
{
	m_pFirstPage = BuildFirstPage();
	m_pBraidPatternDialog = BuildBraidPatternDialog();

	
}

wxWizardPageSimple* CBraidWizard::BuildFirstPage()
{
	wxWizardPageSimple *pPage = new wxWizardPageSimple(this);

	wxBoxSizer *pMainSizer = new wxBoxSizer(wxVERTICAL);
	wxSizerFlags SizerFlags(0);
	SizerFlags.Border();
	SizerFlags.Expand();

	wxSizer *pSubSizer;

	pMainSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("This wizard will create a biaxial braid model for you.")), SizerFlags);

	SizerFlags.Align(wxALIGN_CENTER_VERTICAL);
	pSubSizer = new wxFlexGridSizer(2);
	{
		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Weft Yarns:")), SizerFlags);
		pSubSizer->Add(m_pWeftYarnsSpin = new wxSpinCtrl(pPage, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100), SizerFlags);
		m_pWeftYarnsSpin->SetToolTip(wxT("Controls the number of weft yarns contained within the unit cell"));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Warp Yarns:")), SizerFlags);
		pSubSizer->Add(m_pWarpYarnsSpin = new wxSpinCtrl(pPage, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100), SizerFlags);
		m_pWarpYarnsSpin->SetToolTip(wxT("Controls the number of warp yarns contained within the unit cell"));

		wxTextCtrl* pControl;
		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Yarn Spacing:")), SizerFlags);
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_Spacing, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_YarnSpacing)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the spacing between yarns"));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Yarn Width:")), SizerFlags);
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_Width, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_YarnWidth)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the width of the yarns"));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Fabric Thickness:")), SizerFlags);
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_Thickness, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_FabricThickness)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the thickness of the fabic"));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Braid Angle:")), SizerFlags);
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_BraidAngle, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, RangeValidator( &m_BraidAngle, 0, 89)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the braid angle of the fabic"));

		
		
		const wxString choices[3] = { "Diamond", "Regular", "Hercules" };
		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Braid Pattern: ")), SizerFlags);
		pSubSizer->Add(pBraidPattern = new wxChoice(pPage, ID_BraidPattern, wxDefaultPosition, wxDefaultSize, 3, choices,0, wxDefaultValidator), SizerFlags);
		wxCheckBox* pDomainBox;
		pSubSizer->Add(pDomainBox = new wxCheckBox(pPage, ID_DefaultDomain, wxT("Create Default Domain"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&m_bCreateDomain)), SizerFlags);
		
		
	}
	pMainSizer->Add(pSubSizer, SizerFlags);
	SizerFlags.Align(0);

	pPage->SetSizer(pMainSizer);
	pMainSizer->Fit(pPage);

	m_pWeftYarnsSpin->SetValue(2);
	m_pWarpYarnsSpin->SetValue(2);


	return pPage; 

}
wxDialog* CBraidWizard::BuildBraidPatternDialog()
{
	//	wxDialog* pDailog = new wxDialog(NULL, wxID_ANY, wxT("Weave pattern"));
	wxDialog* pDailog = new wxDialog();

	pDailog->Create(this, wxID_ANY, wxT("Braid pattern"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

	wxBoxSizer *pMainSizer = new wxBoxSizer(wxVERTICAL);
	wxSizerFlags SizerFlags(0);
	SizerFlags.Border();
	SizerFlags.Expand();

	pMainSizer->Add(new wxStaticText(pDailog, wxID_ANY, wxT("Set the braid pattern. Right click on top or side bars to change individual yarn settings")), SizerFlags);

	SizerFlags.Proportion(1);

	m_pBraidPatternCtrl = new wxBraidPatternCtrl(pDailog, wxID_ANY);
	pMainSizer->Add(m_pBraidPatternCtrl, SizerFlags);

	wxSizer* pSubSizer = pDailog->CreateStdDialogButtonSizer(wxOK | wxCANCEL);
	if (pSubSizer)
	{
		SizerFlags.Proportion(0);
		pMainSizer->Add(pSubSizer, SizerFlags);
	}

	pDailog->SetSizer(pMainSizer);
	pMainSizer->Fit(pDailog);

	return pDailog;
}

/*wxWizardPageSimple* CBraidWizard::BuildPatternPage()
{
	wxWizardPageSimple *pPage = new wxWizardPageSimple(this);

	wxBoxSizer *pMainSizer = new wxBoxSizer(wxVERTICAL);
	wxSizerFlags SizerFlags(0);
	SizerFlags.Border();
	SizerFlags.Expand();

	wxSizer *pSubSizer;
	pMainSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Decide the Pattern of the braided fabic: ")), SizerFlags);

}
*/
string CBraidWizard::getCreateTextileCommand(string ExistingTextile)
{
	stringstream StringStream;
	double dYarnSpacing, dFabricThickness, dWidth, dHeight, dBraidAngle;
	int iNumWeftYarns, iNumWarpYarns;
	string braidPattern;
	m_YarnSpacing.ToDouble(&dYarnSpacing);
	m_YarnWidth.ToDouble(&dWidth);
	m_FabricThickness.ToDouble(&dFabricThickness);
	m_BraidAngle.ToDouble(&dBraidAngle);
	dHeight = dFabricThickness / 2;
	iNumWeftYarns = m_pWeftYarnsSpin->GetValue();
	iNumWarpYarns = m_pWarpYarnsSpin->GetValue();
	
	braidPattern = pBraidPattern->GetString(pBraidPattern->GetSelection());
	if (braidPattern == "Diamond")
	{
		StringStream << "braid = CTextileBraid(" << 2 << ", " << 2 << ", " << dWidth << ", " << dHeight << ", " << dYarnSpacing << ", " << dFabricThickness << ", " << dBraidAngle * PI / 180.0 << ")" << endl;
			for (int i = 0; i <= 2; i++) {
				for (int j = 0; j <= 2; j++) {
					if (i % 2 == 0 && j% 2 != 0)
					{
						StringStream << "braid.SwapPosition(" << i <<","<< j << ")" << endl;
					}
					if (i % 2 != 0 && j % 2 == 0)
					{
						StringStream << "braid.SwapPosition(" << i << "," << j << ")" << endl;
					}
				}
			}
			
	
	}
	else if (braidPattern == "Regular")
	{
		StringStream << "braid = CTextileBraid(" << 4 << ", " << 4 << ", " << dWidth << ", " << dHeight << ", " << dYarnSpacing << ", " << dFabricThickness << ", " << dBraidAngle * PI / 180.0 << ")" << endl;
		StringStream << "braid.SwapPosition(0,2)" << endl;
		StringStream << "braid.SwapPosition(0,3)" << endl;
		StringStream << "braid.SwapPosition(1,1)" << endl;
		StringStream << "braid.SwapPosition(1,2)" << endl;
		StringStream << "braid.SwapPosition(2,1)" << endl;
		StringStream << "braid.SwapPosition(2,0)" << endl;
		StringStream << "braid.SwapPosition(3,0)" << endl;
		StringStream << "braid.SwapPosition(3,3)" << endl;


	}
	else if (braidPattern == "Hercules")
	{
		StringStream << "braid = CTextileBraid(" << 6 << ", " << 6 << ", " << dWidth << ", " << dHeight << ", " << dYarnSpacing << ", " << dFabricThickness << ", " << dBraidAngle * PI / 180.0 << ")" << endl;
		StringStream << "braid.SwapPosition(0,3)" << endl;
		StringStream << "braid.SwapPosition(0,4)" << endl;
		StringStream << "briad.SwapPosition(0,5)" << endl;
		StringStream << "braid.SwapPosition(1,3)" << endl;
		StringStream << "braid.SwapPosition(1,4)" << endl;
		StringStream << "briad.SwapPosition(1,5)" << endl;
		StringStream << "braid.SwapPosition(2,3)" << endl;
		StringStream << "braid.SwapPosition(2,4)" << endl;
		StringStream << "briad.SwapPosition(2,5)" << endl;
		StringStream << "briad.SwapPosition(3,0)" << endl;
		StringStream << "braid.SwapPosition(3,1)" << endl;
		StringStream << "braid.SwapPosition(3,2)" << endl;
		StringStream << "briad.SwapPosition(4,0)" << endl;
		StringStream << "braid.SwapPosition(4,1)" << endl;
		StringStream << "braid.SwapPosition(4,2)" << endl;
		StringStream << "briad.SwapPosition(5,0)" << endl;
		StringStream << "braid.SwapPosition(5,1)" << endl;
		StringStream << "braid.SwapPosition(5,2)" << endl;
		
	}
	else
	{
		StringStream << "braid = CTextileBraid(" << iNumWeftYarns << ", " << iNumWarpYarns << ", " << dWidth << ", " << dHeight << ", " << dYarnSpacing << ", " << dFabricThickness << ", " << dBraidAngle * PI / 180.0 << ")" << endl;
	}
	int i, j;
	for (i = 0; i<iNumWarpYarns; ++i)
	{
		for (j = 0; j<iNumWeftYarns; ++j)
		{
			if (GetPatternCell(i, j))
			{
				StringStream << "braid.SwapPosition(" << i << ", " << j/*iHeight-(j+1)*/ << ")" << endl;
			}
		}
	}

	if (m_bCreateDomain)
	{
		StringStream << "braid.AssignDefaultDomain(False, True)" << endl;
	}
	StringStream << "AddTextile(braid)" << endl;
	return StringStream.str();
}

void CBraidWizard::LoadSettings(const CTextileBraid& Braid)
{
	m_YarnSpacing.clear();
	m_YarnWidth.Clear();
	m_FabricThickness.clear();
	m_BraidAngle.clear();
	m_pWeftYarnsSpin->SetValue(Braid.GetNumWeftYarns());
	m_pWarpYarnsSpin->SetValue(Braid.GetNumWarpYarns());
	m_YarnWidth << Braid.GetYarnWidth();
	m_YarnSpacing << Braid.GetYarnSpacing();

}
bool CBraidWizard::GetPatternCell(int i, int j)
{
	return !m_pBraidPatternCtrl->GetCellStatus(i, j);
}

void CBraidWizard::OnWizardPageChanging(wxWizardEvent& event)
{
	if (event.GetPage() == m_pFirstPage && event.GetDirection() == true)
	{
		RebuildBraidPatternCtrl();
		if (m_pBraidPatternDialog->ShowModal() != wxID_OK)
			event.Veto();
	}
}

bool CBraidWizard::RebuildBraidPatternCtrl()
{
	if (!m_pBraidPatternCtrl || !m_pWeftYarnsSpin || !m_pWarpYarnsSpin)
		return false;

	int iNumWidth = m_pWarpYarnsSpin->GetValue(), iNumHeight = m_pWeftYarnsSpin->GetValue();
	if (m_pBraidPatternCtrl->GetBraidWidth() != iNumWidth ||
		m_pBraidPatternCtrl->GetBraidHeight() != iNumHeight )
	{
		m_pBraidPatternCtrl->SetBraidSize(iNumWidth, iNumHeight, false);
		m_bWidthChanged = true;
		m_bSpacingChanged = true;
		m_bThicknessChanged = true;
	}

	CTextileBraid &braid = m_pBraidPatternCtrl->GetBraid();

	double dWidth = 1;
	double dSpacing = 1;
	double dThickness = 1;

	m_YarnWidth.ToDouble(&dWidth);
	m_YarnSpacing.ToDouble(&dSpacing);
	m_FabricThickness.ToDouble(&dThickness);

	if (m_bWidthChanged)
		braid.SetYarnWidths(dWidth);
	if (m_bSpacingChanged)
		braid.SetYarnSpacings(dSpacing);
	if (m_bThicknessChanged)
		braid.SetThickness(dThickness);

	m_bWidthChanged = false;
	m_bSpacingChanged = false;
	m_bThicknessChanged = false;

	return true;
}
