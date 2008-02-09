/***************************************************************************
                          infogui.cpp  -  description
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

#include "infogui.h"
#include "item.h"
#include "creature.h"
#include "shapepalette.h"
#include "rpg/rpglib.h"
#include "rpg/spell.h"
#include "rpg/specialskill.h"

using namespace std;

typedef struct _ColorMark {
  char c;
  Color color;
} ColorMark;

ColorMark colors[] = {
  { '%', Color( 1, 0, 0 ) },
  { '^', Color( 0, 1, 0 ) },
  { '&', Color( 0, 1, 1 ) },
  { 0, Color( 0, 0, 0 ) }
};

InfoGui::InfoGui(Scourge *scourge) {
  this->scourge = scourge;

  int width = 350;
  int height = 400;

  int x = (scourge->getSDLHandler()->getScreen()->w - width) / 2;
  int y = (scourge->getSDLHandler()->getScreen()->h - height) / 2;

  win = scourge->createWindow( x, y, width, height, Constants::getMessage(Constants::INFO_GUI_TITLE) );
  int bx = width / 2;
  int by = height - 55;
  
	closeButton = new Button( bx - 150, by, bx - 55, by + 20, 
                           scourge->getShapePalette()->getHighlightTexture(), 
                           Constants::getMessage(Constants::CLOSE_LABEL) );
  win->addWidget((Widget*)closeButton);

	openButton = new Button( bx - 50, by, bx + 50, by + 20, 
                           scourge->getShapePalette()->getHighlightTexture(), 
                           Constants::getMessage(Constants::OPEN_CONTAINER_LABEL ) );
  win->addWidget((Widget*)openButton);

  idButton = new Button( bx + 55, by, bx + 150, by + 20, 
                           scourge->getShapePalette()->getHighlightTexture(), 
                           _( "Identify" ) );
  win->addWidget((Widget*)idButton);
  
  int n = 48;
  image = new Canvas( width - n - 10, 15, width - 10, 15 + 50, this );
  win->addWidget( image );

  win->createLabel(10, 10, _( "Name:" ), Constants::RED_COLOR);
  strcpy(name, "");
  nameLabel = new ScrollingLabel(10, 15, width - n - 25, 50, name );
  win->addWidget(nameLabel);

  win->createLabel(10, 80, _( "Detailed Description:" ), Constants::RED_COLOR);
  strcpy(description, "");
  label = new ScrollingLabel( 10, 95, width - 20, by - 105, description );
  for( int i = 0; colors[i].c; i++ ) {
    label->addColoring( colors[i].c, colors[i].color );
  }
  label->setInteractive( false );
  win->addWidget(label);
}

InfoGui::~InfoGui() {
  delete win;
}

void InfoGui::setItem( Item *item ) { 
  this->spell = NULL;
  this->skill = NULL;
  this->item = item; 
  describe(); 

	int bx = win->getWidth() / 2;
  int by = win->getHeight() - 55;
	if( item->getRpgItem()->isContainer() ) {

		idButton->setVisible( false );
		openButton->setVisible( true );
		closeButton->move( bx - 100, by );		
		openButton->move( bx + 5, by );
		idButton->move( 0, 0 );
		/*
		openButton->setVisible( true );		
		closeButton->move( bx - 150, by );
		openButton->move( bx - 50, by );
		idButton->move( bx + 55, by );
		*/
	} else if( !item->isIdentified() && item->isMagicItem() ) {
		openButton->setVisible( false );
		idButton->setVisible( true );
		closeButton->move( bx - 100, by );
		openButton->move( 0, 0 );
		idButton->move( bx + 5, by );
	} else {
		openButton->setVisible( false );
		idButton->setVisible( false );
		closeButton->move( bx - 45, by );
		openButton->move( 0, 0 );
		idButton->move( 0, 0 );
	}
}

