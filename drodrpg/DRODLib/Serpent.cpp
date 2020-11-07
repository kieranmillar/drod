// $Id: Serpent.cpp 10108 2012-04-22 04:54:24Z mrimer $

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
 * Contributor(s):
 * Michael Welsh Duggan (md5i)
 *
 * ***** END LICENSE BLOCK ***** */

//Serpent.cpp
//Implementation of CSerpent.

#include "Serpent.h"
#include "MonsterPiece.h"

//
//Public methods.
//

//*****************************************************************************************
CSerpent::CSerpent(const MONSTERTYPE eSerpentType, CCurrentGame *pSetCurrentGame)
	: CMonster(eSerpentType, pSetCurrentGame)
	, wOldTailX(UINT(-1)), wOldTailY(UINT(-1))
{
}

//******************************************************************************************
bool CSerpent::GetSerpentMovement(
//Gets offsets for serpent movement to the swordsman, taking
//obstacles into account.
//
//Params:
	const UINT /*wX*/, const UINT /*wY*/, //(in) target (usually swordsman)
	int& /*dxFirst*/,  //(out) Horizontal delta for where the monster would
					//    go if there weren't obstacles.
	int& /*dyFirst*/,  //(out) Vertical delta for same.
	int& /*dx*/,    //(out) Horizontal delta (-1, 0, or 1) for where monster
					//    can go, taking into account obstacles.
	int& /*dy*/)    //(out) Vertical delta (-1, 0, or 1) for same.
const
{
/*	if (pCurrentGame->bBrainSensesSwordsman)
	{
		if (!GetBrainDirectedMovement(dxFirst, dyFirst, dx, dy))
			GetNormalMovement(wX, wY, dx, dy);	//if brain doesn't give any options, then
															//serpent moves on its own (compat)
		return true;
	}
	if (!CanFindSwordsman())
		return false;
	GetNormalMovement(wX, wY, dx, dy);
	return true;
*/
	return false;
}

//*****************************************************************************************
bool CSerpent::OnStabbed(
//Stabbing a serpent in the head kills it.
//Stabbing it in the tail shortens it by one tile (at the tail end).
//
//Returns: whether monster was killed
//
//Params:
	CCueEvents &CueEvents, const UINT wX, const UINT wY)
{
	if (wX == this->wX && wY == this->wY)
	{
		CueEvents.Add(CID_SnakeDiedFromTruncation, this);
		return true;   //indicates serpent was killed
	}

	if (wX == this->tailX && wY == this->tailY)
	{
		//Remove tail segment.
		if (ShortenTail(CueEvents))
			return true;   //indicates serpent was killed

		CueEvents.Add(CID_MonsterPieceStabbed, new CMoveCoord(wX, wY,
				this->pCurrentGame->GetSwordMovement()), true);
	}

	return false;
}

//*****************************************************************************************
bool CSerpent::LengthenHead(
//Move head out one square (extending body by one).
//
//Returns: whether a move can be made
//
//Params:
	const int dx, const int dy,   //(in)   Offsets that indicate direction
											//    of movement from current square.
	const int oX, const int oY,   //(in)   Current orientation of head.
	CCueEvents &CueEvents)
{
	if (dx || dy)
	{
		ASSERT ((dx == 0) != (dy == 0)); //always moving, no diagonal steps

		UINT tile = T_EMPTY;
		if (dx != 0) 
		{
			switch (nGetO(dx, oY))
			{
			case E: case W:   tile = T_SNK_EW; break;
			case NE: tile = T_SNK_NW; break;
			case SE: tile = T_SNK_SW; break;
			case NW: tile = T_SNK_NE; break;
			case SW: tile = T_SNK_SE; break;
			default: ASSERT(!"Bad orientation."); break;
			}
		} 
		else
		{
			switch(nGetO(oX, dy))
			{
			case N: case S:   tile = T_SNK_NS; break;
			case NE: tile = T_SNK_SE; break;
			case SE: tile = T_SNK_NE; break;
			case NW: tile = T_SNK_SW; break;
			case SW: tile = T_SNK_NW; break;
			default: ASSERT(!"Bad orientation.(2)"); break;
			}
		}
		Move(this->wX + dx, this->wY + dy, &CueEvents);
		//Add segment to the old spot (wX and wY were just updated).
		this->pCurrentGame->pRoom->Plot(this->wX - dx, this->wY - dy, tile, this);

		wO = nGetO(dx, dy);
		return true;
	}
	this->wPrevX = this->wX;
	this->wPrevY = this->wY;
	this->wPrevO = this->wO;
	return false;
}

