/***************************************************************************
                          inventory.cpp  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#include "equip.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "sound.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "item.h"
#include "creature.h"
#include "tradedialog.h"
#include "uncursedialog.h"
#include "identifydialog.h"
#include "rechargedialog.h"
#include "sqbinding/sqbinding.h"
#include "characterinfo.h"
#include "shapepalette.h"
#include "skillsview.h"
#include "gui/confirmdialog.h"
#include "pcui.h"
#include "gui/scrollinglabel.h"
#include "storable.h"

using namespace std;

/**
  *@author Gabor Torok
  */

#define SPELL_SIZE 30
#define DEBUG_SPECIAL_SKILL 0
#define GRID_SIZE 32

// fixme: this should be a property of the magicschool and should come from the .cfg file.
char *schoolIcons[] = {
	"nature", "divine", "life", "history", "tricks", "confrontation"
};

Equip::Equip( PcUi *pcUi, int x, int y, int w, int h ) {
	this->pcUi = pcUi;
	this->creature = NULL;
	this->backgroundTexture = pcUi->getScourge()->getShapePalette()->getNamedTexture( "equip" );
	this->scrollTexture = pcUi->getScourge()->getShapePalette()->getNamedTexture( "scroll" );
	this->currentHole = -1;
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->lastItem = NULL;
	this->mode = EQUIP_MODE;
	this->schoolIndex = -1;
	this->spellIndex = -1;
	this->specialSkill = NULL;
	this->storable = NULL;

	canvas = new Canvas( x, y, x + w, y + h, this, this);
	canvas->setDrawBorders( false );
}

Equip::~Equip() {
}

bool Equip::handleEvent(Widget *widget, SDL_Event *event) {
	if( widget == canvas ) {
		if( pcUi->getScourge()->getSDLHandler()->isDoubleClick ) {
			if( mode == EQUIP_MODE ) {
				Item *item = getItemAtPos( pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x, 
																	 pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT );
				if( item && item->getRpgItem()->isContainer() ) {
					pcUi->getScourge()->openContainerGui(item);
				}
			}
		} else if( pcUi->getScourge()->getSDLHandler()->mouseButton == SDL_BUTTON_RIGHT ) {
			if( mode == EQUIP_MODE ) {
				Item *item = getItemAtPos( pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x, 
																	 pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - y - TITLE_HEIGHT );
				if( item ) {
					pcUi->getScourge()->getInfoGui()->setItem( item );
					if( !pcUi->getScourge()->getInfoGui()->getWindow()->isVisible() ) 
						pcUi->getScourge()->getInfoGui()->getWindow()->setVisible( true );
				}
			} else if( mode == SPELLS_MODE ) {
				int mx = pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x;
				int my = pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - TITLE_HEIGHT;
				int si = getSchoolIndex( mx, my );
				int spi = getSpellIndex( mx, my, si );
				Spell *spell = ( si > -1 && spi > -1 ? 
													MagicSchool::getMagicSchool( si )->getSpell( spi ) :
													NULL );
				if( spell ) {
					pcUi->getScourge()->getInfoGui()->setSpell( spell );
					if( !pcUi->getScourge()->getInfoGui()->getWindow()->isVisible() ) pcUi->getScourge()->getInfoGui()->getWindow()->setVisible( true );
				}
			} else if( mode == CAPABILITIES_MODE ) {
				if( specialSkill ) {
				pcUi->getScourge()->getInfoGui()->setSkill( specialSkill );
				if( !pcUi->getScourge()->getInfoGui()->getWindow()->isVisible() ) pcUi->getScourge()->getInfoGui()->getWindow()->setVisible( true );
				}
			}
		} else if( pcUi->getScourge()->getSDLHandler()->mouseButton == SDL_BUTTON_LEFT ) {
			if( mode == SPELLS_MODE ) {
				// reset buttons first b/c it changes the cursor
				bool cast = pcUi->isCastSelected();
				bool store = pcUi->isStoreSpellSelected();
				if( cast || store ) {
					pcUi->hide();
	
					int mx = pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x;
					int my = pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - TITLE_HEIGHT;
					int si = getSchoolIndex( mx, my );
					int spi = getSpellIndex( mx, my, si );
					Spell *spell = ( si > -1 && spi > -1 ? 
													 MagicSchool::getMagicSchool( si )->getSpell( spi ) :
													 NULL );
					if( spell ) {
						if( cast ) {
							castSpell( spell );
						} else if( store ) {
							storeSpell( spell );
						}
					}
				}
			} else if( mode == CAPABILITIES_MODE ) {
				// reset buttons first b/c it changes the cursor
				bool cast = pcUi->isCastSelected();
				bool store = pcUi->isStoreSpellSelected();
				if( cast || store ) {
					pcUi->hide();
	
					if( specialSkill ) {
						if( cast ) {
							useSpecialSkill( specialSkill );
						} else if( store ) {
							storeSpecialSkill( specialSkill );
						}
					}
				}
			}
		} 
	}
  return false;
}

