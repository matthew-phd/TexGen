#include "PrecompiledHeaders.h"
#include "BraidWizard.h"
#include "WindowIDs.h"
#include "PythonConverter.h"
#include "Wizard.xpm"
#include "RangeValidator.h"
#include "BraidPatternCtrl.h"
#include "BraiderImage.xpm"


BEGIN_EVENT_TABLE(CBraidWizard, wxWizard)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CBraidWizard::OnWizardPageChanging)
	EVT_CHECKBOX(ID_Curved, CBraidWizard::OnCurved)
	EVT_CHECKBOX(ID_CreateLayers, CBraidWizard::OnLayers)
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
	, m_YarnWidth(wxT("3.5"))
	, m_FabricThickness(wxT("0.7"))
	, m_BraidAngle(wxT("55"))
	, m_Radius(wxT("16"))
	, m_HornGearVelocity(wxT("20"))
	, m_pHornGearSpin(NULL)
	, m_Velocity(wxT("3"))
	, m_bCreateDomain(true)
	, m_bRefine(false)
	, m_bWidthChanged(false)
	, m_bSpacingChanged(false)
	, m_bThicknessChanged(false)
	, m_bCurved(false)
	, m_bAdjustSpacing(false)
	, m_pLayersSpin(NULL)
	, m_bLayers(false)
	//, m_bAddedDomainHeight(true)
{
	BuildPages();
	GetPageAreaSizer()->Add(m_pFirstPage);
}


CBraidWizard::~CBraidWizard(void)
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
	wxWizardPageSimple *pPage = new wxWizardPageSimple(this, NULL, NULL, wxBitmap(BraiderImage_xpm));

	wxBoxSizer *pMainSizer = new wxBoxSizer(wxVERTICAL);
	wxSizerFlags SizerFlags(0);
	SizerFlags.Border();
	SizerFlags.Expand();

	wxSizer *pSubSizer;

	pMainSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("This wizard will create a biaxial braid model for you.")), SizerFlags);

	//pMainSizer->Add(new wxStaticBitmap(pPage, wxID_STATIC, wxBitmap(BraiderImage_xpm) ), SizerFlags );

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
		//pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Yarn Spacing:")), SizerFlags);
		//pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_Spacing, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_YarnSpacing)), SizerFlags);
		//pControl->SetToolTip(wxT("Sets the spacing between yarns"));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Yarn Width:")), SizerFlags);
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_Width, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_YarnWidth)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the width of the yarns"));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Fabric Thickness:")), SizerFlags);
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_Thickness, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_FabricThickness)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the thickness of the fabic"));

		//pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Braid Angle:")), SizerFlags);
		//pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_BraidAngle, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, RangeValidator( &m_BraidAngle, 0, 89)), SizerFlags);
		//pControl->SetToolTip(wxT("Sets the braid angle of the fabic"));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Number of Horn Gears: ")), SizerFlags);
		pSubSizer->Add(m_pHornGearSpin = new wxSpinCtrl(pPage, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 3000), SizerFlags);
		m_pHornGearSpin->SetToolTip(wxT("Controls the number of Horn gears on braider used to make fabric"));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Radius of Mandrel (in mm): ")), SizerFlags);
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_Radius, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_Radius)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the radius of Mandrel."));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Horn Gear Velocity (in RPM): ")), SizerFlags);
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_HornGearVelocity, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_HornGearVelocity)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the horn gear velocity in rpm"));

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Take up Velocity (in mm/s): ")), SizerFlags);
		pSubSizer->Add(pControl = new wxTextCtrl(pPage, ID_Velocity, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC, &m_Velocity)), SizerFlags);
		pControl->SetToolTip(wxT("Sets the mandrel take up velocity in mm/s"));
		
		const wxString choices[3] = { "Diamond", "Regular", "Hercules" };
		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("Braid Pattern: ")), SizerFlags);
		pSubSizer->Add(pBraidPattern = new wxChoice(pPage, ID_BraidPattern, wxDefaultPosition, wxDefaultSize, 3, choices,0, wxDefaultValidator), SizerFlags);
		wxCheckBox* pDomainBox;
		wxCheckBox* pRefineBox;
		wxCheckBox* pCurvedBox;
		wxCheckBox* pAdjustSpacingBox;
		wxCheckBox* pLayersBox;
		pSubSizer->Add(pDomainBox = new wxCheckBox(pPage, ID_DefaultDomain, wxT("Create Default Domain"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&m_bCreateDomain)), SizerFlags);
		pSubSizer->Add(pRefineBox = new wxCheckBox(pPage, ID_Refine, wxT("Refine model"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&m_bRefine)), SizerFlags);
		pSubSizer->Add(pCurvedBox = new wxCheckBox(pPage, ID_Curved, wxT("Curved Unit Cell"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&m_bCurved)), SizerFlags);
		pSubSizer->Add(pAdjustSpacingBox = new wxCheckBox(pPage, ID_AdjustSpacing, wxT("Adjust Yarn Spacing"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&m_bAdjustSpacing)), SizerFlags);
		if (m_bCurved)
			pAdjustSpacingBox->Disable();
		

		pSubSizer->Add(pLayersBox = new wxCheckBox(pPage, ID_CreateLayers, wxT("Layers"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&m_bLayers)), SizerFlags);
		if (!m_bCurved)
			pLayersBox->Disable();

		pSubSizer->Add(new wxStaticText(pPage, wxID_ANY, wxT("layers:")), SizerFlags);
		pSubSizer->Add(m_pLayersSpin = new wxSpinCtrl(pPage, ID_NumLayers, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100), SizerFlags);
		m_pWeftYarnsSpin->SetToolTip(wxT("Controls the number of layers of fabric in the model"));
		if (!m_bLayers)
			m_pLayersSpin->Disable();


	}
	pMainSizer->Add(pSubSizer, SizerFlags);
	SizerFlags.Align(0);

	pPage->SetSizer(pMainSizer);
	pMainSizer->Fit(pPage);

	m_pWeftYarnsSpin->SetValue(4);
	m_pWarpYarnsSpin->SetValue(4);
	m_pHornGearSpin->SetValue(24);
	m_pLayersSpin->SetValue(1);


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