//*****************************************************************************************
bool CSerpent::ShortenTail(
//Shrink tail by one square (shortening body by one).
//
//Returns: whether snake died from truncation
//
//Params:
	CCueEvents &CueEvents)  //(in/out)
{
	int dx = nGetOX(this->tailO);
	int dy = nGetOY(this->tailO);
	ASSERT((dx==0) != (dy==0));   //always moving, no diagonals
	
	//Remove tail piece.
	this->pCurrentGame->pRoom->Plot(this->tailX, this->tailY, T_NOMONSTER);
	this->wOldTailX = this->tailX;
	this->wOldTailY = this->tailY;
	this->tailX += dx;
	this->tailY += dy;
	if (this->tailX == this->wX && this->tailY == this->wY) 
	{
		CueEvents.Add(CID_SnakeDiedFromTruncation, this);
		return true;
	}

	//Second-to-last piece becomes new tail -- update its tile.
	CMonster *pMonster = this->pCurrentGame->pRoom->GetMonsterAtSquare(
			this->tailX, this->tailY);
	ASSERT(pMonster);
	CMonsterPiece *pBackPiece = DYN_CAST(CMonsterPiece*, CMonster*, pMonster);
	ASSERT(pBackPiece);
	if (!pBackPiece)
	{
		CueEvents.Add(CID_SnakeDiedFromTruncation, this);
		return true; //bad snake data -- kill it off now
	}
	UINT tile = pBackPiece->wTileNo;
	int t;
	switch (tile)
	{
		case T_SNK_NS: case T_SNK_EW: break;
		case T_SNK_NW: case T_SNK_SE: t = dx; dx = -dy; dy = -t; break;
		case T_SNK_NE: case T_SNK_SW: t = dx; dx = dy; dy = t; break;
		default: ASSERT(!"Bad tile."); break;
	}
	switch (nGetO(dx, dy))
	{
		case N: tile = T_SNKT_S; this->tailO = N; break;
		case S: tile = T_SNKT_N; this->tailO = S; break;
		case E: tile = T_SNKT_W; this->tailO = E; break;
		case W: tile = T_SNKT_E; this->tailO = W; break;
		default: ASSERT(!"Bad orientation.(2)"); break;
	}
	pBackPiece->wTileNo = tile;

	//Reset monster's HP back to full to allow attacking another segment.
	SetHP();

	return false;
}

//*****************************************************************************************
bool CSerpent::IsTileObstacle(
// Override the normal IsTileObstacle for serpents.
// 
//Params:
	const UINT wLookTileNo) //(in)   Tile to evaluate.  Note each tile# will
						//    always be found on the same layer of squares.
//
//Returns:
//True if tile is an obstacle, false if not.
const
{
	return (!(
			bIsFloor(wLookTileNo) ||
			bIsOpenDoor(wLookTileNo) ||
			bIsPlatform(wLookTileNo) ||
			wLookTileNo==T_EMPTY ||
			wLookTileNo==T_NODIAGONAL ||
			wLookTileNo==T_FUSE ||
			wLookTileNo==T_KEY ||
			bIsEquipment(wLookTileNo) ||
			bIsMap(wLookTileNo) ||
			wLookTileNo==T_TOKEN
			//should have T_SCROLL and ARROWs also, but they were left out of
			//Webfoot/Caravel DROD (vv. 1.0 -- 1.6) since the serpent pieces were 
			//located in the t-layer
			));
}

//*****************************************************************************
void CSerpent::OrderPieces()
//Serpent pieces are not maintained in semantic order, i.e. along the length of
//the serpent's body, from head to tail.
//Call this method to ensure the pieces list is put into this order.
{
	//Scan down the length of the serpent's body by position.
	//The tile at each position indicates in which direction the successor piece is.
	int dx = -(int)nGetOX(this->wO); //direction from head to body
	int dy = -(int)nGetOY(this->wO);
	ASSERT ((dx == 0) != (dy == 0));
	UINT x = this->wX, y = this->wY;  //start at head

	list<CMonsterPiece*> orderedPieces;
	while (!this->Pieces.empty())
	{
		//Advance to next piece.
		x += dx;
		y += dy;

		//Find piece located at (x,y).
		UINT tile = T_EMPTY;
		for (list<CMonsterPiece*>::iterator piece = this->Pieces.begin();
				piece != this->Pieces.end(); ++piece)
		{
			CMonsterPiece& p = *(*piece);
			if (p.wX == x && p.wY == y)
			{
				//Put into ordered list.
				orderedPieces.push_back(*piece);
				this->Pieces.erase(piece);

				tile = p.wTileNo;
				ASSERT(bIsSerpentTile(tile));
				break;
			}
		}

		//Get direction to next piece.
		int t;
		switch (tile)
		{
			case T_SNK_EW: case T_SNK_NS: break;
			case T_SNK_NW: case T_SNK_SE: t = dx; dx = -dy; dy = -t; break;
			case T_SNK_NE: case T_SNK_SW: t = dx; dx = dy; dy = t; break;
			case T_SNKT_S: case T_SNKT_W: case T_SNKT_N: case T_SNKT_E:
				ASSERT(this->Pieces.empty());
			break;
			default: ASSERT(!"Bad serpent tile."); break;
		}
	}

	this->Pieces = orderedPieces;
}