bool Equip::handleEvent(SDL_Event *event) {
	int si, spi, mx, my;
	Item *item;
	Spell *spell;
	enum { TEXT_SIZE = 3000 };
	char tooltip[ TEXT_SIZE ], tmp[ TEXT_SIZE ];
  switch(event->type) {
	case SDL_MOUSEMOTION:
		mx = event->motion.x - pcUi->getWindow()->getX() - x;
		my = event->motion.y - pcUi->getWindow()->getY() - TITLE_HEIGHT;
		if( mode == EQUIP_MODE ) {
			schoolIndex = spellIndex = -1;
			currentHole = getHoleAtPos( mx, my );
			item = getItemInHole( currentHole );
			if( item != lastItem ) {
				lastItem = item;
				if( item ) {
					item->getTooltip( tooltip );
					canvas->setTooltip( tooltip );
				} else {
					canvas->setTooltip( NULL );
				}
			}
		} else if( mode == SPELLS_MODE ) {
			lastItem = NULL;
			si = getSchoolIndex( mx, my );
			spi = getSpellIndex( mx, my, si );
			if( schoolIndex != si || spellIndex != spi ) {
				schoolIndex = si;
				spellIndex = spi;
				spell = ( schoolIndex > -1 && spellIndex > -1 ? 
									MagicSchool::getMagicSchool( schoolIndex )->getSpell( spellIndex ) :
									NULL );
				if( spell && creature && creature->isSpellMemorized( spell ) ) {
					Util::addLineBreaks( spell->getNotes(), tmp );
					snprintf( tooltip, TEXT_SIZE, "%s:|%s|%s:%d %s:%d", 
									 spell->getDisplayName(), 
									 tmp,
									 _( "Level" ), spell->getLevel(),
									 _( "MP Cost" ), spell->getMp() );
					canvas->setTooltip( tooltip );
				} else {
					canvas->setTooltip( "" );
				}
			}
		}

    break;
	default: break;
	}
  return false;
}

/*
// FIXME: brittle. This will break if drawCapabilities is changed.
SpecialSkill *Equip::findSpecialSkill( int x, int y ) {
	int startX = 20;
	int xx = startX;
	int yy = 20;
	yy += 5;
	for( int i = 0; i < SpecialSkill::getSpecialSkillCount(); i++ ) {
		SpecialSkill *ss = SpecialSkill::getSpecialSkill( i );   
		if( 1 || creature->hasSpecialSkill( ss ) ) {
			
		}
	}
}
*/

int Equip::getSchoolIndex( int x, int y ) {
	int n = ( y - 20 ) / 47;
	return( n >= 0 && n < MagicSchool::getMagicSchoolCount() ? n : -1 );
}

int Equip::getSpellIndex( int x, int y, int schoolIndex ) {
	int n = ( x - 20 ) / ( SPELL_SIZE + 2 );
	return( schoolIndex > -1 && 
					n >= 0 && 
					n < MagicSchool::getMagicSchool( schoolIndex )->getSpellCount() ? n : -1 );
}

Item *Equip::getItemInHole( int hole ) {
	return( hole > -1 && creature ? creature->getEquippedInventory( hole ) : NULL );
}

Item *Equip::getItemAtPos( int x, int y ) {
  int hole = getHoleAtPos( x, y );
  return getItemInHole( hole );
}