string CBraidWizard::GetCreateTextileCommand(string ExistingTextile)
{
	stringstream StringStream;
	double dFabricThickness, dWidth, dHeight, dRadius, dHornGearVelocity, dVelocity;
	int iNumWeftYarns, iNumWarpYarns, iNumHornGear, iNumLayers;
	bool bRefine, bCurved, bAdjustSpacing;
	string braidPattern;
	//m_YarnSpacing.ToDouble(&dYarnSpacing);
	m_YarnWidth.ToDouble(&dWidth);
	m_FabricThickness.ToDouble(&dFabricThickness);
	m_Radius.ToDouble(&dRadius);
	m_HornGearVelocity.ToDouble(&dHornGearVelocity);
	m_Velocity.ToDouble(&dVelocity);
	//m_BraidAngle.ToDouble(&dBraidAngle);
	dHeight = dFabricThickness / 2;
	iNumWeftYarns = m_pWeftYarnsSpin->GetValue();
	iNumWarpYarns = m_pWarpYarnsSpin->GetValue();
	iNumHornGear = m_pHornGearSpin->GetValue();
	braidPattern = pBraidPattern->GetString(pBraidPattern->GetSelection());
	iNumLayers = m_pLayersSpin->GetValue();

	if (m_bCurved)
	{
		StringStream<< "braid = CTextileBraidCurved(" << iNumWeftYarns << ", " << iNumWarpYarns << ", " << dWidth << ", " << dHeight << ", " << dFabricThickness << ", "
			<< dRadius / 1000 << ", " << dHornGearVelocity * ((2 * PI) / 60) << ", " << iNumHornGear << ", " << dVelocity / 1000 << ", bool(" << m_bCurved << ")" << ", bool(" << m_bRefine << "), " << iNumLayers<< ")" << endl;
	}
	else
	{
		StringStream << "braid = CTextileBraid(" << iNumWeftYarns << ", " << iNumWarpYarns << ", " << dWidth << ", " << dHeight << ", " << dFabricThickness << ", "
			<< dRadius / 1000 << ", " << dHornGearVelocity * ((2 * PI) / 60) << ", " << iNumHornGear << ", " << dVelocity / 1000 << ", bool(" << m_bRefine << ")" << ", bool(" << m_bAdjustSpacing << "))" << endl;
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
		if (m_bCurved)
		{
			StringStream << "braid.AssignDefaultDomain(True)" << endl;
		}
		else
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

void CBraidWizard::OnCurved(wxCommandEvent& event)
{
	RefreshGapTextBox();
}

void CBraidWizard::OnLayers(wxCommandEvent& event)
{
	RefreshGapTextBox();
}

void CBraidWizard::RefreshGapTextBox()
{
	wxCheckBox* pCurved = (wxCheckBox*)FindWindowById(ID_Curved, this);
	wxCheckBox* pAdjustSpacing = (wxCheckBox*)FindWindowById(ID_AdjustSpacing, this);
	wxCheckBox* pLayer = (wxCheckBox*)FindWindowById(ID_CreateLayers, this);
	wxSpinCtrl* pNumLayers = (wxSpinCtrl*)FindWindowById(ID_NumLayers, this);

	if (pCurved && pAdjustSpacing && pLayer && pNumLayers)
	{
		if (pCurved->GetValue())
		{
			pAdjustSpacing->Disable();
			pLayer->Enable();
			if (pLayer->GetValue())
			{
				pNumLayers->Enable();
			}
			else
				pNumLayers->Disable();
		}
		else
		{
			pAdjustSpacing->Enable();
			pLayer->Disable();
		}
	}
	
}