//*****************************************************************************
void CSerpent::GetTail(
//Returns location of the tail.  (Called by the room editor.  Always updates tail info when called.)
//
//Params:
	UINT &wTailX, UINT &wTailY)   //(out) Coords of tail
{
	for (list<CMonsterPiece*>::iterator piece = this->Pieces.begin();
			piece != this->Pieces.end(); ++piece)
	{
		//Find first piece with tail tile.
		const UINT wTileNo = (*piece)->wTileNo;
		switch (wTileNo)
		{
			case T_SNKT_S: this->tailO = N;  break;
			case T_SNKT_W: this->tailO = E;  break;
			case T_SNKT_N: this->tailO = S;  break;
			case T_SNKT_E: this->tailO = W;  break;
			default: continue;   //go to next piece
		}
		wTailX = this->tailX = (*piece)->wX;
		wTailY = this->tailY = (*piece)->wY;
		if (this->wOldTailX == static_cast<UINT>(-1))
		{
			this->wOldTailX = this->tailX;
			this->wOldTailY = this->tailY;
		}
		return;
	}
}

//
//Private methods.
//

//*****************************************************************************
void CSerpent::GetNormalMovement(
// Figures out the normal movement of the serpent when not affected by a brain.
//
//Params:
	const UINT wX, const UINT wY, //(in) target destination (usually swordsman)
	int &dx,    //(out) Horizontal delta (-1, 0, or 1) for where monster
					//    can go, taking into account obstacles.
	int &dy)    //(out) Vertical delta (-1, 0, or 1) for same.
const
{
	const int oX = nGetOX(wO);
	const int oY = nGetOY(wO);

	// If target is ahead or behind the serpent, keep moving toward it.
	// Otherwise, switch between favoring horizontal or vertical movement
	// every five turns.
	if (CanFindSwordsman()) 
	{
		//Is swordsman in front of or behind serpent?
		if (!oX)
		{
			//serpent is moving vertically
			if (wX == this->wX)
			{
				//Yes.  Keep moving this direction.
				dy = oY;
				dx = 0;
				if (CanMoveTo(this->wX + dx, this->wY + dy))
					return;
			}
		} else {
			//serpent moving horizontally
			if (wY == this->wY)
			{
				dx = oX;
				dy = 0;
				if (CanMoveTo(this->wX + dx, this->wY + dy))
					return;
			}
		}

		// Move towards swordsman.
		const bool bHorizontal = (this->pCurrentGame->wSpawnCycleCount % 10) < 5;
		if (bHorizontal)
		{
			dx = sgn(wX - this->wX);
			if (dx == 0)
				dy = sgn(wY - this->wY);
			else
				dy = 0;
		}
		else
		{
			dy = sgn(wY - this->wY);
			if (dy == 0)
				dx = sgn(wX - this->wX);
			else
				dx = 0;
		}

		// Check the coordinates
		if (CanMoveTo(this->wX + dx, this->wY + dy))
			return;  //move here
	}

	// We can't move towards the swordsman in the desired manner.
	// Try the four cardinal directions
	static const UINT directions[4] = {N, E, S, W};
	bool found = false;
	for (int i = 0; i < 4; ++i) 
	{
		dx = nGetOX(directions[i]);
		dy = nGetOY(directions[i]);
		// Don't backtrack
		if (dx == -oX && dy == -oY)
			continue;
		if (CanMoveTo(this->wX + dx, this->wY + dy))
		{
			found = true;
			break;
		}
	}
	if (!found)
		dx = dy = 0;   //stuck
}

