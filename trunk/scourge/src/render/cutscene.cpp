/***************************************************************************
                          cutscene.h  -  description
                             -------------------
    begin                : Tue May 13 2008
    copyright            : (C) 2008 by Dennis Murczak
    email                : dmurczak@versanet.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cutscene.h"

Cutscene::Cutscene( Session *session ){
  this->session = session;
}

Cutscene::~Cutscene(){
}
