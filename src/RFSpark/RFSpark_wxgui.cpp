/**
@file 	RFSpark_wxgui.cpp
@author Lime Microsystems
@brief 	wxWidgets panel for interacting with RFSpark board
*/

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>

#include "RFSpark_wxgui.h"
#include <vector>

const long RFSpark_wxgui::ID_BTNREADADC = wxNewId();
const long RFSpark_wxgui::ID_BTNREADALLADC = wxNewId();
const long RFSpark_wxgui::ID_BTNWRITEGPIO = wxNewId();
const long RFSpark_wxgui::ID_BTNREADGPIO = wxNewId();
const long RFSpark_wxgui::ID_CMBSELECTADC = wxNewId();

BEGIN_EVENT_TABLE(RFSpark_wxgui,wxFrame)

END_EVENT_TABLE()

wxString power2unitsString(char powerx3)
{
	switch (powerx3)
	{
	case -8:
		return "y";
	case -7:
		return "z";
	case -6:
		return "a";
	case -5:
		return "f";
	case -4:
		return "p";
	case -3:
		return "n";
	case -2:
		return "u";
	case -1:
		return "m";
	case 0:
		return "";
	case 1:
		return "k";
	case 2:
		return "M";
	case 3:
		return "G";
	case 4:
		return "T";
	case 5:
		return "P";
	case 6:
		return "E";
	case 7:
		return "Y";
	default:
		return "";
	}
}