int Equip::getHoleAtPos( int x, int y ) {
  SDL_Rect point;
  point.x = x;
  point.y = y;
  point.w = point.h = 1;
  for( int i = 0; i < Constants::INVENTORY_COUNT; i++ ) {
    SDL_Rect *rect = pcUi->getScourge()->getShapePalette()->getInventoryHole( i );
    if( SDLHandler::intersects( rect, &point ) ) {
      return i;
    }
  }
  return -1;
}

void Equip::receive( Widget *widget ) {
	if( mode == EQUIP_MODE && creature ) {
		Item *item = pcUi->getScourge()->getMovingItem();
		if( item ) {
			char *err = creature->canEquipItem( item );
			if( err ) {
				pcUi->getScourge()->showMessageDialog( err );
				pcUi->getScourge()->getSession()->getSound()->playSound( Window::DROP_FAILED, 127 );
			} else {
				if( creature->addInventory( item ) ) {
					// message: the player accepted the item
					char message[120];
					snprintf( message, 119, _( "%s picks up %s." ), 
									 creature->getName(),
									 item->getItemName() );
					pcUi->getScourge()->writeLogMessage( message );
					pcUi->getScourge()->endItemDrag();
					int index = creature->findInInventory( item );
					creature->equipInventory( index, currentHole );
					pcUi->getScourge()->getSession()->getSound()->playSound( Window::DROP_SUCCESS, 127 );
				} else {
					// message: the player's inventory is full
					pcUi->getScourge()->getSession()->getSound()->playSound( Window::DROP_FAILED, 127 );
					pcUi->getScourge()->showMessageDialog( _( "You can't fit the item!" ) );
				}
			}
		}
	}
}

bool Equip::startDrag( Widget *widget, int x, int y ) {
	if( mode == EQUIP_MODE && creature ) {
	
		if( pcUi->getScourge()->getTradeDialog()->getWindow()->isVisible() ) {
			pcUi->getScourge()->showMessageDialog( _( "Can't change inventory while trading." ) );
			return false;
		} else if( pcUi->getScourge()->getUncurseDialog()->getWindow()->isVisible() ) {
			pcUi->getScourge()->showMessageDialog( _( "Can't change inventory while employing a sage's services." ) );
			return false;
		} else if( pcUi->getScourge()->getIdentifyDialog()->getWindow()->isVisible() ) {
			pcUi->getScourge()->showMessageDialog( _( "Can't change inventory while employing a sage's services." ) );
			return false;
		} else if( pcUi->getScourge()->getRechargeDialog()->getWindow()->isVisible() ) {
			pcUi->getScourge()->showMessageDialog( _( "Can't change inventory while employing a sage's services." ) );
			return false;
		}
		
		// what's equiped at this inventory slot?
		currentHole = getHoleAtPos( x, y );
		Item *item = getItemAtPos( x, y );
		if( item ) {
			if( item->isCursed() ) {
				pcUi->getScourge()->showMessageDialog( _( "Can't remove cursed item!" ) );
				return false;
			} else {
				creature->removeInventory( creature->findInInventory( item ) );
				pcUi->getScourge()->startItemDragFromGui( item );
				char message[120];
				snprintf(message, 119, _( "%s drops %s." ), 
								creature->getName(),
								item->getItemName() );
				pcUi->getScourge()->writeLogMessage( message );
	
				return true;
			}
		}
	}
  return false;
}

void Equip::setCreature( Creature *creature ) {
	this->creature = creature;
}

void Equip::drawWidgetContents( Widget *widget ) {
	glEnable( GL_TEXTURE_2D );
	//	glEnable( GL_ALPHA_TEST );
	//	glAlphaFunc( GL_NOTEQUAL, 0 );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	if( mode == EQUIP_MODE ) {
		glBindTexture( GL_TEXTURE_2D, backgroundTexture );
	} else {
		glBindTexture( GL_TEXTURE_2D, scrollTexture );
	}
	glColor4f( 1, 1, 1, 1 );
	glBegin( GL_QUADS );
	glTexCoord2d( 0, 1 );
	glVertex2d( 0, h );
	glTexCoord2d( 0, 0 );
	glVertex2d( 0, 0 );
	glTexCoord2d( 1, 0 );
	glVertex2d( w, 0 );
	glTexCoord2d( 1, 1 );
	glVertex2d( w, h );
	glEnd();
	glDisable( GL_BLEND );
	// glDisable( GL_ALPHA_TEST );
	if( creature ) {
		if( mode == EQUIP_MODE ) {
			drawEquipment();
		} else if( mode == SPELLS_MODE ) {
			drawSpells();
		} else {
			drawCapabilities();
		}
	}
}