//*****************************************************************************************
bool CSerpent::CanMoveTo(
//Ensure arrow constraints are enforced, even though red serpents can't move onto them.
//(The obstacle check will stop them from moving onto arrows.)
//
//Params:
	const int x, const int y)  //(in) proposed destination square
const
{
	bool bCanMove = !(
		  (UINT)x >= pCurrentGame->pRoom->wRoomCols ||
		  (UINT)y >= pCurrentGame->pRoom->wRoomRows ||
		  DoesSquareContainObstacle((UINT)x, (UINT)y));
	if (bCanMove)
		bCanMove = !DoesArrowPreventMovement(x - this->wX, y - this->wY);
	return bCanMove;
}

//*****************************************************************************************
void CSerpent::ReflectX(CDbRoom *pRoom)
{
	CMonster::ReflectX(pRoom);

	//Reverse tail.
	this->tailX = (pRoom->wRoomCols-1) - this->tailX;
	this->tailO = nGetO(-nGetOX(this->tailO),nGetOY(this->tailO));
}

//*****************************************************************************************
void CSerpent::ReflectY(CDbRoom *pRoom)
{
	CMonster::ReflectY(pRoom);

	//Reverse tail.
	this->tailY = (pRoom->wRoomRows-1) - this->tailY;
	this->tailO = nGetO(nGetOX(this->tailO),-nGetOY(this->tailO));
}

//*****************************************************************************************
void CSerpent::RotateClockwise(CDbRoom *pRoom)
{
	CMonster::RotateClockwise(pRoom);

	//Rotate tail.
	const UINT wNewX = (pRoom->wRoomRows-1) - this->tailX;
	this->tailY = this->tailX;
	this->tailX = wNewX;
	this->tailO = nNextCO(nNextCO(this->tailO));
}

//*****************************************************************************
void CSerpent::ReflectPieceX(CDbRoom *pRoom, CMonsterPiece *pPiece)
{
	CMonster::ReflectPieceX(pRoom, pPiece);
	switch (pPiece->wTileNo)
	{
		case T_SNK_NW: pPiece->wTileNo = T_SNK_NE; break;
		case T_SNK_SE: pPiece->wTileNo = T_SNK_SW; break;
		case T_SNK_NE: pPiece->wTileNo = T_SNK_NW; break;
		case T_SNK_SW: pPiece->wTileNo = T_SNK_SE; break;
		case T_SNKT_W: pPiece->wTileNo = T_SNKT_E; break;
		case T_SNKT_E: pPiece->wTileNo = T_SNKT_W; break;
	}
}

//*****************************************************************************
void CSerpent::ReflectPieceY(CDbRoom *pRoom, CMonsterPiece *pPiece)
{
	CMonster::ReflectPieceY(pRoom, pPiece);
	switch (pPiece->wTileNo)
	{
		case T_SNK_NW: pPiece->wTileNo = T_SNK_SW; break;
		case T_SNK_SE: pPiece->wTileNo = T_SNK_NE; break;
		case T_SNK_NE: pPiece->wTileNo = T_SNK_SE; break;
		case T_SNK_SW: pPiece->wTileNo = T_SNK_NW; break;
		case T_SNKT_S: pPiece->wTileNo = T_SNKT_N; break;
		case T_SNKT_N: pPiece->wTileNo = T_SNKT_S; break;
	}
}

//*****************************************************************************
void CSerpent::RotatePieceClockwise(CDbRoom *pRoom, CMonsterPiece *pPiece)
{
	CMonster::RotatePieceClockwise(pRoom, pPiece);
	switch (pPiece->wTileNo)
	{
		case T_SNK_NW: pPiece->wTileNo = T_SNK_NE; break;
		case T_SNK_NE: pPiece->wTileNo = T_SNK_SE; break;
		case T_SNK_SE: pPiece->wTileNo = T_SNK_SW; break;
		case T_SNK_SW: pPiece->wTileNo = T_SNK_NW; break;
		case T_SNKT_S: pPiece->wTileNo = T_SNKT_W; break;
		case T_SNKT_W: pPiece->wTileNo = T_SNKT_N; break;
		case T_SNKT_N: pPiece->wTileNo = T_SNKT_E; break;
		case T_SNKT_E: pPiece->wTileNo = T_SNKT_S; break;
		case T_SNK_EW: pPiece->wTileNo = T_SNK_NS; break;
		case T_SNK_NS: pPiece->wTileNo = T_SNK_EW; break;
	}
}