void InfoGui::setSpell( Spell *spell ) { 
  this->item = NULL;
  this->skill = NULL; 
  this->spell = spell; 
  describe(); 

  int bx = win->getWidth() / 2;
  int by = win->getHeight() - 55;

  openButton->setVisible( false );
  idButton->setVisible( false );
  closeButton->move( bx - 45, by );
  openButton->move( 0, 0 );
  idButton->move( 0, 0 );
}

void InfoGui::setSkill( SpecialSkill *skill ) { 
  this->item = NULL;
  this->spell = NULL;
  this->skill = skill; 
  describe(); 

  int bx = win->getWidth() / 2;
  int by = win->getHeight() - 55;

  openButton->setVisible( false );
  idButton->setVisible( false );
  closeButton->move( bx - 45, by );
  openButton->move( 0, 0 );
  idButton->move( 0, 0 );
}

bool InfoGui::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == win->closeButton) {
    win->setVisible(false);
    return true;
  } else if(widget == closeButton) {
    win->setVisible(false);
  } else if( widget == idButton ) {
		if( !item->isIdentified() ) {

			char key[255];
			snprintf( key, 255, "ID_ITEM.%s", scourge->getParty()->getPlayer()->getName() );
			int n = scourge->getSession()->getCountForDate( key );
			if( n < 10 ) {
				item->identify( scourge->getParty()->getPlayer()->
												getSkill( Skill::IDENTIFY_ITEM ) );
				describe();
				// hand out some experience
				if( item->isIdentified() ) {
					setItem( item ); // re-draw buttons
					float n = (float)( item->getLevel() * 5 );
					scourge->getParty()->getPlayer()->
						addExperienceWithMessage( (int)Util::roll( n, 2 * n ) );
				} else {
					// there can only be 10 failures per hour
					scourge->getSession()->setCountForDate( key, n + 1 );
				}
			} else {
				scourge->showMessageDialog( _( "You must rest a while before identifying items again." ) );
			}
		}
	} else if( widget == openButton ) {
		scourge->openContainerGui( item );
	}
  return false;
}

void InfoGui::drawWidgetContents(Widget *w) {

  if( w == image && ( item || spell || skill ) ) {
    glEnable( GL_ALPHA_TEST );
    //glAlphaFunc( GL_EQUAL, 0xff );
		glAlphaFunc( GL_NOTEQUAL, 0 );
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    //    glTranslatef( x, y, 0 );
	if ( hasItem() ) {
	  glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->tilesTex[ item->getRpgItem()->getIconTileX() ][ item->getRpgItem()->getIconTileY() ] );
	} else if ( hasSpell() ) {
          glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->spellsTex[ spell->getIconTileX() ][ spell->getIconTileY() ] );
	} else if ( hasSkill() ) {
          glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->spellsTex[ skill->getIconTileX() ][ skill->getIconTileY() ] );
	}

    glColor4f(1, 1, 1, 1);
    
    glBegin( GL_QUADS );
    glNormal3f( 0, 0, 1 );
    glTexCoord2f( 0, 0 );
    glVertex3f( 0, 0, 0 );
    glTexCoord2f( 0, 1 );
    glVertex3f( 0, image->getHeight(), 0 );
    glTexCoord2f( 1, 1 );
    glVertex3f( image->getWidth(), image->getHeight(), 0 );
    glTexCoord2f( 1, 0 );
    glVertex3f( image->getWidth(), 0, 0 );
    glEnd();
    glPopMatrix();
    
    glDisable( GL_ALPHA_TEST );
    glDisable(GL_TEXTURE_2D);
  }
}