void Equip::drawEquipment() {
	for( int i = 0; i < Constants::INVENTORY_COUNT; i++ ) {
		SDL_Rect *rect = pcUi->getScourge()->getShapePalette()->getInventoryHole( i );
		Item *item = creature->getEquippedInventory( i );
		if( item ) {
			if( rect && rect->w && rect->h ) {
				glEnable( GL_TEXTURE_2D );
				//int ix = item->getInventoryX() * GRID_SIZE;
				//int iy = item->getInventoryY() * GRID_SIZE;
				int iw = item->getInventoryWidth() * GRID_SIZE;
				int ih = item->getInventoryHeight() * GRID_SIZE;

				int iy = rect->y;
				if(rect->h - ih > GRID_SIZE) iy += rect->h - ih - GRID_SIZE;
				item->renderIcon( pcUi->getScourge(), rect->x + (rect->w - iw) / 2, iy, iw, ih );
				//				item->renderIcon( pcUi->getScourge(), rect->x, rect->y, rect->w, rect->h );
			}
		}
		/*
		if( ( 1 << i ) == creature->getPreferredWeapon() ) {
			glDisable( GL_TEXTURE_2D );
			glColor4f( 1, 0.35f, 0, 1 );
			glBegin( GL_LINE_LOOP );
			glVertex2d( rect->x, rect->y + rect->h );
			glVertex2d( rect->x, rect->y );
			glVertex2d( rect->x + rect->w, rect->y );
			glVertex2d( rect->x + rect->w, rect->y + rect->h );
			glEnd();
		}
		*/
	}
	if( currentHole > -1 ) {
	  SDL_Rect *rect = pcUi->getScourge()->getShapePalette()->getInventoryHole( currentHole );
	  glDisable( GL_TEXTURE_2D );
	  pcUi->getWindow()->setTopWindowBorderColor();
	  glBegin( GL_LINE_LOOP );
	  glVertex2d( rect->x, rect->y + rect->h );
	  glVertex2d( rect->x, rect->y );
	  glVertex2d( rect->x + rect->w, rect->y );
	  glVertex2d( rect->x + rect->w, rect->y + rect->h );
	  glEnd();
	}
}

