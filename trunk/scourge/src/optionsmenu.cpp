/***************************************************************************
                    optionsmenu.cpp  -  The options menu
                             -------------------
    begin                : Tue Aug 12 2003
    copyright            : (C) 2003 by Gabor Torok
    email                : cctorok@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "common/constants.h"
#include "optionsmenu.h"
#include "sound.h"
#include "shapepalette.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

#define XPOS 10
#define SPACING 18
#define MINOR_SPACING 4
#define YPOS ( SPACING + MINOR_SPACING )
#define WIN_WIDTH 340
#define X_SIZE WIN_WIDTH - 40
#define BUTTON_WIDTH ( WIN_WIDTH - 16 ) / 4

OptionsMenu::OptionsMenu( Scourge *scourge ) {
	this->scourge = scourge;
	this->uc = scourge->getUserConfiguration();
	controlsLoaded = false;
	videoLoaded = false;
	gameSettingsLoaded = false;
	waitingForNewKey = false;
	ignoreKeyUp = false;

	mainWin = new Window( scourge->getSDLHandler(),
	                      100, 170, WIN_WIDTH, 340,
	                      _( "Options" ),
	                      scourge->getShapePalette()->getGuiTexture(),
	                      true, Window::BASIC_WINDOW,
	                      scourge->getShapePalette()->getGuiTexture2() );

	int x = 8;
	gameSettingsButton = mainWin->createButton( x, 0, x + BUTTON_WIDTH, SPACING, _( "Gameplay" ), true );
	x += BUTTON_WIDTH;
	videoButton = mainWin->createButton ( x, 0, x + BUTTON_WIDTH, SPACING, _( "Video" ), true );
	x += BUTTON_WIDTH;
	audioButton = mainWin->createButton ( x, 0, x + BUTTON_WIDTH, SPACING, _( "Audio" ), true );
	x += BUTTON_WIDTH;
	controlsButton = mainWin->createButton ( x, 0, x + BUTTON_WIDTH, SPACING, _( "Controls" ), true );

	x = 10;
	saveButton = mainWin->createButton( x, 285, x + BUTTON_WIDTH, 285 + SPACING, _( "Save" ), false );
	x += BUTTON_WIDTH + MINOR_SPACING;
	closeButton = mainWin->createButton( x, 285, x + BUTTON_WIDTH, 285 + SPACING, _( "Close" ), false );

	cards = new CardContainer( mainWin );

	// Controls tab
	int y = YPOS;
	keyBindingsLabel = cards->createLabel( XPOS, y + 10, _( "Key bindings" ), CONTROLS, Constants::RED_COLOR );
	y += SPACING + MINOR_SPACING;
	controlBindingsList = new ScrollingList( XPOS, y, X_SIZE, 190, scourge->getShapePalette()->getHighlightTexture() );
	cards->addWidget( controlBindingsList, CONTROLS );
	y += 190 + MINOR_SPACING;
	changeControlButton = cards->createButton( XPOS, y, XPOS + X_SIZE, y + SPACING,
	                      Constants::getMessage( Constants::CHANGE_KEY ),
	                      CONTROLS, false );
	y += SPACING + MINOR_SPACING;
	waitingLabel = cards->createLabel( 35, 80, " ", CONTROLS, Constants::BLUE_COLOR );

	// Game settings tab
	y = YPOS;
	gameSpeedML = new MultipleLabel( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Game speed" ), 100 );
	gameSpeedML -> addText( _( "Very slow" ) );
	gameSpeedML -> addText( _( "Slow" ) );
	gameSpeedML -> addText( _( "Normal" ) );
	gameSpeedML -> addText( _( "Fast" ) );
	gameSpeedML -> addText( _( "Fastest" ) );
	cards->addWidget( gameSpeedML, GAME_SETTINGS );
	y += SPACING + MINOR_SPACING;
	alwaysCenterMapCheckbox = cards->createCheckbox( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Always center map" ), GAME_SETTINGS );
	y += SPACING + MINOR_SPACING;
	turnBasedBattle = cards->createCheckbox( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Is battle turn-based?" ), GAME_SETTINGS );
	y += SPACING + MINOR_SPACING;
	outlineInteractiveItems = cards->createCheckbox( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Outline items?" ), GAME_SETTINGS );
	y += SPACING + MINOR_SPACING;
	tooltipEnabled = cards->createCheckbox( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Show tooltips" ), GAME_SETTINGS );
	y += SPACING + MINOR_SPACING;
	tooltipInterval = new Slider( XPOS, y, XPOS + X_SIZE, scourge->getShapePalette()->getHighlightTexture(), 0, 200, _( "Tooltip Delay:" ) );
	cards->addWidget( tooltipInterval, GAME_SETTINGS );
	y += SPACING + SPACING;
	logLevelML = new MultipleLabel( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Log level" ), 100 );
	logLevelML->addText( _( "Minimal" ) );
	logLevelML->addText( _( "Partial" ) );
	logLevelML->addText( _( "Verbose" ) );
	logLevelML->addText( _( "Full" ) );
	cards->addWidget( logLevelML, GAME_SETTINGS );

	y += SPACING + MINOR_SPACING;
	pathFindingQualityML = new MultipleLabel( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Path finding quality" ), 100 );
	pathFindingQualityML->addText( _( "Basic" ) );
	pathFindingQualityML->addText( _( "Advanced" ) );
	pathFindingQualityML->addText( _( "Excellent" ) );
	cards->addWidget( pathFindingQualityML, GAME_SETTINGS );

	// Video settings tabs
	y = YPOS;
	videoResolutionML = new MultipleLabel( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Screen resolution" ), 100 );
	int nbModes, i;
	nbModes = scourge->getSDLHandler()->getVideoModeCount();
	for ( i = 0; i < nbModes; i++ ) {
		std::string s = scourge->getSDLHandler()->getVideoMode( i );
		videoResolutionML->addText( s.c_str() );
	}
	cards->addWidget( videoResolutionML, VIDEO );

	y += SPACING + MINOR_SPACING;
	fullscreenCheckbox = cards->createCheckbox( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Fullscreen" ), VIDEO );
	y += SPACING + MINOR_SPACING;
	doublebufCheckbox  = cards->createCheckbox( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Use double buffering" ), VIDEO );
	y += SPACING + MINOR_SPACING;
	stencilbufCheckbox = cards->createCheckbox( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Use stencil buffer" ), VIDEO );
	y += SPACING + MINOR_SPACING;
	multitexturingCheckbox = cards->createCheckbox( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Use multitexturing" ), VIDEO );
	y += SPACING + MINOR_SPACING;
	hwaccelCheckbox = cards->createCheckbox( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Use hardware acceleration" ), VIDEO );
	y += SPACING + MINOR_SPACING;
	anisofilterCheckbox = cards->createCheckbox( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Use anisotropic filtering" ), VIDEO );
	y += SPACING + MINOR_SPACING;
	shadowsML = new MultipleLabel( XPOS, y, XPOS + X_SIZE, y + SPACING, _( "Shadows" ), 100 );
	shadowsML -> addText( _( "None" ) );
	shadowsML -> addText( _( "Some" ) );
	shadowsML -> addText( _( "All" ) );
	cards->addWidget( shadowsML, VIDEO );
	y += SPACING + MINOR_SPACING;
	changeTakeEffectLabel = cards->createLabel( XPOS, y, " ", VIDEO, Constants::BLUE_COLOR );

	// audio
	y = YPOS;
	musicVolume = new Slider( XPOS, y, XPOS + X_SIZE, scourge->getShapePalette()->getHighlightTexture(), 0, 128, _( "Music Volume:" ) );
	cards->addWidget( musicVolume, AUDIO );
	y += SPACING + MINOR_SPACING + 15;
	effectsVolume = new Slider( XPOS, y, XPOS + X_SIZE, scourge->getShapePalette()->getHighlightTexture(), 0, 128, _( "Effects Volume:" ) );
	cards->addWidget( effectsVolume, AUDIO );

	selectedMode = GAME_SETTINGS;
	cards->setActiveCard( GAME_SETTINGS );
	
	// register this class as the event handler for ever widget in mainWin
	mainWin->registerEventHandler( this );
	mainWin->setRawEventHandler( this );
}

void OptionsMenu::loadGameSettings() {

	gameSpeedML->setText( gameSpeedML->getNbText() - uc->getGameSpeedLevel() - 1 );
	alwaysCenterMapCheckbox->setCheck( uc->getAlwaysCenterMap() );
	turnBasedBattle->setCheck( uc->isBattleTurnBased() );
	outlineInteractiveItems->setCheck( uc->isOutlineInteractiveItems() );
	musicVolume->setValue( scourge->getUserConfiguration()->getMusicVolume() );
	effectsVolume->setValue( scourge->getUserConfiguration()->getEffectsVolume() );
	tooltipEnabled->setCheck( uc->getTooltipEnabled() );
	tooltipInterval->setValue( scourge->getUserConfiguration()->getTooltipInterval() );
	logLevelML->setText( uc->getLogLevel() );
	pathFindingQualityML->setText( uc->getPathFindingQuality() );
}

// line i must correspond to engine action i if we want this scrolling list to work
void OptionsMenu::loadControls() {
	for ( int i = 0; i < ENGINE_ACTION_COUNT; i++ ) {
		controlBindingsList->setLine( uc->getEngineActionDescription( i ) + string( "           " ) + uc->getEngineActionKeyName( i ) );
	}
}

void OptionsMenu::loadVideo() {
	string line;
	string s, s1, s2, s3, s4;
	int i;
	int end;

	char temp[50];
	snprintf( temp, 50, "%d x %d", uc -> getW(), uc-> getH() );
	s = temp;
	s1 = Util::getNextWord( s, 0, end );
	s2 = Util::getNextWord( s, end, end );  // ignores the ' x '
	s2 = Util::getNextWord( s, end, end );
	i = 0;

	// don't know why the string::find() function does not work, so...
	while ( i < videoResolutionML -> getNbText() ) {
		line = videoResolutionML -> getText( i );
		s3 = Util::getNextWord( line, 0, end );
		if ( s1 == s3 ) {
			s4 = Util::getNextWord( line, end, end ); // ignores the ' x '
			s4 = Util::getNextWord( line, end, end );
			if ( s2 == s4 ) {
				break;
			}
		}
		i++;
	}
	if ( i < videoResolutionML -> getNbText() ) {
		videoResolutionML -> setText( i );
	} else {
		videoResolutionML -> setText( 0 );
	}
	shadowsML->setText( uc->getShadows() );

	// Checkboxes
	fullscreenCheckbox->setCheck( uc->getFullscreen() );
	doublebufCheckbox->setCheck( uc->getDoublebuf() );
	stencilbufCheckbox->setCheck( uc->getStencilbuf() );
	multitexturingCheckbox->setCheck( uc->getMultitexturing() );
	hwaccelCheckbox->setCheck( uc->getHwaccel() );
	anisofilterCheckbox->setCheck( uc->getAnisoFilter() );
}

void OptionsMenu::setSelectedMode() {

	videoButton->setSelected( selectedMode == VIDEO );
	audioButton->setSelected( selectedMode == AUDIO );
	controlsButton->setSelected( selectedMode == CONTROLS );
	gameSettingsButton->setSelected( selectedMode == GAME_SETTINGS );

	switch ( selectedMode ) {
	case VIDEO :
		if ( !videoLoaded ) {
			loadVideo();
			videoLoaded = true;
		}
		break;
	case AUDIO :
		break;
	case CONTROLS :
		if ( !controlsLoaded ) {
			loadControls();
			controlsLoaded = true;
		}
		break;
	case GAME_SETTINGS :
		if ( !gameSettingsLoaded ) {
			loadGameSettings();
			gameSettingsLoaded = true;
		}
		break;
	default :
		break;
	}

	cards->setActiveCard( selectedMode );
}


bool OptionsMenu::handleEvent( Widget *widget, SDL_Event *event ) {
	if ( widget == mainWin->closeButton ) {
		scourge->toggleOptionsWindow();
		return true;
	} else if ( widget == gameSettingsButton ) {
		selectedMode = GAME_SETTINGS;
	} else if ( widget == videoButton ) {
		selectedMode = VIDEO;
	} else if ( widget == audioButton ) {
		selectedMode = AUDIO;
	} else if ( widget == controlsButton ) {
		selectedMode = CONTROLS;
	} else if ( widget == changeControlButton ) {
		changeControlButton->setLabel( Constants::getMessage( Constants::WAITING_FOR_KEY ) );
		waitingForNewKey = true;
	} else if ( widget == gameSpeedML ) {
		uc -> setGameSpeedLevel( gameSpeedML->getNbText() - gameSpeedML->getCurrentTextInd() - 1 );
	} else if ( widget == alwaysCenterMapCheckbox ) {
		uc -> setAlwaysCenterMap( alwaysCenterMapCheckbox->isChecked() );
	} else if ( widget == turnBasedBattle ) {
		uc ->setBattleTurnBased( turnBasedBattle->isChecked() );
		scourge->getTBCombatWin()->setVisible( scourge->inTurnBasedCombat(), false );
	} else if ( widget == outlineInteractiveItems ) {
		uc ->setOutlineInteractiveItems( outlineInteractiveItems->isChecked() );
	} else if ( widget == tooltipEnabled ) {
		uc ->setTooltipEnabled( tooltipEnabled->isChecked() );
		if ( !( uc ->getTooltipEnabled() ) ) scourge->resetInfos();
	} else if ( widget == tooltipInterval ) {
		uc->setTooltipInterval( tooltipInterval->getValue() );
	} else if ( widget == logLevelML ) {
		uc->setLogLevel( logLevelML->getCurrentTextInd() );
	} else if ( widget == pathFindingQualityML ) {
		uc->setPathFindingQuality( pathFindingQualityML->getCurrentTextInd() );
	} else if ( widget == videoResolutionML ) {
		string line, s1, s2;
		int end;
		line = videoResolutionML->getCurrentText();
		s1 = Util::getNextWord( line, 0, end );
		s2 = Util::getNextWord( line, end, end );
		s2 = Util::getNextWord( line, end, end );
		uc-> setW( atoi( s1.c_str() ) );
		uc-> setH( atoi( s2.c_str() ) );
	} else if ( widget == fullscreenCheckbox ) {
		uc->setFullscreen( fullscreenCheckbox->isChecked() );
	} else if ( widget == doublebufCheckbox ) {
		uc->setDoublebuf( doublebufCheckbox->isChecked() );
	} else if ( widget == hwaccelCheckbox ) {
		uc->setHwaccel( hwaccelCheckbox->isChecked() );
	} else if ( widget == stencilbufCheckbox ) {
		uc->setStencilbuf( stencilbufCheckbox->isChecked() );
	} else if ( widget == multitexturingCheckbox ) {
		uc->setMultitexturing( multitexturingCheckbox->isChecked() );
	} else if ( widget == anisofilterCheckbox ) {
		uc->setAnisoFilter( anisofilterCheckbox->isChecked() );
	} else if ( widget == shadowsML ) {
		uc->setShadows( shadowsML->getCurrentTextInd() );
	} else if ( widget == closeButton ) {
		scourge->toggleOptionsWindow();
		return true;
	} else if ( widget == saveButton ) {
		uc->saveConfiguration();
		if ( selectedMode == VIDEO ) {
			changeTakeEffectLabel -> setText( _( "Some changes will only take effect upon restart" ) );
		}
		scourge->toggleOptionsWindow();
		scourge->showMessageDialog( _( "Saved: some options require a restart to take effect." ) );
		return true;
	} else if ( widget == musicVolume ) {
		scourge->getSession()->getSound()->setMusicVolume( musicVolume->getValue() );
		uc->setMusicVolume( musicVolume->getValue() );
	} else if ( widget == effectsVolume ) {
		scourge->getSession()->getSound()->setEffectsVolume( effectsVolume->getValue() );
		uc->setEffectsVolume( effectsVolume->getValue() );
	}
	setSelectedMode();

	return false;
}

bool OptionsMenu::handleEvent( SDL_Event *event ) {
	switch ( event->type ) {
	case SDL_MOUSEBUTTONUP:
		break;
	case SDL_KEYUP:
		switch ( event->key.keysym.sym ) {
		case SDLK_ESCAPE:
			if ( !ignoreKeyUp ) {
				scourge->toggleOptionsWindow();
				return true;
			} else ignoreKeyUp = false;
		default:
			break;
		}
		break;
	case SDL_KEYDOWN:
		if ( waitingForNewKey ) {
			if ( event->key.keysym.sym != SDLK_ESCAPE ) {
				int ind = controlBindingsList->getSelectedLine();
				string s1 = uc->getEngineActionDescription( ind );
				string s2 = SDL_GetKeyName( event->key.keysym.sym );
				replace( s2.begin(), s2.end(), ' ', '_' );
				uc->setKeyForEngineAction( s2, ind ); // update userConfig too
				controlBindingsList->setLine( ind, s1 + "           " + s2 );
				controlBindingsList->setSelectedLine( ind );
			} else ignoreKeyUp = true;
			changeControlButton->setLabel( Constants::getMessage( Constants::CHANGE_KEY ) );
			waitingForNewKey = false;
		} else {
			switch ( event->key.keysym.sym ) {
			case SDLK_RETURN:
				if ( selectedMode == CONTROLS ) {
					changeControlButton->setLabel( Constants::getMessage( Constants::WAITING_FOR_KEY ) );
					waitingForNewKey = true;
				}

				return true;
			default: break;
			}
		}
	default: break;
	}
	return false;
}

OptionsMenu::~OptionsMenu() {
	// widgets are deleted by mainWin
	delete cards;
	delete mainWin;
}

void OptionsMenu::show() {
	mainWin->setVisible( true );
}

void OptionsMenu::hide() {
	mainWin->setVisible( false );
}