void InfoGui::describeRequirements( char *description, int influenceTypeCount ) {
	enum {TXT_SIZE = 1000 };  
	char tmp[ TXT_SIZE ];
	strcat( description, "|" );
  strcat( description, _( "Requirements" ) );
	strcat( description, ":|" );
  for( int r = 0; r < scourge->getParty()->getPartySize(); r++ ) {
    snprintf( tmp, TXT_SIZE, "%s: ", scourge->getParty()->getParty( r )->getName() );
    strcat( description, tmp );
    bool found = false;
    for( int i = 0; i < Skill::SKILL_COUNT; i++ ) {
      for( int t = 0; t < influenceTypeCount; t++ ) {
        WeaponInfluence *minValue = item->getRpgItem()->getWeaponInfluence( i, t, MIN_INFLUENCE );
        WeaponInfluence *maxValue = item->getRpgItem()->getWeaponInfluence( i, t, MAX_INFLUENCE );
        if( minValue->base > 0 || maxValue->base > 0 ) {
          int skill = scourge->getParty()->getParty( r )->getSkill( i );
          if( minValue->limit > skill ) {
            snprintf( tmp, TXT_SIZE, _( "%s is too low." ), Skill::skills[ i ]->getDisplayName() );
            strcat( description, tmp );
						strcat( description, "|" );
            found = true;
            // only show a skill once
            break;
          } else if( maxValue->limit < skill ) {
            snprintf( tmp, TXT_SIZE, _( "Bonus for %s!" ), Skill::skills[ i ]->getDisplayName() );
            strcat( description, tmp );
						strcat( description, "|" );
            found = true;
            // only show a skill once
            break;
          }
        }
      }
    }
    if( !found ) {
      strcat( description, _( "All requirements met." ) );
			strcat( description, "|" );
    }
  }
}