void Equip::drawSpells() {
	int startX = 20;
	int xx = startX;
	int yy = 20;
	for( int i = 0; i < MagicSchool::getMagicSchoolCount(); i++ ) {
		MagicSchool *school = MagicSchool::getMagicSchool( i );

		glPushMatrix();
		glTranslatef( xx, yy - 12, 0 );

		int size = 15;
		int width = w - 55;

		glDisable( GL_TEXTURE_2D );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		// glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );
		glColor4f( 0, 0, 0, 0.75 );
		glBegin( GL_QUADS );
		glVertex2d( 0, size );
		glVertex2d( 0, 1 );
		glVertex2d( size + width, 1 );
		glVertex2d( size + width, size );
		glEnd();
		glDisable( GL_BLEND );
		glEnable( GL_TEXTURE_2D );

		glEnable( GL_ALPHA_TEST );
		glAlphaFunc( GL_NOTEQUAL, 0 );
		glBindTexture( GL_TEXTURE_2D, pcUi->getScourge()->getShapePalette()->getNamedTexture( schoolIcons[ i ] ) );
		glColor4f( 1, 1, 1, 1 );
		glBegin( GL_QUADS );
		glTexCoord2d( 0, 1 );
		glVertex2d( 0, size );
		glTexCoord2d( 0, 0 );
		glVertex2d( 0, 0 );
		glTexCoord2d( 1, 0 );
		glVertex2d( size, 0 );
		glTexCoord2d( 1, 1 );
		glVertex2d( size, size );
		glEnd();
		glDisable( GL_ALPHA_TEST );

		glColor4f( 1, 0.35f, 0, 1 );
		pcUi->getScourge()->getSDLHandler()->setFontType( Constants::SCOURGE_MONO_FONT );
		pcUi->getScourge()->getSDLHandler()->texPrint( size + 5, 13, school->getDisplayName() );
		pcUi->getScourge()->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );

		glPopMatrix();

		yy += 5;
		for( int t = 0; t < school->getSpellCount(); t++, xx += SPELL_SIZE + 2 ) {
			Spell *spell = school->getSpell( t );
			if( creature && creature->isSpellMemorized( spell ) ) {
				glBindTexture( GL_TEXTURE_2D, pcUi->getScourge()->getShapePalette()->spellsTex[ spell->getIconTileX() ][ spell->getIconTileY() ] );
				glColor4f( 1, 1, 1, 1 );
				glBegin( GL_QUADS );
				glTexCoord2d( 0, 1 );
				glVertex2d( xx, yy + SPELL_SIZE );
				glTexCoord2d( 0, 0 );
				glVertex2d( xx, yy );
				glTexCoord2d( 1, 0 );
				glVertex2d( xx + SPELL_SIZE, yy );
				glTexCoord2d( 1, 1 );
				glVertex2d( xx + SPELL_SIZE, yy + SPELL_SIZE );
				glEnd();
			}
			if( schoolIndex == i && spellIndex == t ) {
				glColor4f( 1, 1, 0, 1 );
			} else {
				pcUi->getWindow()->setTopWindowBorderColor();
			}
			glDisable( GL_TEXTURE_2D );
			glBegin( GL_LINE_LOOP );
			glVertex2d( xx, yy + SPELL_SIZE );
			glVertex2d( xx, yy );
			glVertex2d( xx + SPELL_SIZE, yy );
			glVertex2d( xx + SPELL_SIZE, yy + SPELL_SIZE );
			glEnd();
			glEnable( GL_TEXTURE_2D );
		}

		xx = startX;
		yy += SPELL_SIZE + 12;
	}
}

