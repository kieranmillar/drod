/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Deadly Rooms of Death.
 *
 * The Initial Developer of the Original Code is
 * Caravel Software.
 * Portions created by the Initial Developer are Copyright (C) 1995, 1996,
 * 1997, 2000, 2001, 2002, 2005 Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Kieran Millar (kieranmillar)
 *
 * ***** END LICENSE BLOCK ***** */

 //Construct.h
 //Declarations for CConstruct.
 //Class for handling construct monster game logic.

#ifndef CONSTRUCT_H
#define CONSTRUCT_H

#include "Monster.h"
#include "MonsterFactory.h"

class CConstruct : public CMonsterFacesTarget
{
public:
	CConstruct(CCurrentGame* pSetCurrentGame = NULL) : CMonsterFacesTarget(M_CONSTRUCT, pSetCurrentGame) {}
	IMPLEMENT_CLONE_REPLICATE(CMonster, CConstruct);
};

#endif //...#ifndef CONSTRUCT_H