void InfoGui::describe() {

  if ( hasSpell() ) {
    // describe spell
    if(!spell) return;

    snprintf( name, NAME_SIZE, _( "(L:%d) (M:%d) %s" ), spell->getLevel(), spell->getMp(), spell->getDisplayName() );
    nameLabel->setText( name );

    enum { TXT_SIZE = 1000 };  
    char tmp[ TXT_SIZE ];

    // detailed description
    strcpy(description, spell->getNotes());
    strcat(description, "||");

    // basic info
    snprintf(tmp, TXT_SIZE, _( "School: %s" ), spell->getSchool()->getDisplayName());
    strcat( description, tmp );
    strcat( description, "|" );

    snprintf(tmp, TXT_SIZE, _( "Skill: %s" ), Skill::skills[ spell->getSchool()->getSkill() ]->getDisplayName() );
    strcat( description, tmp );
    strcat( description, "|" );

    snprintf(tmp, TXT_SIZE, _( "Deity: %s" ), spell->getSchool()->getDeity());
    strcat( description, tmp );
    strcat( description, "||" );

    snprintf(tmp, TXT_SIZE, _( "Level: %d" ), spell->getLevel());
    strcat( description, tmp );
    strcat( description, "|" );

    snprintf(tmp, TXT_SIZE, _( "MP: %d" ), spell->getMp());
    strcat( description, tmp );
    strcat( description, "|" );

    snprintf(tmp, TXT_SIZE, _( "Failure rate: %d percent" ), spell->getFailureRate());
    strcat( description, tmp );
    strcat( description, "|" );

    label->setText(description);

    return;

  }

  if ( hasSkill() ) {
    // describe special skill
    if(!skill) return;

    snprintf( name, NAME_SIZE, _( "(%s) %s" ), (skill->getType() == SpecialSkill::SKILL_TYPE_MANUAL) ? _( "M" ) : _( "A" ), skill->getDisplayName() );
    nameLabel->setText( name );

    enum { TXT_SIZE = 1000 };  
    char tmp[ TXT_SIZE ];

    // detailed description
    strcpy(description, skill->getDescription());
    strcat(description, "||");

    // basic info
    snprintf(tmp, TXT_SIZE, _( "Type: %s" ), (skill->getType() == SpecialSkill::SKILL_TYPE_MANUAL) ? _( "Manual" ) : _( "Automatic" ));
    strcat( description, tmp );
    strcat( description, "|" );

    label->setText(description);

    return;

  }

  // describe item
  if(!item) return;
  std::string descr;
  item->getDetailedDescription( descr );
  snprintf( name, NAME_SIZE, "%s", descr.c_str() );
  nameLabel->setText( name );

	enum { TXT_SIZE = 1000 };  
	char tmp[ TXT_SIZE ];

  // detailed description
  strcpy(description, item->getRpgItem()->getLongDesc());
  strcat(description, "||");

  appendMagicItemInfo( description, item );

  // basic info
  snprintf(tmp, TXT_SIZE, _( "Level: %d" ), item->getLevel());
  strcat( description, tmp );
	strcat( description, "|" );
  snprintf(tmp, TXT_SIZE, _( "Weight: %.2f" ), item->getWeight());
  strcat( description, tmp );
	strcat( description, "|" );
  snprintf(tmp, TXT_SIZE, _( "Price: %d" ), item->getPrice());
  strcat( description, tmp );
	strcat( description, "|" );
  if( item->getRpgItem()->isWeapon() ) {
    snprintf(tmp, TXT_SIZE, _( "Damage: %d percent" ), item->getRpgItem()->getDamage() );
    strcat( description, tmp );
		strcat( description, "|" );
		snprintf(tmp, TXT_SIZE, _( "Damage Type: %s" ), _( RpgItem::DAMAGE_TYPE_NAME[ item->getRpgItem()->getDamageType() ] ) );
    strcat( description, tmp );
		strcat( description, "|" );
		snprintf(tmp, TXT_SIZE, _( "Skill: %s" ), Skill::skills[ item->getRpgItem()->getDamageSkill() ]->getDisplayName() );
		strcat( description, tmp );
		strcat( description, "|" );
		if( item->getRpgItem()->getParry() > 0 ) {
			snprintf(tmp, TXT_SIZE, _( "Parry: %d percent of %s skill" ), item->getRpgItem()->getParry(), 
							Skill::skills[ item->getRpgItem()->getDamageSkill() ]->getDisplayName() );
			strcat( description, tmp );
			strcat( description, "|" );
		}
		if( item->getRpgItem()->getAP() > 0 ) {
			snprintf(tmp, TXT_SIZE, _( "AP cost: %d" ), item->getRpgItem()->getAP() );
			strcat( description, tmp );
			strcat( description, "|" );
		}
		if( item->getRange() > MIN_DISTANCE ) {
			snprintf(tmp, TXT_SIZE, _( "Range: %d" ), item->getRange());
			strcat( description, tmp );
			strcat( description, "|" );
		}    
	}
	if( item->getRpgItem()->isArmor() ) {
		for( int i = 0; i < RpgItem::DAMAGE_TYPE_COUNT; i++ ) {
			snprintf(tmp, TXT_SIZE, _( "Defense vs. %s damage: %d" ), 
							_( RpgItem::DAMAGE_TYPE_NAME[ i ] ), 
							item->getRpgItem()->getDefense( i ) );
			strcat( description, tmp );
			strcat( description, "|" );
		}
		snprintf(tmp, TXT_SIZE, _( "Skill: %s" ), Skill::skills[ item->getRpgItem()->getDefenseSkill() ]->getDisplayName() );
		strcat( description, tmp );
		strcat( description, "|" );
		snprintf(tmp, TXT_SIZE, _( "Dodge penalty: %d" ), item->getRpgItem()->getDodgePenalty() );
    strcat( description, tmp );
		strcat( description, "|" );
	}
	if( item->getRpgItem()->getPotionPower() ) {
		snprintf(tmp, TXT_SIZE, _( "Power: %d" ), item->getRpgItem()->getPotionPower() );
    strcat( description, tmp );
		strcat( description, "|" );
	}
  if( item->getRpgItem()->getMaxCharges() > 0 &&
			( !item->getRpgItem()->hasSpell() || item->getSpell() ) ) {
    snprintf(tmp, TXT_SIZE, _( "Charges: %d(%d)" ), item->getCurrentCharges(), item->getRpgItem()->getMaxCharges() );
    strcat( description, tmp );
		strcat( description, "|" );
    if( item->getSpell() ) {
      snprintf( tmp, TXT_SIZE, _( "Spell: %s" ), item->getSpell()->getDisplayName() );
      strcat( description, tmp );
			strcat( description, "|" );
      snprintf( tmp, TXT_SIZE, "%s|", item->getSpell()->getNotes() );
      strcat( description, tmp );
    }
  }
  if( item->getRpgItem()->getPotionTime() > 0 ) {
    snprintf(tmp, TXT_SIZE, _( "Duration: %d" ), item->getRpgItem()->getPotionTime());
    strcat( description, tmp );
		strcat( description, "|" );
  }
  switch( item->getRpgItem()->getTwoHanded() ) {
  case RpgItem::ONLY_TWO_HANDED: 
		snprintf( tmp, TXT_SIZE, _( "Two handed weapon." ) );
	strcat( description, tmp );
	strcat( description, "|" );
  break;
  case RpgItem::OPTIONAL_TWO_HANDED:
    snprintf( tmp, TXT_SIZE, _( "Optionally handed weapon." ) );
  strcat( description, tmp );
	strcat( description, "|" );
  break;
  default:
    break;
  }
  
  if( item->getRpgItem()->isWeapon() ) {
    snprintf( tmp, TXT_SIZE, _( "Attack Info for each player:" ) );
		strcat( description, "|" );
    strcat( description, tmp );
		strcat( description, "|" );
    float max, min;
    for( int i = 0; i < scourge->getSession()->getParty()->getPartySize(); i++ ) {
      char *err = 
        ( scourge->getSession()->getParty()->getParty( i )->isEquipped(item) ? 
          NULL :
          scourge->getSession()->getParty()->getParty( i )->
          canEquipItem( item, false ) );
      if( !err ) {
        scourge->getSession()->getParty()->getParty( i )->
          getAttack( item, &max, &min );
        if( toint( max ) > toint( min ) )
          snprintf( tmp, TXT_SIZE, "#%d ^%s: %d - %d (%.2f %s)|", 
                   ( i + 1 ), _( "ATK" ), toint( min ), toint( max ), 
                   scourge->getSession()->getParty()->getParty( i )->
                   getAttacksPerRound( item ), _( "APR" ) );
        else
          snprintf( tmp, TXT_SIZE, "#%d ^%s: %d (%.2f %s)|", 
                   ( i + 1 ), _( "ATK" ), toint( min ), 
                   scourge->getSession()->getParty()->getParty( i )->
                   getAttacksPerRound( item ),
									 _( "APR" ) );
      } else {
        snprintf( tmp, TXT_SIZE, "#%d %%%s: %s|", ( i + 1 ), _( "ATK" ), err );
      }
      strcat( description, tmp );
    }

		describeRequirements( description, INFLUENCE_TYPE_COUNT );
  } else if( item->getRpgItem()->isArmor() ) {
		describeRequirements( description, 1 );
	}

  label->setText(description);
}

