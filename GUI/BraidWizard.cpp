#include "PrecompiledHeaders.h"
#include "BraidWizard.h"
#include "WindowIDs.h"
#include "PythonConverter.h"
#include "Wizard.xpm"
#include "RangeValidator.h"
#include "BraidPatternCtrl.h"

BEGIN_EVENT_TABLE(CBraidWizard, wxWizard)
	//EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CBraidWizard::OnWizardPageChanging)
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
	, m_YarnSpacing(wxT("1"))
	, m_YarnWidth(wxT("0.8"))
	, m_FabricThickness(wxT("0.2"))
	//, m_GapSize(wxT("0"))
	, m_BraidAngle(wxT("0.0"))
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
	//if (m_pBraidPatternDialog)
		//m_pBraidPatternDialog->Destroy(); 
}

bool CBraidWizard::RunIt()
{
	return RunWizard(m_pFirstPage);
}

void CBraidWizard::BuildPages()
{
	m_pFirstPage = BuildFirstPage();
	
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
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_Spacing, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_YarnSpacing)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the width of the yarns"));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Fabric Thickness:")), SizerFlags);
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_Spacing, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_YarnSpacing)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the thickness of the fabic"));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Braid Angle:")), SizerFlags);
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_Spacing, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_YarnSpacing)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the braid angle of the fabic"));

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

string CBraidWizard::getCreateTextileCommand(string ExistingTextile)
{
	stringstream StringStream;
	StringStream << "Braid" << endl;
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