void Equip::drawCapabilities() {
	int startX = 20;
	int xx = startX;
	int yy = 20;

	glPushMatrix();
	glTranslatef( xx, yy - 12, 0 );
		
	int size = 15;
	int width = w - 55;
	
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	// glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );
	glColor4f( 0, 0, 0, 0.75 );
	glBegin( GL_QUADS );
	glVertex2d( 0, size );
	glVertex2d( 0, 1 );
	glVertex2d( size + width, 1 );
	glVertex2d( size + width, size );
	glEnd();
	glDisable( GL_BLEND );
	glEnable( GL_TEXTURE_2D );
	
	glColor4f( 1, 0.35f, 0, 1 );
	pcUi->getScourge()->getSDLHandler()->setFontType( Constants::SCOURGE_MONO_FONT );
	pcUi->getScourge()->getSDLHandler()->texPrint( size + 5, 13, _( "Special Capabilities" ) );
	pcUi->getScourge()->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );

	glPopMatrix();

	int mx = pcUi->getScourge()->getSDLHandler()->mouseX - pcUi->getWindow()->getX() - x;
	int my = pcUi->getScourge()->getSDLHandler()->mouseY - pcUi->getWindow()->getY() - TITLE_HEIGHT;

	yy += 5;
	bool found = false;
	for( int i = 0; i < SpecialSkill::getSpecialSkillCount(); i++ ) {
		SpecialSkill *ss = SpecialSkill::getSpecialSkill( i );   
		if( DEBUG_SPECIAL_SKILL || creature->hasSpecialSkill( ss ) ) {

			if( !found &&
					mx >= xx && mx < xx + SPELL_SIZE &&
					yy >= yy && my < yy + SPELL_SIZE ) {
				if( specialSkill != ss ) {
					specialSkill = ss;
					enum { TEXT_SIZE = 3000 };
					char tmp[ TEXT_SIZE ], tooltip[ TEXT_SIZE ];
					Util::addLineBreaks( ss->getDescription(), tmp );
					snprintf( tooltip, TEXT_SIZE, "%s:|%s|%s", 
									 ss->getDisplayName(), 
									 tmp,
									 ss->getType() == SpecialSkill::SKILL_TYPE_MANUAL ? 
									 _( "Manual Capability" ) : 
									 _( "Automatic Capability" ) );
					canvas->setTooltip( tooltip );
				}
				found = true;
			}

			glBindTexture( GL_TEXTURE_2D, pcUi->getScourge()->getShapePalette()->spellsTex[ ss->getIconTileX() ][ ss->getIconTileY() ] );
			glColor4f( 1, 1, 1, 1 );
			glBegin( GL_QUADS );
			glTexCoord2d( 0, 1 );
			glVertex2d( xx, yy + SPELL_SIZE );
			glTexCoord2d( 0, 0 );
			glVertex2d( xx, yy );
			glTexCoord2d( 1, 0 );
			glVertex2d( xx + SPELL_SIZE, yy );
			glTexCoord2d( 1, 1 );
			glVertex2d( xx + SPELL_SIZE, yy + SPELL_SIZE );
			glEnd();

			if( specialSkill == ss ) {
				glColor4f( 1, 1, 0, 1 );
			} else {
				pcUi->getWindow()->setTopWindowBorderColor();
			}
			glDisable( GL_TEXTURE_2D );
			glBegin( GL_LINE_LOOP );
			glVertex2d( xx, yy + SPELL_SIZE );
			glVertex2d( xx, yy );
			glVertex2d( xx + SPELL_SIZE, yy );
			glVertex2d( xx + SPELL_SIZE, yy + SPELL_SIZE );
			glEnd();
			glEnable( GL_TEXTURE_2D );

			xx += SPELL_SIZE + 2;
			if( xx > w - 50 ) {
				xx = startX;
				yy += SPELL_SIZE + 2;
			}
		}
	}
	if( !found ) {
		specialSkill = false;
		canvas->setTooltip( "" );
	}
}

void Equip::useSpecialSkill( SpecialSkill *ss ) {
	if( ss && ( DEBUG_SPECIAL_SKILL || creature->hasSpecialSkill( ss ) ) ) {
		if( ss->getType() != SpecialSkill::SKILL_TYPE_MANUAL ) {
			pcUi->getScourge()->showMessageDialog( _( "Only manual type capabilities can be used this way." ) );
		} else {

			creature->setAction( Constants::ACTION_SPECIAL, NULL, NULL, ss );
			creature->setTargetCreature( creature );
	
			// set this as a quickspell if there is space
			for( int i = 0; i < 12; i++ ) {
				if( !creature->getQuickSpell( i ) ) {
					creature->setQuickSpell( i, ss );
					break;
				}
			}
		}
	}
}

void Equip::castSpell( Spell *spell ) {
	if( spell ) {
		if( spell->getMp() > creature->getMp() ) {
			pcUi->getScourge()->showMessageDialog( _( "Not enough Magic Points to cast this spell!" ) );
		} else {
			// set this as a quickspell if there is space
			for( int i = 0; i < 12; i++ ) {
				if( !creature->getQuickSpell( i ) ) {
					creature->setQuickSpell( i, spell );
					break;
				}
			}
			
			creature->setAction( Constants::ACTION_CAST_SPELL, NULL, spell );
			if( !spell->isPartyTargetAllowed() ) {
				pcUi->getScourge()->setTargetSelectionFor( creature );
			} else {
				creature->setTargetCreature( creature );
			}
		}
	}
}

void Equip::storeSpell( Spell *spell ) {
	if( spell ) {
		storeStorable( (Storable*)spell );
	}
}

void Equip::storeSpecialSkill( SpecialSkill *ss ) {
	if( ss && ( DEBUG_SPECIAL_SKILL || creature->hasSpecialSkill( ss ) ) ) {
		if( ss->getType() != SpecialSkill::SKILL_TYPE_MANUAL ) {
			pcUi->getScourge()->showMessageDialog( _( "Only manual type capabilities can be stored." ) );
		} else {
			storeStorable( (Storable*)ss );
		}
	}
}