void InfoGui::appendMagicItemInfo( char *description, Item *item ) {
	enum { TXT_SIZE = 1000 };  
	char tmp[ TXT_SIZE ];

  // DEBUG
  //infoDetailLevel = 100;
  // DEBUG
  bool missedSomething = false;
  if(item->isMagicItem()) {
    if( item->getIdentifiedBit( Item::ID_BONUS ) ) {

      if( item->getRpgItem()->isWeapon() ) {
        snprintf(tmp, TXT_SIZE, _( "%d bonus to attack and damage." ), item->getBonus() );
      } else {
        snprintf(tmp, TXT_SIZE, _( "%d bonus to armor points." ), item->getBonus() );
      }
      strcat(description, tmp);
      strcat( description, "|" );
    } else {
      missedSomething = true;
    }
    if( item->getDamageMultiplier() > 1 ) {
      if( item->getIdentifiedBit( Item::ID_DAMAGE_MUL ) ) {
        if( item->getDamageMultiplier() == 2 ) {
          snprintf( tmp, TXT_SIZE, _( "Double damage" ) );
          strcat( description, tmp );
          strcat( description, "|" );
        } else if( item->getDamageMultiplier() == 3 ) {
          snprintf( tmp, TXT_SIZE, _( "Triple damage" ) );
          strcat( description, tmp );
          strcat( description, "|" );
        } else if( item->getDamageMultiplier() == 4 ) {
          snprintf( tmp, TXT_SIZE, _( "Quad damage" ) );          
          strcat( description, tmp );
          strcat( description, "|" );
        } else if( item->getDamageMultiplier() > 4 ) {
          snprintf( tmp, TXT_SIZE, _( "%dX damage" ), item->getDamageMultiplier());
          strcat( description, tmp );
          strcat( description, "|" );
        }
        if( item->getMonsterType() ) {
          char *p = Monster::getDescriptiveType( item->getMonsterType() );
          snprintf( tmp, TXT_SIZE, " %s %s.", _( "vs." ), ( p ? p : item->getMonsterType() ));
          strcat( description, tmp );
        } else {
          strcat( description, _( " vs. any creature." ) );
        }
      } else {
        missedSomething = true;
      }
    }
    if(item->getSchool() ) {
      if( item->getIdentifiedBit( Item::ID_MAGIC_DAMAGE ) ) {
        if(item->getRpgItem()->isWeapon()) {
          snprintf(tmp, TXT_SIZE, _( "Extra damage of %1$s %2$s magic." ), 
                  item->describeMagicDamage(),
                  item->getSchool()->getName());
        } else {
          snprintf(tmp, TXT_SIZE, _( "Extra %d pts of %s magic resistance." ), 
                  item->getMagicResistance(),
                  item->getSchool()->getName());
        }
        strcat(description, tmp);
        strcat( description, "|" );
      } else {
        missedSomething = true;
      }
    }
    if( item->getIdentifiedBit( Item::ID_STATE_MOD ) ) {
      strcpy(tmp, _( "Sets state mods:" ) );
      bool found = false;
      for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
        if(item->isStateModSet(i)) {
          strcat(tmp, " ");
          strcat( tmp, StateMod::stateMods[i]->getDisplayName() );
          found = true;
        }
      }
      if(found) {
        strcat(description, tmp);
        strcat( description, "|" );
      }
    } else {
      missedSomething = true;
    }
    if( item->getIdentifiedBit( Item::ID_PROT_STATE_MOD ) ) {
      strcpy(tmp, _( "Protects from state mods:" ) );
      bool found = false;
      for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
        if(item->isStateModProtected(i)) {
          strcat(tmp, " ");
          strcat( tmp, StateMod::stateMods[i]->getDisplayName() );
          found = true;
        }
      }
      if(found) {
        strcat(description, tmp);
        strcat( description, "|" );
      }
    } else {
      missedSomething = true;
    }
    if( item->getIdentifiedBit( Item::ID_SKILL_BONUS ) ) {
      bool found = false;
      strcpy( tmp, "" );
      map<int,int> *skillBonusMap = item->getSkillBonusMap();
      for(map<int, int>::iterator i=skillBonusMap->begin(); i!=skillBonusMap->end(); ++i) {
        int skill = i->first;
        int bonus = i->second;
        char tmp2[80];
        snprintf(tmp2, 80, " %s+%d", Skill::skills[skill]->getDisplayName(), bonus);
        strcat( tmp, tmp2 );
        found = true;
      }
      if(found) {
        strcat(description, _( "Bonuses to skills:" ) );
        strcat(description, tmp);
        strcat( description, "|" );
      }
    } else {
      missedSomething = true;
    }
    // cursed is hard to detect
    if( item->isCursed() ) {
      if( item->getShowCursed() || 
          item->getIdentifiedBit( Item::ID_CURSED ) ) {
        strcat(description, _( "This item is cursed!" ) );
        strcat( description, "|" );
      }
    }
  } else if(item->getRpgItem()->getType() == RpgItem::SCROLL) {
    strcat(description, "|");
    strcat(description, _( "School: " ) );
    strcat(description, item->getSpell()->getSchool()->getName() );
    strcat(description, "|");
    strcat(description, item->getSpell()->getNotes());
  }

  if( missedSomething ) {
    strcat( description, _( "You have a feeling there is more to this item than what you've been able to glean..." ) );
    strcat( description, "|" );
  }

  strcat( description, "|" );
}

