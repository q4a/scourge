#include "pageclasses.h"
#include "dfclasses.h"
#include <wx/wx.h>
#include "common.h"

PageClasses::PageClasses()
{
	skillEdits["Attributes"] = new SkillEdit;
	skillEdits["WeaponSkills"] = new SkillEdit;
	skillEdits["DefensiveSkills"] = new SkillEdit;
	skillEdits["MagicSkills"] = new SkillEdit;
	skillEdits["ThievingSkills"] = new SkillEdit;
}

PageClasses::~PageClasses()
{
	delete attributeStrArray;

	for ( std::map<std::string,SkillEdit*>::iterator itr = skillEdits.begin(); itr != skillEdits.end(); itr++ )
	{
		delete itr->second->strArray;
		delete itr->second;
	}
}

void PageClasses::Init(wxNotebook *notebook, DF *dataFile)
{
	dfClasses = (DFClasses*)dataFile;
	this->dataFile = dataFile;
	page = new wxNotebookPage(notebook, ID_ClassesPage);

	Class *c = dfClasses->GetCurrent();

	// name
	wxStaticText *nameText = new wxStaticText(page, -1, _("Name"), wxPoint(10,10));
	nameEdit = new wxTextCtrl(page, -1, std2wx(c->name), wxPoint(10,30), wxSize(150,25));

	// short name
	wxStaticText *shortNameText = new wxStaticText(page, -1, _("Short Name"), wxPoint(170,10));
	shortNameEdit = new wxTextCtrl(page, -1, std2wx(c->shortName), wxPoint(170,30), wxSize(50,25));

	// description
	wxStaticText *descText = new wxStaticText(page, -1, _("Description"), wxPoint(450,10));
	descEdit = new wxTextCtrl(page, -1, std2wx(c->description), wxPoint(450,30), wxSize(350,150), wxTE_MULTILINE);

	// Per level Bonuses
	wxStaticBox *bonusesBox = new wxStaticBox(page, -1, L"Per level bonuses", wxPoint(10,60),wxSize(160,80));
		// hp bonus
		wxStaticText *hpBonusText = new wxStaticText(page, -1, _("HP"), wxPoint(20,80));
		hpBonusEdit = new wxTextCtrl(page, -1, std2wx(c->hpBonus), wxPoint(20,100), wxSize(30,-1));
		// mp bonus
		wxStaticText *mpBonusText = new wxStaticText(page, -1, _("MP"), wxPoint(55,80));
		mpBonusEdit = new wxTextCtrl(page, -1, std2wx(c->mpBonus), wxPoint(55,100), wxSize(30,-1));
		// skill bonus
		wxStaticText *skillBonusText = new wxStaticText(page, -1, _("Skill"), wxPoint(90,80));
		skillBonusEdit = new wxTextCtrl(page, -1, std2wx(c->skillBonus), wxPoint(90,100), wxSize(30,-1));
		// attack bonus
		wxStaticText *attackBonusText = new wxStaticText(page, -1, _("Attack"), wxPoint(125,80));
		attackBonusEdit = new wxTextCtrl(page, -1, std2wx(c->attackBonus), wxPoint(125,100), wxSize(30,-1));
	// level progression
	wxStaticText *levelProgressionText = new wxStaticText(page, -1, _("Level Progression"), wxPoint(180,80));
	levelProgressionEdit = new wxTextCtrl(page, -1, std2wx(c->levelProgression), wxPoint(210,100), wxSize(30,-1));
	// additional attack level
	wxStaticText *additionalAttackLevelText = new wxStaticText(page, -1, _("Additional Attack Level"), wxPoint(300,80));
	additionalAttackLevelEdit = new wxTextCtrl(page, -1, std2wx(c->additionalAttackLevel), wxPoint(350,100), wxSize(30,-1));

	SkillEdit *skillEdit;
/**
	Attributes
**/
	wxStaticBox *attributeBox = new wxStaticBox(page, -1, L"Attributes", wxPoint(10,185),wxSize(140,140));
	skillEdit = skillEdits["Attributes"];
		wxString str[] = { L"SPEED", L"COORDINATION", L"POWER", L"IQ", L"LEADERSHIP", L"LUCK", L"PIETY", L"LORE" };
		skillEdit->strArray = new wxArrayString(8, str);
		skillEdit->list = new wxListBox(page, ID_ClassesSkillList, wxPoint(15,205), wxSize(130,80), *skillEdit->strArray);
			skillEdit->list->SetFont( wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
		// Min
		wxStaticText *attributeMinText = new wxStaticText(page, -1, _("Min:"), wxPoint(15,295));
		skillEdit->minEdit = new wxTextCtrl(page, -1, L"", wxPoint(40,290), wxSize(35,25));

		// Max
		wxStaticText *skillMaxText = new wxStaticText(page, -1, _("Max:"), wxPoint(80,295));
		skillEdit->maxEdit = new wxTextCtrl(page, -1, L"", wxPoint(110,290), wxSize(35,-1));

/**
	Weapon Skills
**/
	wxStaticBox *weaponSkillBox = new wxStaticBox(page, -1, L"Weapon Skills", wxPoint(160,185),wxSize(180,140));
	skillEdit = skillEdits["WeaponSkills"];
		wxString wStr[] = { L"SWORD_WEAPON", L"AXE_WEAPON", L"BOW_WEAPON", L"MACE_WEAPON", L"HAND_TO_HAND_COMBAT" };
		skillEdit->strArray = new wxArrayString(5, wStr);
		skillEdit->list = new wxListBox(page, ID_ClassesSkillList, wxPoint(165,205), wxSize(170,80), *skillEdit->strArray);
			skillEdit->list->SetFont( wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
		// Min
		wxStaticText *weaponSkillMinText = new wxStaticText(page, -1, _("Min:"), wxPoint(165,295));
		skillEdit->minEdit = new wxTextCtrl(page, -1, L"", wxPoint(195,290), wxSize(45,25));

		// Max
		wxStaticText *weaponSkillMaxText = new wxStaticText(page, -1, _("Max:"), wxPoint(255,295));
		skillEdit->maxEdit = new wxTextCtrl(page, -1, L"", wxPoint(285,290), wxSize(45,-1));

/**
	Defensive Skills
**/
	wxStaticBox *defensiveSkillBox = new wxStaticBox(page, -1, L"Defensive Skills", wxPoint(350,185),wxSize(140,140));
	skillEdit = skillEdits["DefensiveSkills"];
		wxString dStr[] = { L"SHIELD_DEFEND", L"ARMOR_DEFEND", L"WEAPON_DEFEND", L"HAND_DEFEND" };
		skillEdit->strArray = new wxArrayString(4, dStr);
		skillEdit->list = new wxListBox(page, ID_ClassesSkillList, wxPoint(355,205), wxSize(130,80), *skillEdit->strArray);
			skillEdit->list->SetFont( wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
		// Min
		wxStaticText *defensiveSkillMinText = new wxStaticText(page, -1, _("Min:"), wxPoint(355,295));
		skillEdit->minEdit = new wxTextCtrl(page, -1, L"", wxPoint(380,290), wxSize(35,25));

		// Max
		wxStaticText *defensiveSkillMaxText = new wxStaticText(page, -1, _("Max:"), wxPoint(420,295));
		skillEdit->maxEdit = new wxTextCtrl(page, -1, L"", wxPoint(450,290), wxSize(35,-1));

/**
	Magic Skills
**/
	wxStaticBox *magicSkillBox = new wxStaticBox(page, -1, L"Magic Skills", wxPoint(500,185),wxSize(175,140));
	skillEdit = skillEdits["MagicSkills"];
		wxString mStr [] = { L"NATURE_MAGIC", L"AWARENESS_MAGIC", L"LIFE_AND_DEATH_MAGIC", L"HISTORY_MAGIC", L"DECEIT_MAGIC", L"CONFRONTATION" };
		skillEdit->strArray = new wxArrayString(6, mStr);
		skillEdit->list = new wxListBox(page, ID_ClassesSkillList, wxPoint(505,205), wxSize(165,80), *skillEdit->strArray);
			skillEdit->list->SetFont( wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
		// Min
		wxStaticText *magicSkillMinText = new wxStaticText(page, -1, _("Min:"), wxPoint(505,295));
		skillEdit->minEdit = new wxTextCtrl(page, -1, L"", wxPoint(535,290), wxSize(35,25));

		// Max
		wxStaticText *magicSkillMaxText = new wxStaticText(page, -1, _("Max:"), wxPoint(575,295));
		skillEdit->maxEdit = new wxTextCtrl(page, -1, L"", wxPoint(605,290), wxSize(35,-1));

/**
	Thieving Skills
**/
	wxStaticBox *thievingSkillBox = new wxStaticBox(page, -1, L"Magic Skills", wxPoint(685,185),wxSize(140,140));
	skillEdit = skillEdits["ThievingSkills"];
		wxString tStr [] = { L"OPEN_LOCK", L"FIND_TRAP", L"MOVE_UNDETECTED", L"STEALING" };
		skillEdit->strArray = new wxArrayString(4, tStr);
		skillEdit->list = new wxListBox(page, ID_ClassesSkillList, wxPoint(690,205), wxSize(130,80), *skillEdit->strArray);
			skillEdit->list->SetFont( wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
		// Min
		wxStaticText *thievingSkillMinText = new wxStaticText(page, -1, _("Min:"), wxPoint(690,295));
		skillEdit->minEdit = new wxTextCtrl(page, -1, L"", wxPoint(715,290), wxSize(35,25));

		// Max
		wxStaticText *thievingSkillMaxText = new wxStaticText(page, -1, _("Max:"), wxPoint(755,295));
		skillEdit->maxEdit = new wxTextCtrl(page, -1, L"", wxPoint(785,290), wxSize(35,-1));

	notebook->AddPage(page, _("Classes"));
}

void PageClasses::UpdatePage()
{
}

void PageClasses::GetCurrent()
{
	Class *c = dfClasses->GetCurrent();

	nameEdit->SetValue( std2wx(c->name) );
	shortNameEdit->SetValue( std2wx(c->shortName) );
	descEdit->SetValue( std2wx(c->description) );

	hpBonusEdit->SetValue( std2wx(c->hpBonus) );
	mpBonusEdit->SetValue( std2wx(c->mpBonus) );
	skillBonusEdit->SetValue( std2wx(c->skillBonus) );
	attackBonusEdit->SetValue( std2wx(c->attackBonus) );

	levelProgressionEdit->SetValue( std2wx(c->levelProgression) );
	additionalAttackLevelEdit->SetValue( std2wx(c->additionalAttackLevel) );

	GetSkills();
}

void PageClasses::SetCurrent()
{
	Class *c = dfClasses->GetCurrent();

	c->name = wx2std( nameEdit->GetValue() );
	c->shortName = wx2std( shortNameEdit->GetValue() );
	c->description = wx2std( descEdit->GetValue() );

	c->hpBonus = wx2std( hpBonusEdit->GetValue() );
	c->mpBonus = wx2std( mpBonusEdit->GetValue() );
	c->skillBonus = wx2std( skillBonusEdit->GetValue() );
	c->attackBonus = wx2std( attackBonusEdit->GetValue() );

	c->levelProgression = wx2std( levelProgressionEdit->GetValue() );
	c->additionalAttackLevel = wx2std( additionalAttackLevelEdit->GetValue() );

	SetSkills();
}

void PageClasses::ClearCurrent()
{
	Class *c = dfClasses->GetCurrent();

	c->Clear();
}

void PageClasses::GetSkills()
{
	std::string str;
	str = "Attributes";			GetSkill(str);
	str = "WeaponSkills";		GetSkill(str);
	str = "DefensiveSkills";	GetSkill(str);
	str = "MagicSkills";		GetSkill(str);
	str = "ThievingSkills";		GetSkill(str);
}
void PageClasses::SetSkills()
{
	std::string str;
	str = "Attributes";			SetSkill(str);
	str = "WeaponSkills";		SetSkill(str);
	str = "DefensiveSkills";	SetSkill(str);
	str = "MagicSkills";		SetSkill(str);
	str = "ThievingSkills";		SetSkill(str);
}
void PageClasses::GetSkill(std::string &skillType)
{
	Skill *skill = GetSelectedSkill(skillType);
	if ( skill )
	{
		skillEdits[skillType]->minEdit->SetValue( std2wx(skill->min) );
		skillEdits[skillType]->maxEdit->SetValue( std2wx(skill->max) );
	}
}
void PageClasses::SetSkill(std::string &skillType)
{
	Skill *skill = currentSkills[skillType];
	if ( skill )
	{
		skill->min = wx2std( skillEdits[skillType]->minEdit->GetValue() );
		skill->max = wx2std( skillEdits[skillType]->maxEdit->GetValue() );
	}
}

Skill* PageClasses::GetSelectedSkill(std::string &skillType)
{
	PageClasses *pPage = ((PageClasses*)currentPage);		// wxWidgets event callback safety
	Class *c = pPage->dfClasses->GetCurrent();

	wxArrayInt selected;
	if ( pPage->skillEdits[skillType]->list->GetSelections(selected) == 0 )
		return 0;

	wxArrayString *pAttributeStrArray = pPage->skillEdits[skillType]->strArray;
	wxString str = (*pAttributeStrArray)[ (selected[0]) ];

	currentSkills[skillType] = c->skills[ wx2std(str) ];
	return currentSkills[skillType];
}

void PageClasses::OnSkillChange()
{
	PageClasses *pPage = ((PageClasses*)currentPage);

	pPage->SetSkills();
	pPage->GetSkills();
}