RFSpark_wxgui::RFSpark_wxgui(wxWindow* parent,wxWindowID id, const wxString& title, const wxPoint& pos,const wxSize& size, long style)
{
    lmsControl = nullptr;
    Create(parent, id, title, wxDefaultPosition, wxDefaultSize, style, title);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    SetIcon(wxIcon(_("aaaaAPPicon")));
#endif

    //ADC values
    wxFlexGridSizer* sizerADCs = new wxFlexGridSizer(0, 3, 0, 10);

    ADCdataGUI adcElement;
    sizerADCs->Add(new wxStaticText(this, wxNewId(), _("Channel")), 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerADCs->Add(new wxStaticText(this, wxNewId(), _("Value")), 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerADCs->Add(new wxStaticText(this, wxNewId(), _("Units")), 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    wxArrayString adcList;
    wxString adcNames[] = { "VDD12_TIA_RFE", "VDD_TBB", "VDD14_RBB", "DVDD_SXT", "VDD_DIC_SXT", "VDD12_TXBUF", "VDD12_VCO_SXT", "VDD14_LNA_RFE",
        "DVDD_SXR", "VDD_DIV_SXR", "VDD_CP_SXR", "VDD18_SXR", "VD_MXLOBUF_RFE", "VDD14_TIA_RFE", "VDD12_LNA_RFE", "VDD_TLOBUF_TRF",
        "VDD12_RXBIF", "VDD_AFE", "DIGPRVDD1", "VDD_SPI_BUF", "VDD18_TPAD_TRF", "DVDD_CGEN", "VDD_DIV_CGEN", "VDD_CP_CGEN", "VDD12_DIG", "VDD14_VCO_CGEN",
        "TSTAO", "TSTDO 0", "TSTDO 1", "3V3", "VDIO", "VDIO_FMC" };
    for (int i = 0; i < mADCcount; ++i)
    {
        adcElement.title = new wxStaticText(this, wxNewId(), wxString::Format("%s", adcNames[i]));
        adcList.push_back(adcElement.title->GetLabel());
        adcElement.value = new wxStaticText(this, wxNewId(), _("????"));
        adcElement.units = new wxStaticText(this, wxNewId(), _("????"));
        sizerADCs->Add(adcElement.title, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
        sizerADCs->Add(adcElement.value, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
        sizerADCs->Add(adcElement.units, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
        mADCgui.push_back(adcElement);
        ADCdata data;
        data.channel = i;
        //data.powerOf10coefficient = 0;
        //data.units = 0;
        data.value = 0;
        mADCdata.push_back(data);
    }

    btnReadAllADC = new wxButton(this, ID_BTNREADALLADC, "Refresh All");
    btnReadADC = new wxButton(this, ID_BTNREADADC, "Refresh");
    cmbADCselect = new wxComboBox(this, wxNewId(), adcList[0], wxDefaultPosition, wxDefaultSize, adcList);
    cmbADCselect->SetSelection(0);
    wxFlexGridSizer* sizerADCbuttons = new wxFlexGridSizer(0, 3, 5, 5);
    sizerADCbuttons->Add(btnReadAllADC, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerADCbuttons->Add(btnReadADC, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerADCbuttons->Add(cmbADCselect, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

    wxFlexGridSizer* sizerBoxADC = new wxFlexGridSizer(0, 1, 0, 0);
    sizerBoxADC->Add(sizerADCs, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerBoxADC->Add(sizerADCbuttons, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_TOP, 0);

    wxStaticBoxSizer* boxADC = new wxStaticBoxSizer(wxVERTICAL, this, _T("ADC values"));
    boxADC->Add(sizerBoxADC, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

    wxFlexGridSizer* sizerGPIOs = new wxFlexGridSizer(0, 8, 10, 5);

    wxString gpios7_0[] = { "ADCinQ2_N 15", "ADCinQ2_P", "ADCinI2_N", "ADCinI2_P", "ADCinQ1_N", "ADCinQ1_P", "ADCinI1_N", "ADCinI1_P" };
    for (int j = 0; j < 8; ++j)
    {
        long id = wxNewId();
        wxCheckBox* chkgpio = new wxCheckBox(this, id, wxString::Format("%s", gpios7_0[j]));
        this->Connect(id, wxEVT_CHECKBOX, (wxObjectEventFunction)&RFSpark_wxgui::OnbtnWriteGPIO);
        sizerGPIOs->Add(chkgpio, wxALIGN_LEFT | wxALIGN_TOP, 0);
        mGPIOboxes.push_back(chkgpio);
    }

    wxString gpios15_8[] = { "GPIO 15", "DIG_RST", "CORE_LDO_EN", "IQSELEN2RX_DIR", "IQSELEN1TX_DIR", "DIO_BUF_OE", "DIQ2RX_DIR", "DIQ1TX_DIR" };
    for (int j = 0; j < 8; ++j)
    {
        long id = wxNewId();
        wxCheckBox* chkgpio = new wxCheckBox(this, id, wxString::Format("%s", gpios15_8[j]));
        this->Connect(id, wxEVT_CHECKBOX, (wxObjectEventFunction)&RFSpark_wxgui::OnbtnWriteGPIO);
        sizerGPIOs->Add(chkgpio, wxALIGN_LEFT | wxALIGN_TOP, 0);
        mGPIOboxes.push_back(chkgpio);
    }
    wxString gpios23_16[] = { "GPIO 23", "GPIO 22", "GPIO 21", "GPIO 20", "GPIO 19", "GPIO 18", "TXEN_LMS", "RXEN_LMS" };
    for (int j = 0; j < 8; ++j)
    {
        long id = wxNewId();
        wxCheckBox* chkgpio = new wxCheckBox(this, id, wxString::Format("%s", gpios23_16[j]));
        this->Connect(id, wxEVT_CHECKBOX, (wxObjectEventFunction)&RFSpark_wxgui::OnbtnWriteGPIO);
        sizerGPIOs->Add(chkgpio, wxALIGN_LEFT | wxALIGN_TOP, 0);
        mGPIOboxes.push_back(chkgpio);
    }

    wxButton* btnReadGPIO = new wxButton(this, ID_BTNREADGPIO, "Read");
    wxButton* btnWriteGPIO = new wxButton(this, ID_BTNWRITEGPIO, "Write");
    wxFlexGridSizer* sizerGPIObuttons = new wxFlexGridSizer(0, 2, 0, 5);
    sizerGPIObuttons->Add(btnReadGPIO, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerGPIObuttons->Add(btnWriteGPIO, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

    wxFlexGridSizer* sizerBoxGPIO = new wxFlexGridSizer(0, 1, 5, 5);
    sizerBoxGPIO->Add(sizerGPIOs, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerBoxGPIO->Add(sizerGPIObuttons, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

    wxStaticBoxSizer* boxGPIO = new wxStaticBoxSizer(wxVERTICAL, this, _T("GPIO states"));
    boxGPIO->Add(sizerBoxGPIO, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

    wxFlexGridSizer* mainGrid = new wxFlexGridSizer(0, 2, 5, 5);
    mainGrid->Add(boxADC, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    mainGrid->Add(boxGPIO, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

    SetSizer(mainGrid);
    mainGrid->Fit(this);
    mainGrid->SetSizeHints(this);
    Layout();

    Connect(ID_BTNREADALLADC, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RFSpark_wxgui::OnbtnRefreshAllADC);
    Connect(ID_BTNREADADC, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RFSpark_wxgui::OnbtnRefreshADC);
    Connect(ID_BTNWRITEGPIO, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RFSpark_wxgui::OnbtnWriteGPIO);
    Connect(ID_BTNREADGPIO, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RFSpark_wxgui::OnbtnReadGPIO);
}

void RFSpark_wxgui::Initialize(lms_device_t* pSerPort)
{
    lmsControl = pSerPort;
    wxCommandEvent evt;
    OnbtnReadGPIO(evt);
    OnbtnRefreshAllADC(evt);
}

RFSpark_wxgui::~RFSpark_wxgui()
{

}

void RFSpark_wxgui::UpdateADClabels()
{
	for (unsigned i = 0; i < mADCdata.size(); ++i)
	{
        mADCgui[i].value->SetLabelText(wxString::Format("%f", mADCdata[i].value));
        mADCgui[i].units->SetLabelText(wxString::Format("%s", mADCdata[i].units));
    }
}

void RFSpark_wxgui::OnbtnRefreshAllADC(wxCommandEvent& event)
{
    for (size_t i = 0; i < mADCdata.size(); ++i)
    {
        float_type val;
        lms_name_t units;
        if (LMS_ReadCustomBoardParam(lmsControl,i,&val,units)!=0)
        {
            wxMessageBox(_("Board response: ") + wxString::From8BitData(LMS_GetLastErrorMessage()), _("Warning"));
            return;
        }
        mADCdata[i].channel = i;
        mADCdata[i].units = units;
        mADCdata[i].value = val;
    }
    UpdateADClabels();
}

void RFSpark_wxgui::OnbtnRefreshADC(wxCommandEvent& event)
{
    unsigned int index = cmbADCselect->GetSelection();
    if(index >= mADCdata.size())
        return;

    float_type val;
    lms_name_t units;
    if (LMS_ReadCustomBoardParam(lmsControl,index,&val,units)!=0)
    {
        wxMessageBox(_("Board response: ") + wxString::From8BitData(LMS_GetLastErrorMessage()), _("Warning"));
        return;
    }
    mADCdata[index].channel = index;
    mADCdata[index].units = units;
    mADCdata[index].value = val;
    UpdateADClabels();
}

void RFSpark_wxgui::OnbtnWriteGPIO(wxCommandEvent& event)
{
    std::vector<uint8_t> values(mGPIOboxes.size()/8, 0);
    int gpioIndex = 0;
    for (size_t i = 0; i < mGPIOboxes.size()/8; ++i)
    {
        unsigned char value = 0;
        for (int j = 7; j >= 0; --j)
            value |= mGPIOboxes[gpioIndex++]->IsChecked() << j;
        values[i] = value;
    }
    if (LMS_GPIOWrite(lmsControl, values.data(), mGPIOboxes.size()/8) != 0)
        wxMessageBox(_("Board response: ") + wxString::From8BitData(LMS_GetLastErrorMessage()), _("Warning"));
}

void RFSpark_wxgui::OnbtnReadGPIO(wxCommandEvent& event)
{
    std::vector<uint8_t> values(mGPIOboxes.size()/8);
    if (LMS_GPIORead(lmsControl, values.data(), mGPIOboxes.size()/8) != 0)
    {
        wxMessageBox(_("Board response: ") + wxString::From8BitData(LMS_GetLastErrorMessage()), _("Warning"));
        return;
    }
    size_t gpioIndex = 0;
    int gpioByte = 0;
    for (size_t i = 0; i < mGPIOboxes.size() / 8; ++i)
    {
        for (int j = 7; j >= 0 && gpioIndex < mGPIOboxes.size(); --j)
            mGPIOboxes[gpioIndex++]->SetValue( values[gpioByte] & (0x1 << j) );
        ++gpioByte;
    }
}