void Equip::storeStorable( Storable *storable ) {
	this->storable = NULL;
	const char *p = storable->isStorable();
	if( p ) {
		pcUi->getScourge()->showMessageDialog( p );
	} else {
		this->storable = storable;
		pcUi->getScourge()->showMessageDialog( _( "Click a quickspell slot to store this spell." ) );
	}
}

MissionInfoUI::MissionInfoUI( PcUi *pcUi, int x, int y, int w, int h ) {
	this->pcUi = pcUi;
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;

	description = new ScrollingLabel( x, y, w, h - 95, "" );
	pcUi->getWindow()->addWidget( description );

	objectivesLabel = pcUi->getWindow()->createLabel( x, y + h - 80, _( "Mission Objectives" ) );
	objectiveList = new ScrollingList( x, y + h - 75, w, 45, pcUi->getScourge()->getShapePalette()->getHighlightTexture() );
	pcUi->getWindow()->addWidget( objectiveList );
	consoleButton = pcUi->getWindow()->createButton( x, y + h - 25, x + w, y + h - 5, _( "Show Squirrel Console" ) );
}

MissionInfoUI::~MissionInfoUI() {
	// objectiveText has static size now
}

void MissionInfoUI::refresh() {
	enum { TMP_SIZE = 80, MISSNTXT_SIZE = 3000 }; 
	Color green(0.2f, 0.7f, 0.2f), red(0.7f, 0.2f, 0.2f);
	char missionText[MISSNTXT_SIZE], tmp[TMP_SIZE];
	Scourge *scourge = pcUi->getScourge();
	Mission* curMission = scourge->getSession()->getCurrentMission();
	objectiveText.clear();
	missionColor.clear();

	if( curMission ) {
		snprintf( tmp, TMP_SIZE, _("Depth: %d out of %d."), (scourge->getCurrentDepth() + 1), curMission->getDepth() );
		snprintf( missionText, MISSNTXT_SIZE, "%s:|%s|%s", curMission->getDisplayName(), tmp, curMission->getDescription() );

		for(int t = 0; t < curMission->getItemCount(); t++) {
			snprintf( tmp, TMP_SIZE, _( "Find %s" ), curMission->getItem(t)->getDisplayName() );
			if(curMission->getItemHandled(t)) {
				objectiveText.push_back(tmp + string(". ") + _("(completed)"));
				missionColor.push_back(green);
			}
			else {
				objectiveText.push_back(tmp + string(". ") + _("(not yet found)"));
				missionColor.push_back(red);
			}
		}

		for(int t = 0; t < curMission->getCreatureCount(); t++) {
			snprintf( tmp, TMP_SIZE, _( "Vanquish %s." ), curMission->getCreature(t)->getDisplayName() );
			if(curMission->getCreatureHandled(t)) {
				objectiveText.push_back(tmp + string(". ") + _("(completed)"));
				missionColor.push_back(green);
			}
			else {
				objectiveText.push_back(tmp + string(". ") + _("(not yet done)"));
				missionColor.push_back(red);
			}
		}

		if( objectiveText.size() == 0 && curMission->isCompleted()) {
			objectiveText.push_back(_("Special") + string(". ") + _("(completed)"));
			missionColor.push_back(green);
		}
		else if( objectiveText.size() == 0 ) {
			objectiveText.push_back(_("Special") + string(". ") + _("(not yet done)"));
			missionColor.push_back(red);
		}
		cerr << "objectiveText size=" << objectiveText.size() << endl;
		objectiveList->setLines( objectiveText.begin(), objectiveText.end(), &missionColor[0] );
	} else {
		strncpy(missionText, _( "No current mission." ), MISSNTXT_SIZE );
	}
	description->setText( missionText );
}

void MissionInfoUI::show() {
	description->setVisible( true );
	objectiveList->setVisible( true );
	objectivesLabel->setVisible( true );
	consoleButton->setVisible( true );
}

void MissionInfoUI::hide() {
	description->setVisible( false );
	objectiveList->setVisible( false );
	objectivesLabel->setVisible( false );
	consoleButton->setVisible( false );
}

