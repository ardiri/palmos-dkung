/*
 * @(#)game.c
 *
 * Copyright 2000-2001, Aaron Ardiri (mailto:aaron@ardiri.com)
 * All rights reserved.
 *
 * This file was generated as part of the "dkung" program developed for 
 * the Palm Computing Platform designed by Palm: 
 *
 *   http://www.palm.com/ 
 *
 * The contents of this file is confidential and proprietrary in nature 
 * ("Confidential Information"). Redistribution or modification without 
 * prior consent of the original author is prohibited. 
 */

#include "palm.h"

// interface

static void GameAdjustLevel(PreferencesType *)                        __GAME__;
static void GameIncrementScore(PreferencesType *, UInt16)             __GAME__;
static void GameMovePlayer(PreferencesType *)                         __GAME__;
static void GameMoveObstacles(PreferencesType *)                      __GAME__;
static void GameBackupBitmapArea(WinHandle, WinHandle, WinHandle, 
                                 RectangleType *, RectangleType *)    __GAME__;

// global variable structure
typedef struct
{
  WinHandle winBackground;                  // background bitmap
  WinHandle winDigits;                      // scoring digits bitmaps

  WinHandle winLives;                       // the lives notification bitmaps

  WinHandle winIrons;                       // the iron bitmaps
  WinHandle winIronBackup[MAX_IRONS];       // the area behind the iron bitmap
  Boolean   ironChanged;                    // do we need to repaint irons?
  Boolean   ironOnScreen[MAX_IRONS];        // are iron bitmaps on screen?

  WinHandle winBars;                        // the bar bitmaps
  WinHandle winBarBackup;                   // the area behind the bar bitmap

  WinHandle winCranes;                      // the crane bitmaps
  Boolean   craneChanged;                   // do we need to repaint crane?
  Boolean   craneOnScreen;                  // is crane bitmap on screen?
  UInt16    craneOnScreenPosition;          // where? :)

  WinHandle winSwitches;                    // the switch bitmaps
  Boolean   switchChanged;                  // do we need to repaint switch?
  Boolean   switchOnScreen;                 // is switch bitmap on screen?
  UInt16    switchOnScreenPosition;         // where? :)

  WinHandle winKeys;                        // the key bitmaps
  Boolean   keyChanged;                     // do we need to repaint key?
  Boolean   keyOnScreen;                    // is key bitmap on screen?
  UInt16    keyOnScreenPosition;            // where? :)

  WinHandle winHooks;                       // the hook bitmaps
  WinHandle winHookBackup;                  // the area behind the hook bitmap
  Boolean   hookChanged;                    // do we need to repaint hook?
  Boolean   hookOnScreen;                   // is hook bitmap on screen?
  UInt16    hookOnScreenPosition;           // where? :)

  WinHandle winMarios;                      // the mario bitmaps
  WinHandle winMarioBackup;                 // the area behind the mario bitmap
  Boolean   marioChanged;                   // do we need to repaint mario?
  Boolean   marioOnScreen;                  // is mario bitmap on screen?
  UInt16    marioOnScreenPosition;          // where? :)

  WinHandle winKongs;                       // the kong bitmaps
  WinHandle winKongBackup;                  // the area behind the kong bitmap
  Boolean   kongChanged;                    // do we need to repaint kong?
  Boolean   kongOnScreen;                   // is kong bitmap on screen?
  UInt16    kongOnScreenPosition;           // where? :)

  WinHandle winBalls;                       // the ball bitmaps
  WinHandle winBallBackup[MAX_BALLS];       // the area behind the ball bitmap
  Boolean   ballChanged;                    // do we need to repaint balls?
  Boolean   ballOnScreen[MAX_BALLS];        // are ball bitmaps on screen?

  UInt8     gameType;                       // the type of game active
  Boolean   playerDied;                     // has the player died?
  UInt8     moveDelayCount;                 // the delay between moves
  UInt8     moveLast;                       // the last move performed
  UInt8     moveNext;                       // the next desired move

  UInt16    gameSpeed[GAME_count];          // game speeds :)

  struct {

    Boolean    gamePadPresent;              // is the gamepad driver present
    UInt16     gamePadLibRef;               // library reference for gamepad

  } hardware;

} GameGlobals;

/**
 * Initialize the Game.
 *
 * @return true if game is initialized, false otherwise
 */  
Boolean   
GameInitialize()
{
  GameGlobals *gbls;
  Err         err;
  Boolean     result;

  // create the globals object, and register it
  gbls = (GameGlobals *)MemPtrNew(sizeof(GameGlobals));
  MemSet(gbls, sizeof(GameGlobals), 0);
  FtrSet(appCreator, ftrGameGlobals, (UInt32)gbls);

  // load the gamepad driver if available
  {
    Err err;

    // attempt to load the library
    err = SysLibFind(GPD_LIB_NAME,&gbls->hardware.gamePadLibRef);
    if (err == sysErrLibNotFound)
      err = SysLibLoad('libr',GPD_LIB_CREATOR,&gbls->hardware.gamePadLibRef);

    // lets determine if it is available
    gbls->hardware.gamePadPresent = (err == errNone);

    // open the library if available
    if (gbls->hardware.gamePadPresent)
      GPDOpen(gbls->hardware.gamePadLibRef);
  }

  // configure the game speeds;
  gbls->gameSpeed[GAME_A] = 7;
  gbls->gameSpeed[GAME_B] = 6;
  gbls->gameSpeed[GAME_C] = 5;
  gbls->gameSpeed[GAME_D] = 4;
  gbls->gameSpeed[GAME_E] = 3;
  gbls->gameSpeed[GAME_F] = 2;

  // initialize our "bitmap" windows
  err = errNone;
  {
    Err   e;
    Int16 i;

    gbls->winBackground = 
      WinCreateOffscreenWindow(158, 183, screenFormat, &e); err |= e;

    gbls->winDigits = 
      WinCreateOffscreenWindow(70, 12, screenFormat, &e); err |= e;

    gbls->winLives = 
      WinCreateOffscreenWindow(33, 10, screenFormat, &e); err |= e;

    gbls->winIrons = 
      WinCreateOffscreenWindow(30, 7, screenFormat, &e); err |= e;
    MemSet(gbls->winIronBackup, sizeof(WinHandle) * MAX_IRONS, 0);
    {
      UInt16 backup[] = { 0, 0, 0, 1, 1 };

      // allocate memory for backup :)
      for (i=0; i<MAX_IRONS; i++) {
        gbls->winIronBackup[i] = (backup[i] == 1)
          ? WinCreateOffscreenWindow(15, 7, screenFormat, &e) 
          : NULL;
        gbls->ironOnScreen[i] = false;
        err |= e;
      }
    }
    gbls->ironChanged           = true;

    gbls->winBars = 
      WinCreateOffscreenWindow(98, 52, screenFormat, &e); err |= e;
    gbls->winBarBackup = 
      WinCreateOffscreenWindow(98, 26, screenFormat, &e); err |= e;

    gbls->winCranes = 
      WinCreateOffscreenWindow(90, 38, screenFormat, &e); err |= e;
    gbls->craneChanged          = true;
    gbls->craneOnScreen         = false;

    gbls->winSwitches = 
      WinCreateOffscreenWindow(12, 5, screenFormat, &e); err |= e;
    gbls->switchChanged         = true;
    gbls->switchOnScreen        = false;

    gbls->winKeys = 
      WinCreateOffscreenWindow(64, 11, screenFormat, &e); err |= e;
    gbls->keyChanged            = true;
    gbls->keyOnScreen           = false;

    gbls->winHooks = 
      WinCreateOffscreenWindow(60, 12, screenFormat, &e); err |= e;
    gbls->winHookBackup = 
      WinCreateOffscreenWindow(12, 12, screenFormat, &e); err |= e;
    gbls->hookChanged           = true;
    gbls->hookOnScreen          = false;

    gbls->winMarios = 
      WinCreateOffscreenWindow(160, 100, screenFormat, &e); err |= e;
    gbls->winMarioBackup = 
      WinCreateOffscreenWindow(20, 20, screenFormat, &e); err |= e;
    gbls->marioChanged          = true;
    gbls->marioOnScreen         = false;

    gbls->winKongs = 
      WinCreateOffscreenWindow(93, 72, screenFormat, &e); err |= e;
    gbls->winKongBackup = 
      WinCreateOffscreenWindow(31, 24, screenFormat, &e); err |= e;
    gbls->kongChanged           = true;
    gbls->kongOnScreen          = false;

    gbls->winBalls = 
      WinCreateOffscreenWindow(60, 56, screenFormat, &e); err |= e;
    MemSet(gbls->winBallBackup, sizeof(WinHandle) * MAX_BALLS, 0);
    {
      UInt16 backup[] = { 0, 1, 1, 0, 0, 0, 
                          1, 1, 1, 0, 0, 0,
                          1, 1, 1, 0, 0, 0,
                          0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0,
                          1, 0, 0, 0, 0, 0 };

      // allocate memory for backup :)
      for (i=0; i<MAX_BALLS; i++) {
        gbls->winBallBackup[i] = (backup[i] == 1)
          ? WinCreateOffscreenWindow(10, 8, screenFormat, &e) 
          : NULL;
        gbls->ballOnScreen[i] = false;
        err |= e;
      }
    }
    gbls->ballChanged           = true;
  }

  // no problems creating back buffers? load images.
  if (err == errNone) {
  
    WinHandle currWindow;
    MemHandle bitmapHandle;

    currWindow = WinGetDrawWindow();

    // background
    WinSetDrawWindow(gbls->winBackground);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapBackground);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // digits
    WinSetDrawWindow(gbls->winDigits);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapDigits);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // lives
    WinSetDrawWindow(gbls->winLives);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapLives);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // irons
    WinSetDrawWindow(gbls->winIrons);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapIrons);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // bars
    WinSetDrawWindow(gbls->winBars);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapBars);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // cranes
    WinSetDrawWindow(gbls->winCranes);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapCranes);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // switches
    WinSetDrawWindow(gbls->winSwitches);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapSwitches);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // keys
    WinSetDrawWindow(gbls->winKeys);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapKeys);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // hooks
    WinSetDrawWindow(gbls->winHooks);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapHooks);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // marios
    WinSetDrawWindow(gbls->winMarios);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapMarios);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // kongs
    WinSetDrawWindow(gbls->winKongs);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapKongs);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // balls
    WinSetDrawWindow(gbls->winBalls);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapBalls);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    WinSetDrawWindow(currWindow);
  }

  result = (err == errNone);

  return result;
}

/**
 * Reset the Game.
 * 
 * @param prefs the global preference data.
 * @param gameType the type of game to configure for.
 */  
void   
GameReset(PreferencesType *prefs, Int8 gameType)
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // turn off all the "bitmaps"
  FrmDrawForm(FrmGetActiveForm());
  {
    const CustomPatternType erase = { 0,0,0,0,0,0,0,0 };
    RectangleType rect    = { {   0,   0 }, {   0,   0 } };
    RectangleType scrRect = { {   0,   0 }, {   0,   0 } };

    // OUTLINE AREA
    if (!prefs->handera.device)
    {
      WinDrawLine(  0,  34, 159,  34);
      WinDrawLine(159,  34, 159, 140);
      WinDrawLine(159, 140,   0, 140);
      WinDrawLine(  0, 140,   0,  34);
    }
    else
    {
      WinDrawLine( 40,  35, 199,  35);
      WinDrawLine(199,  35, 199, 220);
      WinDrawLine(199, 220,  40, 220);
      WinDrawLine( 40, 220,  40,  35);
    }

    // BACKGROUND
 
    // what is the rectangle we need to copy?
    if (!prefs->handera.device)
    {
      scrRect.topLeft.x = 1;
      scrRect.topLeft.y = 36;
      scrRect.extent.x  = 158;
      scrRect.extent.y  = 103;
      rect.topLeft.x    = 0;
      switch (prefs->game.dkung.displaySetting)
      {
        case displayUpper: 
             rect.topLeft.y = 0;
             break;
  
        case displayMiddle: 
             rect.topLeft.y = 44;
             break;
  
        case displayLower: 
        default:
             rect.topLeft.y = 80;
             break;
      }
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;
    }
    else
    {
      scrRect.topLeft.x = 41;
      scrRect.topLeft.y = 37;
      scrRect.extent.x  = 158;
      scrRect.extent.y  = 183;
      rect.topLeft.x    = 0;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;
    }

    // draw the background!
    WinCopyRectangle(gbls->winBackground, WinGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);

    // SCORE AREA (must clear)
    if (!prefs->handera.device)
    {
      rect.topLeft.x    = 121;
      rect.topLeft.y    = 124;
      rect.extent.x     = 38;
      rect.extent.y     = 16;
    }
    else
    {
      rect.topLeft.x    = 160;
      rect.topLeft.y    = 204;
      rect.extent.x     = 38;
      rect.extent.y     = 16;
    }

    // clear the area where the score will be drawn
    WinSetPattern(&erase);
    WinFillRectangle(&rect, 0);
  }

  // turn on all the "bitmaps"
  {
    RectangleType clip    = { {   1,  36 }, { 159, 103 } };
    RectangleType rect    = { {   0,   0 }, {   0,   0 } };
    RectangleType scrRect = { {   0,   0 }, {   0,   0 } };
    UInt16        i;

    // set the clipping to the game area :)
    if (!prefs->handera.device)
      WinSetClip(&clip);

    //
    // draw the score
    //

    for (i=0; i<4; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs, 
                            spriteDigit, i, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 7;
      scrRect.extent.y  = 12;
      rect.topLeft.x    = 8 * scrRect.extent.x;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the digit!
      WinCopyRectangle(gbls->winDigits, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);
    }

    //
    // draw the lives
    //

    // what is the rectangle we need to copy?
    GameGetSpritePosition(prefs,
                          spriteLife, 0, prefs->game.dkung.displaySetting,
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

      scrRect.extent.x  = 33;
      scrRect.extent.y  = 10;
      rect.topLeft.x    = 0;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the life bitmap!
      WinCopyRectangle(gbls->winLives, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    //
    // draw the irons
    //  

    for (i=0; i<5; i++) {

      GameGetSpritePosition(prefs,
                            spriteIron, i, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 15;
        scrRect.extent.y  = 7;
        rect.topLeft.x    = (i % 2) * scrRect.extent.x;
        rect.topLeft.y    = 0;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // draw the iron bitmap!
        WinCopyRectangle(gbls->winIrons, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }

    //
    // draw the bars
    //  

    for (i=0; i<2; i++) {

      GameGetSpritePosition(prefs,
                            spriteBar, i, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 98;
        scrRect.extent.y  = 26;
        rect.topLeft.x    = 0;
        rect.topLeft.y    = i * scrRect.extent.y;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // draw the bar bitmap!
        WinCopyRectangle(gbls->winBars, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }

    //
    // draw the cranes
    //  

    for (i=0; i<3; i++) {

      GameGetSpritePosition(prefs, 
                            spriteCrane, i, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 30;
        scrRect.extent.y  = 38;
        rect.topLeft.x    = i * scrRect.extent.x;
        rect.topLeft.y    = 0;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // draw the crane bitmap!
        WinCopyRectangle(gbls->winCranes, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }

    //
    // draw the switches
    //  

    for (i=0; i<2; i++) {

      GameGetSpritePosition(prefs,
                            spriteSwitch, i, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 6;
        scrRect.extent.y  = 5;
        rect.topLeft.x    = i * scrRect.extent.x;
        rect.topLeft.y    = 0;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // draw the switch bitmap!
        WinCopyRectangle(gbls->winSwitches, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }

    // 
    // draw the keys
    //

    GameGetSpritePosition(prefs,
                          spriteKey, 0, prefs->game.dkung.displaySetting,
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

      scrRect.extent.x  = 16;
      scrRect.extent.y  = 11;
      rect.topLeft.x    = 0;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the key bitmap!
      WinCopyRectangle(gbls->winKeys, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    // 
    // draw the hooks
    //

    for (i=0; i<5; i++) {

      GameGetSpritePosition(prefs,
                            spriteHook, i, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 12;
        scrRect.extent.y  = 12;
        rect.topLeft.x    = i * scrRect.extent.x;
        rect.topLeft.y    = 0;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // draw the hook bitmap!
        WinCopyRectangle(gbls->winHooks, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }

    // 
    // draw the mario's
    //

    for (i=0; i<40; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs,
                            spriteMario, 
                            (i != 23)?i:i-1, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 20;
        scrRect.extent.y  = 20;
        rect.topLeft.x    = (i % 8) * scrRect.extent.x; 
        rect.topLeft.y    = (i / 8) * scrRect.extent.y; 
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // draw the mario bitmap!
        WinCopyRectangle(gbls->winMarios, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }

    // 
    // draw the kongs 
    //

    for (i=0; i<9; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs,
                            spriteKong, i, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 31;
        scrRect.extent.y  = 24;
        rect.topLeft.x    = (i % 3) * scrRect.extent.x; 
        rect.topLeft.y    = (i / 3) * scrRect.extent.y; 
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // draw the kong bitmap!
        WinCopyRectangle(gbls->winKongs, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }

    // 
    // draw the balls 
    //

    for (i=0; i<42; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs,
                            spriteBall, i, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 10;
        scrRect.extent.y  = 8;
        rect.topLeft.x    = (i % 6) * scrRect.extent.x; 
        rect.topLeft.y    = (i / 6) * scrRect.extent.y; 
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // draw the ball bitmap!
        WinCopyRectangle(gbls->winBalls, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }

    // reset the clipping area :)
    if (!prefs->handera.device)
      WinResetClip();
  }

  // wait a good two seconds :))
  SysTaskDelay(2 * SysTicksPerSecond());

  // turn off all the "bitmaps"
  FrmDrawForm(FrmGetActiveForm());
  {
    const CustomPatternType erase = { 0,0,0,0,0,0,0,0 };
    RectangleType rect    = { {   0,   0 }, {   0,   0 } };
    RectangleType scrRect = { {   0,   0 }, {   0,   0 } };

    // OUTLINE AREA
    if (!prefs->handera.device)
    {
      WinDrawLine(  0,  34, 159,  34);
      WinDrawLine(159,  34, 159, 140);
      WinDrawLine(159, 140,   0, 140);
      WinDrawLine(  0, 140,   0,  34);
    }
    else
    {
      WinDrawLine( 40,  35, 199,  35);
      WinDrawLine(199,  35, 199, 220);
      WinDrawLine(199, 220,  40, 220);
      WinDrawLine( 40, 220,  40,  35);
    }

    // BACKGROUND
 
    // what is the rectangle we need to copy?
    if (!prefs->handera.device)
    {
      scrRect.topLeft.x = 1;
      scrRect.topLeft.y = 36;
      scrRect.extent.x  = 158;
      scrRect.extent.y  = 103;
      rect.topLeft.x    = 0;
      switch (prefs->game.dkung.displaySetting)
      {
        case displayUpper: 
             rect.topLeft.y = 0;
             break;
  
        case displayMiddle: 
             rect.topLeft.y = 44;
             break;
  
        case displayLower: 
        default:
             rect.topLeft.y = 80;
             break;
      }
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;
    }
    else
    {
      scrRect.topLeft.x = 41;
      scrRect.topLeft.y = 37;
      scrRect.extent.x  = 158;
      scrRect.extent.y  = 183;
      rect.topLeft.x    = 0;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;
    }

    // draw the digit!
    WinCopyRectangle(gbls->winBackground, WinGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);

    // SCORE AREA (must clear)
    if (!prefs->handera.device)
    {
      rect.topLeft.x    = 121;
      rect.topLeft.y    = 124;
      rect.extent.x     = 38;
      rect.extent.y     = 16;
    }
    else
    {
      rect.topLeft.x    = 160;
      rect.topLeft.y    = 204;
      rect.extent.x     = 38;
      rect.extent.y     = 16;
    }

    // clear the area where the score will be drawn
    WinSetPattern(&erase);
    WinFillRectangle(&rect, 0);
  }

  // reset the preferences
  GameResetPreferences(prefs, gameType);
}

/**
 * Reset the Game preferences.
 * 
 * @param prefs the global preference data.
 * @param gameType the type of game to configure for.
 */  
void   
GameResetPreferences(PreferencesType *prefs, Int8 gameType)
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // now we are playing
  prefs->game.gamePlaying            = true;
  prefs->game.gamePaused             = false;
  prefs->game.gameWait               = true;
  prefs->game.gameAnimationCount     = 0;

  // reset score and lives
  prefs->game.gameScore              = 0;
  prefs->game.gameLives              = 3;

  // reset dkung specific things
  prefs->game.dkung.gameType         = gameType;
  prefs->game.dkung.bonusAvailable   = true;
  prefs->game.dkung.bonusScoring     = false;

  prefs->game.dkung.displaySetting   = displayLower;
  prefs->game.dkung.displayChanged   = true;

  prefs->game.dkung.cranePosition    = 0;

  prefs->game.dkung.switchPosition   = 0;

  prefs->game.dkung.keyCount         = 4;

  prefs->game.dkung.hookHide         = true; 
  prefs->game.dkung.hookDirection    = 1;
  prefs->game.dkung.hookPosition     = 0;
  prefs->game.dkung.hookWait         = 0;

  prefs->game.dkung.marioCount       = 0;
  prefs->game.dkung.marioPosition    = 0;
  prefs->game.dkung.marioNewPosition = 0;
  prefs->game.dkung.marioJumpWait    = 0;

  prefs->game.dkung.kongPosition     = 0;
  prefs->game.dkung.kongWait         = gbls->gameSpeed[gbls->gameType];

  MemSet(prefs->game.dkung.ballPosition, MAX_BALLS * sizeof(Boolean), 0);
  prefs->game.dkung.ballPosition[8]  = true;
  prefs->game.dkung.ballPosition[28] = true;
  prefs->game.dkung.ballPosition[31] = true;
  prefs->game.dkung.ballWait         = gbls->gameSpeed[gbls->gameType];

  MemSet(prefs->game.dkung.ironPosition, MAX_IRONS * sizeof(Boolean), 0);
  prefs->game.dkung.ironPosition[0]  = (gameType == GAME_A) ? false : true;  
  prefs->game.dkung.ironWait         = gbls->gameSpeed[gbls->gameType] + 2;

  gbls->gameType                     = gameType;
  gbls->playerDied                   = false;
  gbls->moveDelayCount               = 0;
  gbls->moveLast                     = moveNone;
  gbls->moveNext                     = moveNone;
}

/**
 * Process key input from the user.
 * 
 * @param prefs the global preference data.
 * @param keyStatus the current key state.
 */  
void   
GameProcessKeyInput(PreferencesType *prefs, UInt32 keyStatus)
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  keyStatus &= (prefs->config.ctlKeyJump  |
                prefs->config.ctlKeyUp    |
                prefs->config.ctlKeyDown  |
                prefs->config.ctlKeyLeft  |
                prefs->config.ctlKeyRight);

  // additonal checks here
  if (gbls->hardware.gamePadPresent) {

    UInt8 gamePadKeyStatus;
    Err   err;

    // read the state of the gamepad
    err = GPDReadInstant(gbls->hardware.gamePadLibRef, &gamePadKeyStatus);
    if (err == errNone) {

      // process
      if (((gamePadKeyStatus & GAMEPAD_RIGHTFIRE) != 0) ||
          ((gamePadKeyStatus & GAMEPAD_LEFTFIRE)  != 0))
        keyStatus |= prefs->config.ctlKeyJump;
      if  ((gamePadKeyStatus & GAMEPAD_DOWN)      != 0) 
        keyStatus |= prefs->config.ctlKeyDown;
      if  ((gamePadKeyStatus & GAMEPAD_UP)        != 0) 
        keyStatus |= prefs->config.ctlKeyUp;
      if  ((gamePadKeyStatus & GAMEPAD_LEFT)      != 0) 
        keyStatus |= prefs->config.ctlKeyLeft;
      if  ((gamePadKeyStatus & GAMEPAD_RIGHT)     != 0) 
        keyStatus |= prefs->config.ctlKeyRight;

      // special purpose :)
      if  ((gamePadKeyStatus & GAMEPAD_SELECT)    != 0) {

	// wait until they let it go :) 
	do {
          GPDReadInstant(gbls->hardware.gamePadLibRef, &gamePadKeyStatus);
        } while ((gamePadKeyStatus & GAMEPAD_SELECT) != 0);

        keyStatus = 0;
	prefs->game.gamePaused = !prefs->game.gamePaused;
      }
      if  ((gamePadKeyStatus & GAMEPAD_START)     != 0) {
	
	// wait until they let it go :) 
	do {
          GPDReadInstant(gbls->hardware.gamePadLibRef, &gamePadKeyStatus);
        } while ((gamePadKeyStatus & GAMEPAD_START) != 0);

        keyStatus = 0;
        GameReset(prefs, prefs->game.dkung.gameType);
      }
    }
  }

  // did they press at least one of the game keys?
  if (keyStatus != 0) {

    // if they were waiting, we should reset the game animation count
    if (prefs->game.gameWait) { 
      prefs->game.gameAnimationCount = 0;
      prefs->game.gameWait           = false;
    }

    // great! they wanna play
    prefs->game.gamePaused = false;
  }

  // jump / jump for key
  if (
      ((keyStatus &  prefs->config.ctlKeyJump) != 0) &&
      (
       (gbls->moveDelayCount == 0) || 
       (gbls->moveLast       != moveJump)
      ) &&
      ((prefs->game.dkung.marioPosition % 2) == 0) && // on floor
      (prefs->game.dkung.marioPosition !=  8) &&
      (prefs->game.dkung.marioPosition != 18) &&
      (prefs->game.dkung.marioPosition != 19) &&
      (prefs->game.dkung.marioPosition != 20) &&      // ladder climb pos :)
      (prefs->game.dkung.marioPosition != 22)         // flick switch pos :)
     ) { 
    
    // adjust the position if possible
    switch (prefs->game.dkung.marioPosition)
    {
      // jump locations
      case  0: case  6: case 12: case 14: case 26:

           prefs->game.dkung.marioNewPosition = 
             (prefs->game.dkung.marioPosition + 1);

           break;

      default:
           break;
    }
  }
 
  // flick the switch?
  else
  if (
      (
       (
        ((keyStatus &  prefs->config.ctlKeyJump) != 0) &&
        (
         (gbls->moveDelayCount == 0) || 
         (gbls->moveLast       != moveJump)
        )
       ) ||
       (
        ((keyStatus &    prefs->config.ctlKeyLeft) != 0) && 
        (
         (gbls->moveDelayCount == 0) || 
         (gbls->moveLast       != moveLeft)
        )
       )
      ) &&
      (prefs->game.dkung.marioPosition == 22)
     ) {

    // special case, flick switch
    prefs->game.dkung.marioNewPosition = 23;
  }

  // move left
  else
  if (
      ((keyStatus &  prefs->config.ctlKeyLeft) != 0) &&
      (
       (gbls->moveDelayCount == 0) || 
       (gbls->moveLast       != moveLeft)
      )
     ) {

    // adjust the position if possible
    switch (prefs->game.dkung.marioPosition)
    {
      // incremental
      case 10: case 12: case 14: case 16:

           prefs->game.dkung.marioNewPosition = 
             (prefs->game.dkung.marioPosition + 2);

           break;

      // decremental
      case  2: case  4: case  6: case  8:
      case 24: case 26:

           prefs->game.dkung.marioNewPosition = 
             (prefs->game.dkung.marioPosition - 2);

           break;

      default:
           break;
    }
  }

  // move right
  else
  if (
      ((keyStatus & prefs->config.ctlKeyRight) != 0) &&
      (
       (gbls->moveDelayCount == 0) || 
       (gbls->moveLast       != moveRight)
      )
     ) {

    // adjust the position if possible
    switch (prefs->game.dkung.marioPosition)
    {
      // incremental
      case 0:  case  2: case  4: case  6:
      case 22: case 24: 

           prefs->game.dkung.marioNewPosition = 
             (prefs->game.dkung.marioPosition + 2);

           break;

      // fall off edge :)
      case 26:  

           prefs->game.dkung.marioJumpWait =
             gbls->gameSpeed[gbls->gameType] - 2;
           prefs->game.dkung.marioPosition = 36;
           gbls->marioChanged              = true;

           break;

      // decremental
      case 12: case 14: case 16: case 18:

           prefs->game.dkung.marioNewPosition = 
             (prefs->game.dkung.marioPosition - 2);

           break;

      default:
           break;
    }
  }

  // move up
  else
  if (
      (
       (
        ((keyStatus &  prefs->config.ctlKeyJump) != 0) &&
        (
         (gbls->moveDelayCount == 0) || 
         (gbls->moveLast       != moveJump)
        )
       ) ||
       (
        ((keyStatus &    prefs->config.ctlKeyUp) != 0) && 
        (
         (gbls->moveDelayCount == 0) || 
         (gbls->moveLast       != moveUp)
        )
       )
      )
     ) {

    // adjust the position if possible
    switch (prefs->game.dkung.marioPosition)
    {
      // lowest ladder 
      case  8:

           prefs->game.dkung.marioNewPosition = 10; 
           break;

      // upper ladder 
      case 18:

           prefs->game.dkung.marioNewPosition = 19;
           break;

      case 19:

           prefs->game.dkung.marioNewPosition = 20;
           break;

      case 20:

           prefs->game.dkung.marioNewPosition = 22; 
           break;

      default:
           break;
    }
  }

  // move down
  else
  if (
      ((keyStatus &  prefs->config.ctlKeyDown) != 0) &&
      (
       (gbls->moveDelayCount == 0) || 
       (gbls->moveLast       != moveDown)
      )
     ) {

    // adjust the position if possible
    switch (prefs->game.dkung.marioPosition)
    {
      // lowest ladder 
      case 10:

           prefs->game.dkung.marioNewPosition = 8; 
           break;

      // upper ladder 
      case 19:

           prefs->game.dkung.marioNewPosition = 18;
           break;

      case 20:

           prefs->game.dkung.marioNewPosition = 19;
           break;

      case 22:

           prefs->game.dkung.marioNewPosition = 20; 
           break;

      default:
           break;
    }
  }
}
  
/**
 * Process stylus input from the user.
 * 
 * @param prefs the global preference data.
 * @param x the x co-ordinate of the stylus event.
 * @param y the y co-ordinate of the stylus event.
 */  
void   
GameProcessStylusInput(PreferencesType *prefs, Coord x, Coord y)
{
  RectangleType rect;
  UInt16        i;

  // lets take a look at all the possible "positions"
  for (i=0; i<28; i++) {

    // get the bounding box of the position
    GameGetSpritePosition(prefs,
                          spriteMario, i, prefs->game.dkung.displaySetting,
                          &rect.topLeft.x, &rect.topLeft.y);
    rect.extent.x  = 20;
    rect.extent.y  = 20;

    // did they tap inside this rectangle?
    if ((rect.topLeft.x != -1) && (rect.topLeft.y != -1) &&
        (RctPtInRectangle(x, y, &rect))) {

      // ok, this is where we are going to go :)
      prefs->game.dkung.marioNewPosition = i;

      // if they were waiting, we should reset the game animation count
      if (prefs->game.gameWait) {
        prefs->game.gameAnimationCount = 0;
        prefs->game.gameWait           = false;
      }

      // great! they wanna play
      prefs->game.gamePaused = false;
      break;                                        // stop looking
    }
  }
}

/**
 * Process the object movement in the game.
 * 
 * @param prefs the global preference data.
 */  
void   
GameMovement(PreferencesType *prefs)
{
  const CustomPatternType erase = { 0,0,0,0,0,0,0,0 };
        RectangleType     rect  = {{   0,  16 }, { 160, 16 }};

  GameGlobals    *gbls;
  SndCommandType missSnd  = {sndCmdFreqDurationAmp,0, 512,250,sndMaxAmp};
  SndCommandType deathSnd = {sndCmdFreqDurationAmp,0, 768, 50,sndMaxAmp};

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  //
  // the game is NOT paused.
  //

  if (!prefs->game.gamePaused) {

    // we must make sure the user is ready for playing 
    if (!prefs->game.gameWait) {

      // we cannot be dead yet :)
      gbls->playerDied = false;

      // if the hook is visible (and mario still in play), animate it
      if ((!prefs->game.dkung.hookHide) &&
          ((prefs->game.dkung.marioPosition <= 27) ||
           (prefs->game.dkung.marioPosition >= 36))) {

        // can we move it?
        if (prefs->game.dkung.hookWait == 0) {

          // hook swinging has come to an end :((
          if ((prefs->game.dkung.hookPosition  == 4) &&
              (prefs->game.dkung.hookDirection == 3)) {

            // back to normal :)
            prefs->game.dkung.hookHide       = true;
            prefs->game.dkung.cranePosition  = 0;
            prefs->game.dkung.switchPosition = 0;

            gbls->hookChanged                = true;
            gbls->craneChanged               = true;
            gbls->switchChanged              = true;
          }

          // adjust the hook position :)
          else {

            Int16 offset = 0;

            // adjust direction?
            if ((prefs->game.dkung.hookPosition  == 0) &&
                (prefs->game.dkung.hookDirection == 2)) 
              prefs->game.dkung.hookDirection = 3;
            if ((prefs->game.dkung.hookPosition  == 4) &&
                (prefs->game.dkung.hookDirection == 1)) 
              prefs->game.dkung.hookDirection = 2;


            // adjust the position
            switch (prefs->game.dkung.hookDirection)
            {
              case 1: 
              case 3: 
                      offset = 1;
                      break;

              case 2:
                      offset = -1;
                      break;

              default:
                      break;
            } 

            prefs->game.dkung.hookPosition += offset;
            prefs->game.dkung.hookWait      = 3; 
            gbls->hookChanged               = true;
          }
        }
        else
          prefs->game.dkung.hookWait--;
      }

      // are we in bonus mode?
      if ((prefs->game.dkung.bonusScoring) &&
          (prefs->game.gameAnimationCount % GAME_FPS) < (GAME_FPS >> 1)) {

        Char   str[32];
        FontID currFont = FntGetFont();

        StrCopy(str, "    * BONUS PLAY *    ");
        FntSetFont(boldFont);
        WinDrawChars(str, StrLen(str),
                     80 - (FntCharsWidth(str, StrLen(str)) >> 1), 19);
        FntSetFont(currFont);
      }
      else {

        // tweak for handera
        if (prefs->handera.device)
        {
          rect.topLeft.y = 23;
          rect.extent.x  = 240;
          rect.extent.y  = 12;
        }

        // erase the status area
        WinSetPattern(&erase);
        WinFillRectangle(&rect, 0);
      }

      // player gets first move
      GameMovePlayer(prefs);

      // enemies move next
      GameMoveObstacles(prefs);

      // is it time to upgrade the game?
      if (prefs->game.gameAnimationCount >= 0x01ff) {

        // reset the counter
        prefs->game.gameAnimationCount = 0;

        // adjust the game accordiningly
        switch (gbls->gameType)
        {
          case GAME_A:
               gbls->gameType = GAME_B;
               break;

          case GAME_B:
               gbls->gameType = GAME_C;
               break;

          case GAME_C:
               gbls->gameType = GAME_D;
               break;

          case GAME_D:
               gbls->gameType = GAME_E;
               break;

          case GAME_E:
               gbls->gameType = GAME_F;
               break;

          case GAME_F:  // they are good :) see how long they last :P
          default:
               break;
        }
      } 

      // has the player died in this frame?
      if (gbls->playerDied) {

        UInt16        i, index;
        RectangleType rect    = { {   0,   0 }, {   0,   0 } };
        RectangleType scrRect = { {   0,   0 }, {   0,   0 } };

        // force a complete redraw of the screen
        GameDraw(prefs);

        // play death sound and flash the player (if we should)
        for (i=0; ((prefs->game.dkung.marioPosition < 32) && (i<2)); i++) {

          index = prefs->game.dkung.marioPosition;

          //
          // draw mario
          //

          if (gbls->marioOnScreen) {

            index = gbls->marioOnScreenPosition;

            // what is the rectangle we need to copy?
            GameGetSpritePosition(prefs,
                                  spriteMario, index, prefs->game.dkung.displaySetting,
                                  &scrRect.topLeft.x, &scrRect.topLeft.y);
            if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {
  
              scrRect.extent.x  = 20;
              scrRect.extent.y  = 20;
              rect.topLeft.x    = (index % 8) * scrRect.extent.x; 
              rect.topLeft.y    = (index / 8) * scrRect.extent.y; 
              rect.extent.x     = scrRect.extent.x;
              rect.extent.y     = scrRect.extent.y;

              // invert the old mario bitmap
              WinCopyRectangle(gbls->winMarios, WinGetDrawWindow(),
                               &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
              gbls->marioOnScreen = false;
            }

            // restore the background area
            rect.topLeft.x    = 0;
            rect.topLeft.y    = 0;
            rect.extent.x     = scrRect.extent.x;
            rect.extent.y     = scrRect.extent.y;
            WinCopyRectangle(gbls->winMarioBackup, WinGetDrawWindow(),
                             &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
          }

          // what is the rectangle we need to copy?
          GameGetSpritePosition(prefs,
                                spriteMario, index, prefs->game.dkung.displaySetting,
                                &scrRect.topLeft.x, &scrRect.topLeft.y);
          if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

            scrRect.extent.x  = 20;
            scrRect.extent.y  = 20;
            rect.topLeft.x    = (index % 8) * scrRect.extent.x;
            rect.topLeft.y    = (index / 8) * scrRect.extent.y;
            rect.extent.x     = scrRect.extent.x;
            rect.extent.y     = scrRect.extent.y;
  
            // save this location and backup area, record mario is onscreen
            gbls->marioOnScreen         = true;
            gbls->marioOnScreenPosition = index;
            GameBackupBitmapArea(WinGetDrawWindow(), 
                                 gbls->winMarios, gbls->winMarioBackup,
                                 &scrRect, &rect); 

            // draw the mario bitmap!
            WinCopyRectangle(gbls->winMarios, WinGetDrawWindow(),
                             &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
          }

          // first beep = crash sound :P
          if (i == 0) {
            DevicePlaySound(&missSnd);
          }
          else
            DevicePlaySound(&deathSnd);
          SysTaskDelay(40);

          //
          // erase mario
          //

          index = gbls->marioOnScreenPosition;

          // what is the rectangle we need to copy?
          GameGetSpritePosition(prefs,
                                spriteMario, index, prefs->game.dkung.displaySetting,
                                &scrRect.topLeft.x, &scrRect.topLeft.y);
          if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

            scrRect.extent.x  = 20;
            scrRect.extent.y  = 20;
            rect.topLeft.x    = (index % 8) * scrRect.extent.x; 
            rect.topLeft.y    = (index / 8) * scrRect.extent.y; 
            rect.extent.x     = scrRect.extent.x;
            rect.extent.y     = scrRect.extent.y;

            // invert the old mario bitmap
            WinCopyRectangle(gbls->winMarios, WinGetDrawWindow(),
                             &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
            gbls->marioOnScreen = false;
          }

          // restore the background area
          rect.topLeft.x    = 0;
          rect.topLeft.y    = 0;
          rect.extent.x     = scrRect.extent.x;
          rect.extent.y     = scrRect.extent.y;
          WinCopyRectangle(gbls->winMarioBackup, WinGetDrawWindow(),
                           &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);

          // play the beep sound
          DevicePlaySound(&deathSnd);
          SysTaskDelay(40);
        }

        // lose a life :(
        prefs->game.gameLives--;

        // no more lives left: GAME OVER!!
        if (prefs->game.gameLives == 0) {

          EventType event;
          SndCommandType deathSnd1 
            = {sndCmdFreqDurationAmp,0,4096,1,sndMaxAmp};
          SndCommandType deathSnd2 
            = {sndCmdFreqDurationAmp,0, 512,1,sndMaxAmp};

          // play that little "tweet" sound :)
          for (i=0; i<8; i++) {
            DevicePlaySound(&deathSnd1);
            SysTaskDelay(1);
            DevicePlaySound(&deathSnd2);
            SysTaskDelay(1);
          }

          // return to main screen
          MemSet(&event, sizeof(EventType), 0);
          event.eType            = menuEvent;
          event.data.menu.itemID = gameMenuItemExit;
          EvtAddEventToQueue(&event);

          prefs->game.gamePlaying = false;
        }

        // reset player position and continue game
        else {
          GameAdjustLevel(prefs);
          prefs->game.dkung.bonusScoring = false;
          prefs->game.gameWait           = true;
        }
      }
    }

    // we have to display "GET READY!"
    else {

      // flash on:
      if ((prefs->game.gameAnimationCount % GAME_FPS) < (GAME_FPS >> 1)) {

        Char   str[32];
        FontID currFont = FntGetFont();

        StrCopy(str, "    * GET READY *    ");
        FntSetFont(boldFont);
        if (!prefs->handera.device)
          WinDrawChars(str, StrLen(str), 
                       80 - (FntCharsWidth(str, StrLen(str)) >> 1), 19);

        else
          WinDrawChars(str, StrLen(str), 
                       120 - (FntCharsWidth(str, StrLen(str)) >> 1), 24);

        FntSetFont(currFont);
      }

      // flash off:
      else {

        // tweak for handera
        if (prefs->handera.device)
        {
          rect.topLeft.y = 23;
          rect.extent.x  = 240;
          rect.extent.y  = 12;
        }

        // erase the status area
        WinSetPattern(&erase);
        WinFillRectangle(&rect, 0);
      }
    }

    // update the animation counter
    prefs->game.gameAnimationCount++;
  }

  //
  // the game is paused.
  //

  else {

    Char   str[32];
    FontID currFont = FntGetFont();

    StrCopy(str, "    *  PAUSED  *    ");
    FntSetFont(boldFont);
    if (!prefs->handera.device)
      WinDrawChars(str, StrLen(str), 
                   80 - (FntCharsWidth(str, StrLen(str)) >> 1), 19);
    else
      WinDrawChars(str, StrLen(str), 
                   120 - (FntCharsWidth(str, StrLen(str)) >> 1), 24);
    FntSetFont(currFont);
  }
}

/**
 * Draw the game on the screen.
 * 
 * @param prefs the global preference data.
 */
void   
GameDraw(PreferencesType *prefs)
{
  GameGlobals   *gbls;
  UInt16        i, index;
  RectangleType clip    = { {   1,  36 }, { 159, 103 } };
  RectangleType rect    = { {   0,   0 }, {   0,   0 } };
  RectangleType scrRect = { {   0,   0 }, {   0,   0 } };

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  //
  // DRAW INFORMATION/BITMAPS ON SCREEN
  //

  // background image (if needed)
  if (prefs->game.dkung.displayChanged) {

    const CustomPatternType erase = { 0,0,0,0,0,0,0,0 };

    // OUTLINE AREA
    if (!prefs->handera.device)
    {
      WinDrawLine(  0,  34, 159,  34);
      WinDrawLine(159,  34, 159, 140);
      WinDrawLine(159, 140,   0, 140);
      WinDrawLine(  0, 140,   0,  34);
    }
    else
    {
      WinDrawLine( 40,  35, 199,  35);
      WinDrawLine(199,  35, 199, 220);
      WinDrawLine(199, 220,  40, 220);
      WinDrawLine( 40, 220,  40,  35);
    }

    // BACKGROUND
 
    // what is the rectangle we need to copy?
    if (!prefs->handera.device)
    {
      scrRect.topLeft.x = 1;
      scrRect.topLeft.y = 36;
      scrRect.extent.x  = 158;
      scrRect.extent.y  = 103;
      rect.topLeft.x    = 0;
      switch (prefs->game.dkung.displaySetting)
      {
        case displayUpper: 
             rect.topLeft.y = 0;
             break;
  
        case displayMiddle: 
             rect.topLeft.y = 44;
             break;
  
        case displayLower: 
        default:
             rect.topLeft.y = 80;
             break;
      }
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;
    }
    else
    {
      scrRect.topLeft.x = 41;
      scrRect.topLeft.y = 37;
      scrRect.extent.x  = 158;
      scrRect.extent.y  = 183;
      rect.topLeft.x    = 0;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;
    }
 
    // draw the background!
    WinCopyRectangle(gbls->winBackground, WinGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);

    // SCORE AREA (must clear)
    if (!prefs->handera.device)
    {
      rect.topLeft.x    = 121;
      rect.topLeft.y    = 124;
      rect.extent.x     = 38;
      rect.extent.y     = 16;
    }
    else
    {
      rect.topLeft.x    = 160;
      rect.topLeft.y    = 204;
      rect.extent.x     = 38;
      rect.extent.y     = 16;
    }

    // clear the area where the score will be drawn
    WinSetPattern(&erase);
    WinFillRectangle(&rect, 0);

    prefs->game.dkung.displayChanged = false;

    // draw the lives bitmap(s) - if on lower level only
    if (prefs->handera.device ||
        ((prefs->game.dkung.displaySetting == displayLower) &&
         (prefs->game.gameLives > 1))) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs,
                            spriteLife, 0, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 11 * (prefs->game.gameLives - 1);
        scrRect.extent.y  = 10;
        rect.topLeft.x    = 0;
        rect.topLeft.y    = 0;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // draw the life bitmap!
        WinCopyRectangle(gbls->winLives, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }

    // draw the "platform" that kong is standing on :)
    if (prefs->handera.device ||
        (prefs->game.dkung.displaySetting == displayUpper)) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs,
                            spriteBar, 0, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 98;
        scrRect.extent.y  = 26;
        rect.topLeft.x    = 0;
        rect.topLeft.y    = 0;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // draw the bar bitmap!
        WinCopyRectangle(gbls->winBars, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }

    // reset *everything*
    gbls->craneChanged   = true;
    gbls->craneOnScreen  = false;
    gbls->switchChanged  = true;
    gbls->switchOnScreen = false;
    gbls->keyChanged     = true;
    gbls->keyOnScreen    = false;
    gbls->hookChanged    = true;
    gbls->hookOnScreen   = false;
    gbls->kongChanged    = true;
    gbls->kongOnScreen   = false;
    gbls->ballChanged    = true;
    for (i=0; i<MAX_BALLS; i++) 
      gbls->ballOnScreen[i] = false;
    gbls->ironChanged    = true;
    for (i=0; i<MAX_IRONS; i++) 
      gbls->ironOnScreen[i] = false;
    gbls->marioChanged   = true;
    gbls->marioOnScreen  = false;
  }

  // set the clipping to the game area :)
  if (!prefs->handera.device)
    WinSetClip(&clip);

  // draw the score
  {
    UInt16 base;

    base = 1000;  // max score (4 digits)
    for (i=0; i<4; i++) {

      index = (prefs->game.gameScore / base) % 10;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs,
                            spriteDigit, i, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 7;
      scrRect.extent.y  = 12;
      rect.topLeft.x    = index * scrRect.extent.x;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the digit!
      WinCopyRectangle(gbls->winDigits, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);
      base /= 10;
    }
  }

  // draw crane (only if required)
  if (gbls->craneChanged) {

    //
    // erase the previous crane
    //

    if (gbls->craneOnScreen) {

      index = gbls->craneOnScreenPosition;

      GameGetSpritePosition(prefs,
                            spriteCrane, index, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 30;
        scrRect.extent.y  = 38;
        rect.topLeft.x    = index * scrRect.extent.x;
        rect.topLeft.y    = 0;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // invert the old crane bitmap!
        WinCopyRectangle(gbls->winCranes, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
        gbls->craneOnScreen = false;
      }
    }

    // 
    // draw the crane at the right position
    //

    index = prefs->game.dkung.cranePosition;

    GameGetSpritePosition(prefs,
                          spriteCrane, index, prefs->game.dkung.displaySetting,
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

      scrRect.extent.x  = 30;
      scrRect.extent.y  = 38;
      rect.topLeft.x    = index * scrRect.extent.x;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // save this location, record crane is on screen
      gbls->craneOnScreen         = true;
      gbls->craneOnScreenPosition = index;

      // draw old crane bitmap!
      WinCopyRectangle(gbls->winCranes, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    // dont draw until we need to
    gbls->craneChanged = false;
  }

  // draw switch (only if required)
  if (gbls->switchChanged) {

    //
    // erase the previous switch
    //

    if (gbls->switchOnScreen) {

      index = gbls->switchOnScreenPosition;

      GameGetSpritePosition(prefs,
                            spriteSwitch, index, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 6;
        scrRect.extent.y  = 5;
        rect.topLeft.x    = index * scrRect.extent.x;
        rect.topLeft.y    = 0;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // invert the old switch bitmap!
        WinCopyRectangle(gbls->winSwitches, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
        gbls->switchOnScreen = false;
      }
    }

    // 
    // draw the switch at the right position
    //

    index = prefs->game.dkung.switchPosition;

    GameGetSpritePosition(prefs,
                          spriteSwitch, index, prefs->game.dkung.displaySetting,
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

      scrRect.extent.x  = 6;
      scrRect.extent.y  = 5;
      rect.topLeft.x    = index * scrRect.extent.x;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // save this location, record switch is on screen
      gbls->switchOnScreen         = true;
      gbls->switchOnScreenPosition = index;

      // draw the switch bitmap!
      WinCopyRectangle(gbls->winSwitches, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    // dont draw until we need to
    gbls->switchChanged = false;
  }

  // draw keys (only if required)
  if (gbls->keyChanged) {

    //
    // erase the previous key
    //

    if (gbls->keyOnScreen) {

      index = gbls->keyOnScreenPosition;

      GameGetSpritePosition(prefs,
                            spriteKey, index, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 16;
        scrRect.extent.y  = 11;
        rect.topLeft.x    = index * scrRect.extent.x;
        rect.topLeft.y    = 0;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // invert the old key bitmap!
        WinCopyRectangle(gbls->winKeys, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
        gbls->keyOnScreen = false;
      }
    }

    // 
    // draw the key at the right position
    //

    index = (4 - prefs->game.dkung.keyCount);

    GameGetSpritePosition(prefs,
                          spriteKey, index, prefs->game.dkung.displaySetting,
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

      scrRect.extent.x  = 16;
      scrRect.extent.y  = 11;
      rect.topLeft.x    = index * scrRect.extent.x;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // save this location, record key is on screen
      gbls->keyOnScreen         = true;
      gbls->keyOnScreenPosition = index;

      // draw the key bitmap!
      WinCopyRectangle(gbls->winKeys, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    // dont draw until we need to
    gbls->keyChanged = false;
  }

  // draw hook (only if required)
  if (gbls->hookChanged) {

    //
    // erase the previous hook
    //

    if (gbls->hookOnScreen) {

      index = gbls->hookOnScreenPosition;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs,
                            spriteHook, index, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 12;
        scrRect.extent.y  = 12;
        rect.topLeft.x    = index * scrRect.extent.x; 
        rect.topLeft.y    = 0;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // invert the old hook bitmap
        WinCopyRectangle(gbls->winHooks, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
        gbls->hookOnScreen = false;
      }

      // restore the background area
      rect.topLeft.x    = 0;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;
      WinCopyRectangle(gbls->winHookBackup, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    //
    // draw hook at the new position
    //

    // lets make sure we can see the hook :)
    if (!prefs->game.dkung.hookHide) {

      index = prefs->game.dkung.hookPosition;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs,
                            spriteHook, index, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 12;
        scrRect.extent.y  = 12;
        rect.topLeft.x    = index * scrRect.extent.x;
        rect.topLeft.y    = 0;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;
  
        // save this location and backup area, record hook is onscreen
        gbls->hookOnScreen         = true;
        gbls->hookOnScreenPosition = index;
        GameBackupBitmapArea(WinGetDrawWindow(), 
                             gbls->winHooks, gbls->winHookBackup,
                             &scrRect, &rect); 
  
        // draw the hook bitmap!
        WinCopyRectangle(gbls->winHooks, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }

    // dont draw until we need to
    gbls->hookChanged = false;
  }

  // draw kong (only if required)
  if (gbls->kongChanged) {

    //
    // erase the previous kong
    //

    if (gbls->kongOnScreen) {

      index = gbls->kongOnScreenPosition;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs,
                            spriteKong, index, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 31;
        scrRect.extent.y  = 24;
        rect.topLeft.x    = (index % 3) * scrRect.extent.x; 
        rect.topLeft.y    = (index / 3) * scrRect.extent.y; 
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // invert the old kong bitmap
        WinCopyRectangle(gbls->winKongs, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
        gbls->kongOnScreen = false;
      }

      // restore the background area
      rect.topLeft.x    = 0;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;
      WinCopyRectangle(gbls->winKongBackup, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    //
    // draw kong at the new position
    //

    index = prefs->game.dkung.kongPosition;

    // what is the rectangle we need to copy?
    GameGetSpritePosition(prefs,
                          spriteKong, index, prefs->game.dkung.displaySetting,
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

      scrRect.extent.x  = 31;
      scrRect.extent.y  = 24;
      rect.topLeft.x    = (index % 3) * scrRect.extent.x;
      rect.topLeft.y    = (index / 3) * scrRect.extent.y;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;
  
      // save this location and backup area, record kong is onscreen
      gbls->kongOnScreen         = true;
      gbls->kongOnScreenPosition = index;
      GameBackupBitmapArea(WinGetDrawWindow(), 
                           gbls->winKongs, gbls->winKongBackup,
                           &scrRect, &rect); 

      // draw the kong bitmap!
      WinCopyRectangle(gbls->winKongs, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    // dont draw until we need to
    gbls->kongChanged = false;
  }

  // draw barrels (only if required)
  if (gbls->ballChanged) {

    // 
    // ERASE PREVIOUSLY VISIBLE BALLS
    //

    // go through all possible combinations
    for (i=0; i<MAX_BALLS; i++) {

      // is the ball on screen
      if (gbls->ballOnScreen[i]) {

        // what is the rectangle we need to copy?
        GameGetSpritePosition(prefs,
                              spriteBall, i, prefs->game.dkung.displaySetting,
                              &scrRect.topLeft.x, &scrRect.topLeft.y);
        if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

          scrRect.extent.x  = 10;
          scrRect.extent.y  = 8;
          rect.topLeft.x    = (i % 6) * scrRect.extent.x; 
          rect.topLeft.y    = (i / 6) * scrRect.extent.y; 
          rect.extent.x     = scrRect.extent.x;
          rect.extent.y     = scrRect.extent.y;

          // invert the old ball bitmap
          WinCopyRectangle(gbls->winBalls, WinGetDrawWindow(),
                           &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
          gbls->ballOnScreen[i] = false;
        }

        // restore the background area
        if (gbls->winBallBackup[i] != NULL) {

          rect.topLeft.x    = 0;
          rect.topLeft.y    = 0;
          rect.extent.x     = scrRect.extent.x;
          rect.extent.y     = scrRect.extent.y;
          WinCopyRectangle(gbls->winBallBackup[i], WinGetDrawWindow(),
                           &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
        }
      }
    }

    //
    // DRAW BALL AT NEW POSITION
    //

    // go through all possible combinations
    for (i=0; i<MAX_BALLS; i++) {

      // is the ball visible?
      if (prefs->game.dkung.ballPosition[i]) {

        // what is the rectangle we need to copy?
        GameGetSpritePosition(prefs,
                              spriteBall, i, prefs->game.dkung.displaySetting,
                              &scrRect.topLeft.x, &scrRect.topLeft.y);
        if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

          scrRect.extent.x  = 10;
          scrRect.extent.y  = 8;
          rect.topLeft.x    = (i % 6) * scrRect.extent.x;
          rect.topLeft.y    = (i / 6) * scrRect.extent.y;
          rect.extent.x     = scrRect.extent.x;
          rect.extent.y     = scrRect.extent.y;
  
          // save this location and backup area, record ball is onscreen
          gbls->ballOnScreen[i] = true;
          if (gbls->winBallBackup[i] != NULL) {
            GameBackupBitmapArea(WinGetDrawWindow(), 
                                 gbls->winBalls, gbls->winBallBackup[i],
                                 &scrRect, &rect); 
          }

          // draw the ball bitmap!
          WinCopyRectangle(gbls->winBalls, WinGetDrawWindow(),
                           &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
        }
      }
    }

    // dont draw until we need to
    gbls->ballChanged = false;
  }

  // draw irons (only if required)
  if (gbls->ironChanged) {

    // 
    // ERASE PREVIOUSLY VISIBLE IRONS
    //

    // go through all possible combinations
    for (i=0; i<MAX_IRONS; i++) {

      // is the iron on screen
      if (gbls->ironOnScreen[i]) {

        // what is the rectangle we need to copy?
        GameGetSpritePosition(prefs,
                              spriteIron, i, prefs->game.dkung.displaySetting,
                              &scrRect.topLeft.x, &scrRect.topLeft.y);
        if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

          scrRect.extent.x  = 15;
          scrRect.extent.y  = 7;
          rect.topLeft.x    = (i % 2) * scrRect.extent.x; 
          rect.topLeft.y    = 0;
          rect.extent.x     = scrRect.extent.x;
          rect.extent.y     = scrRect.extent.y;

          // invert the old iron bitmap
          WinCopyRectangle(gbls->winIrons, WinGetDrawWindow(),
                           &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
          gbls->ironOnScreen[i] = false;
        }

        // restore the background area
        if (gbls->winIronBackup[i] != NULL) {

          rect.topLeft.x    = 0;
          rect.topLeft.y    = 0;
          rect.extent.x     = scrRect.extent.x;
          rect.extent.y     = scrRect.extent.y;
          WinCopyRectangle(gbls->winIronBackup[i], WinGetDrawWindow(),
                           &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
        }
      }
    }

    //
    // DRAW IRON AT NEW POSITION
    //

    // go through all possible combinations
    for (i=0; i<MAX_IRONS; i++) {

      // is the iron visible?
      if (prefs->game.dkung.ironPosition[i]) {

        // what is the rectangle we need to copy?
        GameGetSpritePosition(prefs,
                              spriteIron, i, prefs->game.dkung.displaySetting,
                              &scrRect.topLeft.x, &scrRect.topLeft.y);
        if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

          scrRect.extent.x  = 15;
          scrRect.extent.y  = 7;
          rect.topLeft.x    = (i % 2) * scrRect.extent.x;
          rect.topLeft.y    = 0;
          rect.extent.x     = scrRect.extent.x;
          rect.extent.y     = scrRect.extent.y;
  
          // save this location and backup area, record iron is onscreen
          gbls->ironOnScreen[i] = true;
          if (gbls->winIronBackup[i] != NULL) {
            GameBackupBitmapArea(WinGetDrawWindow(), 
                                 gbls->winIrons, gbls->winIronBackup[i],
                                 &scrRect, &rect); 
          }

          // draw the iron bitmap!
          WinCopyRectangle(gbls->winIrons, WinGetDrawWindow(),
                           &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
        }
      }
    }

    // dont draw until we need to
    gbls->ironChanged = false;
  }

  // draw mario (only if required)
  if (gbls->marioChanged) {

    //
    // erase the previous mario
    //

    if (gbls->marioOnScreen) {

      index = gbls->marioOnScreenPosition;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs,
                            spriteMario, index, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

        scrRect.extent.x  = 20;
        scrRect.extent.y  = 20;
        rect.topLeft.x    = (index % 8) * scrRect.extent.x; 
        rect.topLeft.y    = (index / 8) * scrRect.extent.y; 
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // invert the old mario bitmap
        WinCopyRectangle(gbls->winMarios, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
        gbls->marioOnScreen = false;
      }

      // restore the background area
      rect.topLeft.x    = 0;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;
      WinCopyRectangle(gbls->winMarioBackup, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    //
    // draw mario at the new position
    //

    index = prefs->game.dkung.marioPosition;

    // what is the rectangle we need to copy?
    GameGetSpritePosition(prefs,
                          spriteMario, index, prefs->game.dkung.displaySetting,
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

      scrRect.extent.x  = 20;
      scrRect.extent.y  = 20;
      rect.topLeft.x    = (index % 8) * scrRect.extent.x;
      rect.topLeft.y    = (index / 8) * scrRect.extent.y;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;
  
      // save this location and backup area, record mario is onscreen
      gbls->marioOnScreen         = true;
      gbls->marioOnScreenPosition = index;
      GameBackupBitmapArea(WinGetDrawWindow(), 
                           gbls->winMarios, gbls->winMarioBackup,
                           &scrRect, &rect); 

      // draw the mario bitmap!
      WinCopyRectangle(gbls->winMarios, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    // play the miss sound if needed :P
    switch (prefs->game.dkung.marioPosition)
    {
      case 36:
           {
             SndCommandType missSnd 
               = {sndCmdFreqDurationAmp,0,4096,250,sndMaxAmp};
             DevicePlaySound(&missSnd);
           }
           break;

      case 37:
           {
             SndCommandType missSnd 
               = {sndCmdFreqDurationAmp,0, 512,250,sndMaxAmp};
             DevicePlaySound(&missSnd);
           }
           break;

      default:
           break;
    }

    // dont draw until we need to
    gbls->marioChanged = false;
  }

  // reset the clipping area :)
  if (!prefs->handera.device)
    WinResetClip();
}

/**
 * Get the position of a particular sprite on the screen.
 *
 * @param spriteType the type of sprite.
 * @param index the index required in the sprite position list.
 * @param displayLevel which display is currently being shown.
 * @param x the x co-ordinate of the position
 * @param y the y co-ordinate of the position
 */
void
GameGetSpritePosition(PreferencesType *prefs, 
                      UInt8 spriteType, 
                      UInt8 index, 
                      UInt8 displayLevel, 
                      Coord *x, 
                      Coord *y)
{
  UInt16 i;

  switch (spriteType) 
  {
    case spriteDigit:
         {
           *x = 123 + (index * 9);
           if (!prefs->handera.device)
             *y = 92;
           else
             *y = 170;
         }
         break;

    case spriteLife:
         {
           Coord positions[][3][2] = {
                          { {  64,  95 }, {  -1,  -1 }, {  -1,  -1 } }  
                                     };

           *x = positions[index][displayLevel][0];
           *y = positions[index][displayLevel][1];

           // offset as appropriate
           if (prefs->handera.device)
           {
             // loop through all :)
             for (i=0; i<3; i++)
             {
               if (positions[index][i][0] != -1)
               {
                 *x = positions[index][i][0]; 
                 *y = positions[index][i][1];

                 if (i == displayLower)  *y += 78; else
                 if (i == displayMiddle) *y += 42; 

                 i = 4;
               }
             }
           }
         }
         break;

    case spriteIron:
         {
           Coord positions[][3][2] = {
                          { { 109,  23 }, { 109,  59 }, { 109, 103 } },  
                          { {  86,  22 }, {  86,  58 }, {  86, 102 } },  
                          { {  62,  22 }, {  62,  58 }, {  62, 102 } },  
                          { {  38,  21 }, {  38,  57 }, {  38, 101 } },  
                          { {  14,  20 }, {  14,  56 }, {  14, 100 } }  
                                     };

           *x = positions[index][displayLevel][0];
           *y = positions[index][displayLevel][1];

           // offset as appropriate
           if (prefs->handera.device)
           {
             // loop through all :)
             for (i=0; i<3; i++)
             {
               if (positions[index][i][0] != -1)
               {
                 *x = positions[index][i][0]; 
                 *y = positions[index][i][1];

                 if (i == displayLower)  *y += 78; else
                 if (i == displayMiddle) *y += 42; 

                 i = 4;
               }
             }
           }
         }
         break;

    case spriteBar:
         {
           Coord positions[][3][2] = {
                          { {  -1,  -1 }, {  -1,  -1 }, {  14,  28 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {   5,  32 } }  
                                     };

           *x = positions[index][displayLevel][0];
           *y = positions[index][displayLevel][1];

           // offset as appropriate
           if (prefs->handera.device)
           {
             // loop through all :)
             for (i=0; i<3; i++)
             {
               if (positions[index][i][0] != -1)
               {
                 *x = positions[index][i][0]; 
                 *y = positions[index][i][1];

                 if (i == displayLower)  *y += 78; else
                 if (i == displayMiddle) *y += 42; 

                 i = 4;
               }
             }
           }
         }
         break;

    case spriteCrane:
         {
           Coord positions[][3][2] = {
                          { {  -1,  -1 }, { 141,   0 }, { 141,  44 } },
                          { {  -1,  -1 }, {  -1,  -1 }, { 131,  27 } },  
                          { {  -1,  -1 }, {  -1,  -1 }, { 138,   3 } }
                                     };

           *x = positions[index][displayLevel][0];
           *y = positions[index][displayLevel][1];

           // offset as appropriate
           if (prefs->handera.device)
           {
             // loop through all :)
             for (i=0; i<3; i++)
             {
               if (positions[index][i][0] != -1)
               {
                 *x = positions[index][i][0]; 
                 *y = positions[index][i][1];

                 if (i == displayLower)  *y += 78; else
                 if (i == displayMiddle) *y += 42; 

                 i = 4;
               }
             }
           }
         }
         break;

    case spriteSwitch:
         {
           Coord positions[][3][2] = {
                          { {  -1,  -1 }, {  13,  14 }, {  13,  58 } },
                          { {  -1,  -1 }, {  11,   8 }, {  11,  52 } }  
                                     };

           *x = positions[index][displayLevel][0];
           *y = positions[index][displayLevel][1];

           // offset as appropriate
           if (prefs->handera.device)
           {
             // loop through all :)
             for (i=0; i<3; i++)
             {
               if (positions[index][i][0] != -1)
               {
                 *x = positions[index][i][0]; 
                 *y = positions[index][i][1];

                 if (i == displayLower)  *y += 78; else
                 if (i == displayMiddle) *y += 42; 

                 i = 4;
               }
             }
           }
         }
         break;

    case spriteKey:
         {
           Coord positions[][3][2] = {
                          { {  -1,  -1 }, {  -1,  -1 }, { 111,  21 } },
                          { {  -1,  -1 }, {  -1,  -1 }, { 111,  21 } },
                          { {  -1,  -1 }, {  -1,  -1 }, { 111,  21 } },
                          { {  -1,  -1 }, {  -1,  -1 }, { 111,  21 } }
                                     };

           *x = positions[index][displayLevel][0];
           *y = positions[index][displayLevel][1];

           // offset as appropriate
           if (prefs->handera.device)
           {
             // loop through all :)
             for (i=0; i<3; i++)
             {
               if (positions[index][i][0] != -1)
               {
                 *x = positions[index][i][0]; 
                 *y = positions[index][i][1];

                 if (i == displayLower)  *y += 78; else
                 if (i == displayMiddle) *y += 42; 

                 i = 4;
               }
             }
           }
         }
         break;

    case spriteHook:
         {
           Coord positions[][3][2] = {
                          { {  -1,  -1 }, {  -1,  -1 }, { 119,  32 } },
                          { {  -1,  -1 }, {  -1,  -1 }, { 124,  34 } },
                          { {  -1,  -1 }, {  -1,  -1 }, { 131,  34 } },
                          { {  -1,  -1 }, {  -1,  -1 }, { 135,  34 } },
                          { {  -1,  -1 }, {  -1,  -1 }, { 138,  34 } }
                                     };

           *x = positions[index][displayLevel][0];
           *y = positions[index][displayLevel][1];

           // offset as appropriate
           if (prefs->handera.device)
           {
             // loop through all :)
             for (i=0; i<3; i++)
             {
               if (positions[index][i][0] != -1)
               {
                 *x = positions[index][i][0]; 
                 *y = positions[index][i][1];

                 if (i == displayLower)  *y += 78; else
                 if (i == displayMiddle) *y += 42; 

                 i = 4;
               }
             }
           }
         }
         break;

    case spriteMario:
         {
           Coord positions[][3][2] = {
                          { {  19,  75 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  19,  64 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  43,  74 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  67,  73 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  89,  72 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  89,  61 }, {  -1,  -1 }, {  -1,  -1 } },

                          { { 116,  69 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { { 114,  43 }, { 114,  79 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  90,  41 }, {  90,  77 }, {  -1,  -1 } },
                          { {  87,  29 }, {  87,  65 }, {  -1,  -1 } },
                          { {  66,  40 }, {  66,  76 }, {  -1,  -1 } },
                          { {  66,  29 }, {  66,  65 }, {  -1,  -1 } },

                          { {  44,  38 }, {  44,  74 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  17,  39 }, {  17,  75 }, {  -1,  -1 } },
                          { {  16,  27 }, {  16,  63 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  15,  29 }, {  15,  73 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  16,   6 }, {  16,  50 } },
                          { {  -1,  -1 }, {   0,   6 }, {   0,  50 } },

                          { {  -1,  -1 }, {  61,   6 }, {  61,  50 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  86,   6 }, {  86,  50 } },
                          { {  -1,  -1 }, {  -1,  -1 }, { 103,  34 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },

                          { {  -1,  -1 }, { 126,   2 }, { 126,  46 } },
                          { {  -1,  -1 }, {  -1,  -1 }, { 126,  12 } },
                          { {  -1,  -1 }, { 126,   2 }, { 126,  46 } },
                          { {  -1,  -1 }, { 146,  30 }, { 146,  74 } },
                          { {  -1,  -1 }, { 116,  11 }, { 116,  55 } },
                          { {  -1,  -1 }, { 126,  28 }, { 126,  72 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } } // info

                                     };

           *x = positions[index][displayLevel][0];
           *y = positions[index][displayLevel][1];

           // offset as appropriate
           if (prefs->handera.device)
           {
             // loop through all :)
             for (i=0; i<3; i++)
             {
               if (positions[index][i][0] != -1)
               {
                 *x = positions[index][i][0]; 
                 *y = positions[index][i][1];

                 if (i == displayLower)  *y += 78; else
                 if (i == displayMiddle) *y += 42; 

                 i = 4;
               }
             }
           }
         }
         break;

    case spriteKong:
         {
           Coord positions[][3][2] = {
                          { {  -1,  -1 }, {  -1,  -1 }, {  19,   3 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  19,   3 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  49,   3 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  49,   3 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  81,   3 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  81,   3 } },
                          { {  -1,  -1 }, { 105,  23 }, { 105,  67 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } }
                                     };

           *x = positions[index][displayLevel][0];
           *y = positions[index][displayLevel][1];

           // offset as appropriate
           if (prefs->handera.device)
           {
             // loop through all :)
             for (i=0; i<3; i++)
             {
               if (positions[index][i][0] != -1)
               {
                 *x = positions[index][i][0]; 
                 *y = positions[index][i][1];

                 if (i == displayLower)  *y += 78; else
                 if (i == displayMiddle) *y += 42; 

                 i = 4;
               }
             }
           }
         }
         break;

    case spriteBall:
         {
           Coord positions[][3][2] = {
                          { {  -1,  -1 }, {  -1,  -1 }, {  28,  31 } },
                          { {  -1,  -1 }, {  30,   2 }, {  30,  46 } },
                          { {  -1,  -1 }, {  29,  21 }, {  29,  65 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } }, // f1

                          { {  -1,  -1 }, {  -1,  -1 }, {  58,  31 } },
                          { {  -1,  -1 }, {  58,   2 }, {  58,  46 } },
                          { {  -1,  -1 }, {  59,  21 }, {  59,  65 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } }, // f2

                          { {  -1,  -1 }, {  -1,  -1 }, {  92,  31 } },
                          { {  -1,  -1 }, {  94,   2 }, {  94,  46 } },
                          { {  -1,  -1 }, {  93,  21 }, {  93,  65 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } }, // f3

                          { {  -1,  -1 }, {  93,  31 }, {  93,  75 } },
                          { {  -1,  -1 }, {  77,  32 }, {  77,  76 } },
                          { {  -1,  -1 }, {  61,  33 }, {  61,  77 } },
                          { {  -1,  -1 }, {  45,  34 }, {  45,  78 } },
                          { {  -1,  -1 }, {  29,  34 }, {  29,  78 } },
                          { {   7,   2 }, {   7,  38 }, {   7,  82 } },

                          { {   7,  28 }, {   7,  64 }, {  -1,  -1 } },
                          { {   7,  43 }, {   7,  79 }, {  -1,  -1 } },
                          { {  34,  44 }, {  34,  80 }, {  -1,  -1 } },
                          { {  57,  45 }, {  57,  81 }, {  -1,  -1 } },
                          { {  81,  46 }, {  81,  82 }, {  -1,  -1 } },
                          { { 104,  47 }, { 104,  83 }, {  -1,  -1 } },

                          { { 128,  49 }, { 128,  85 }, {  -1,  -1 } },
                          { { 128,  75 }, {  -1,  -1 }, {  -1,  -1 } },
                          { { 104,  76 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  81,  77 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  57,  78 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  34,  79 }, {  -1,  -1 }, {  -1,  -1 } },

                          { {   8,  82 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } },
                          { {  -1,  -1 }, {  -1,  -1 }, {  -1,  -1 } }
                                     };

           *x = positions[index][displayLevel][0];
           *y = positions[index][displayLevel][1];

           // offset as appropriate
           if (prefs->handera.device)
           {
             // loop through all :)
             for (i=0; i<3; i++)
             {
               if (positions[index][i][0] != -1)
               {
                 *x = positions[index][i][0]; 
                 *y = positions[index][i][1];

                 if (i == displayLower)  *y += 78; else
                 if (i == displayMiddle) *y += 42; 

                 i = 4;
               }
             }
           }
         }
         break;

    default:
         break;
  }

  // offset to real world co-ordinates 
  if (prefs->handera.device)
  {
    *y += 36;
    *x += 40;
  }
  else
    *y += 34;
}

/**
 * Terminate the game.
 */
void   
GameTerminate()
{
  GameGlobals *gbls;
  Int16       i;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // unlock the gamepad driver (if available)
  if (gbls->hardware.gamePadPresent) {

    Err    err;
    UInt32 gamePadUserCount;

    err = GPDClose(gbls->hardware.gamePadLibRef, &gamePadUserCount);
    if (gamePadUserCount == 0)
      SysLibRemove(gbls->hardware.gamePadLibRef);
  }

  // clean up windows/memory
  if (gbls->winBackground != NULL) 
    WinDeleteWindow(gbls->winBackground,  false);
  if (gbls->winDigits != NULL)
    WinDeleteWindow(gbls->winDigits,      false);
  if (gbls->winLives != NULL)
    WinDeleteWindow(gbls->winLives,       false);
  if (gbls->winIrons != NULL)
    WinDeleteWindow(gbls->winIrons,       false);
  for (i=0; i<MAX_IRONS; i++) {
    if (gbls->winIronBackup[i] != NULL) 
      WinDeleteWindow(gbls->winIronBackup[i], false); 
  }
  if (gbls->winBars != NULL)
    WinDeleteWindow(gbls->winBars,        false);
  if (gbls->winBarBackup != NULL)
    WinDeleteWindow(gbls->winBarBackup,   false);
  if (gbls->winCranes != NULL)
    WinDeleteWindow(gbls->winCranes,      false);
  if (gbls->winSwitches != NULL)
    WinDeleteWindow(gbls->winSwitches,    false);
  if (gbls->winKeys != NULL)
    WinDeleteWindow(gbls->winKeys,        false);
  if (gbls->winHooks != NULL)
    WinDeleteWindow(gbls->winHooks,       false);
  if (gbls->winHookBackup != NULL)
    WinDeleteWindow(gbls->winHookBackup,  false);
  if (gbls->winMarios != NULL)
    WinDeleteWindow(gbls->winMarios,      false);
  if (gbls->winMarioBackup != NULL) 
    WinDeleteWindow(gbls->winMarioBackup, false);
  if (gbls->winKongs != NULL)
    WinDeleteWindow(gbls->winKongs,       false);
  if (gbls->winKongBackup != NULL)
    WinDeleteWindow(gbls->winKongBackup,  false);
  if (gbls->winBalls != NULL)
    WinDeleteWindow(gbls->winBalls,       false);
  for (i=0; i<MAX_BALLS; i++) {
    if (gbls->winBallBackup[i] != NULL) 
      WinDeleteWindow(gbls->winBallBackup[i], false); 
  }
  MemPtrFree(gbls);

  // unregister global data
  FtrUnregister(appCreator, ftrGameGlobals);
}

/**
 * Adjust the level (remove birds that are too close and reset positions)
 *
 * @param prefs the global preference data.
 */
static void 
GameAdjustLevel(PreferencesType *prefs)
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // return to start position
  prefs->game.dkung.marioCount       = 0;
  prefs->game.dkung.marioPosition    = 0;
  prefs->game.dkung.marioNewPosition = 0;
  prefs->game.dkung.marioJumpWait    = 0;
  prefs->game.dkung.displaySetting   = displayLower;
  prefs->game.dkung.displayChanged   = true;

  if (prefs->game.dkung.kongPosition == 6) {
    prefs->game.dkung.kongPosition = ((SysRandom(0) % 3) * 2);
    prefs->game.dkung.kongWait     = gbls->gameSpeed[gbls->gameType] + 1;
    prefs->game.dkung.ballWait     = gbls->gameSpeed[gbls->gameType] + 1;

    gbls->kongChanged              = true;
  }

  // give player a chance next time around
  prefs->game.dkung.ballPosition[34] = false;
  prefs->game.dkung.ballPosition[35] = false;
  prefs->game.dkung.ballPosition[36] = false;
  gbls->ballChanged                  = true;

  // player is not dead :))
  gbls->playerDied                   = false;
}

/**
 * Increment the players score. 
 *
 * @param prefs the global preference data.
 * @param count the amount to increment the score by.
 */
static void 
GameIncrementScore(PreferencesType *prefs, UInt16 count)
{
  GameGlobals    *gbls;
  UInt16         i, index;
  RectangleType  rect     = { {   0,   0 }, {   0,   0 } };
  RectangleType  scrRect  = { {   0,   0 }, {   0,   0 } };
  SndCommandType scoreSnd = {sndCmdFreqDurationAmp,0,1024, 5,sndMaxAmp};

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // adjust accordingly
  for (i=0; i<count; i++)
    prefs->game.gameScore += (prefs->game.dkung.bonusScoring) ? 2 : 1;

  // redraw score bitmap
  {
    UInt16 base;
 
    base = 1000;  // max score (4 digits)
    for (i=0; i<4; i++) {

      index = (prefs->game.gameScore / base) % 10;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(prefs,
                            spriteDigit, i, prefs->game.dkung.displaySetting,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 7;
      scrRect.extent.y  = 12;
      rect.topLeft.x    = index * scrRect.extent.x;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the digit!
      WinCopyRectangle(gbls->winDigits, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);
      base /= 10;
    }
  }

  // play the sound (if collecting) 
  if (prefs->game.dkung.marioPosition >= 32) {
    DevicePlaySound(&scoreSnd);
    SysTaskDelay(5);
  }

  // is it time for a bonus?
  if (
      (prefs->game.gameScore >= 300) &&
      (prefs->game.dkung.bonusAvailable)
     ) {

    SndCommandType snd = {sndCmdFreqDurationAmp,0,0,5,sndMaxAmp};

    // give a little fan-fare sound
    for (i=0; i<15; i++) {
      snd.param1 += 256 + (1 << i);  // frequency
      DevicePlaySound(&snd);

      SysTaskDelay(2); // small deley 
    }

    // apply the bonus!
    if (prefs->game.gameLives == 3)
      prefs->game.dkung.bonusScoring = true;
 
    prefs->game.gameLives++;
    prefs->game.dkung.bonusAvailable = false;

    //
    // DRAW LIVES
    //

    // what is the rectangle we need to copy?
    GameGetSpritePosition(prefs,
                          spriteLife, 0, prefs->game.dkung.displaySetting,
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

      scrRect.extent.x  = 33;
      scrRect.extent.y  = 10;
      rect.topLeft.x    = 0;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the life bitmap!
      WinCopyRectangle(gbls->winLives, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }
  }
}

/**
 * Move the player.
 *
 * @param prefs the global preference data.
 */
static void
GameMovePlayer(PreferencesType *prefs) 
{
  GameGlobals    *gbls;
  SndCommandType plymvSnd = {sndCmdFreqDurationAmp,0, 768, 5,sndMaxAmp};
  SndCommandType plyjpSnd = {sndCmdFreqDurationAmp,0,1024, 5,sndMaxAmp};

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // mario in normal gameplay?
  if (prefs->game.dkung.marioPosition < 27) {

    //
    // where does mario want to go today?
    //

    // current position differs from new position?
    if (prefs->game.dkung.marioPosition != prefs->game.dkung.marioNewPosition) {

      // lower platform
      if (prefs->game.dkung.marioPosition < 10) {

        // special case, jump up to next level
        if ((prefs->game.dkung.marioPosition    == 8) &&
            (prefs->game.dkung.marioNewPosition > 8)) {
          gbls->moveNext = moveUp;
        }

        // move left
        else
        if ((prefs->game.dkung.marioPosition    >> 1) >
            (prefs->game.dkung.marioNewPosition >> 1)) {
          gbls->moveNext = moveLeft;
        }

        // move right:
        else
        if ((prefs->game.dkung.marioPosition    >> 1) <
            (prefs->game.dkung.marioNewPosition >> 1)) {
          gbls->moveNext = moveRight;
        }

        // jump up:
        else
        if (((prefs->game.dkung.marioNewPosition % 2) == 1) &&
            ((prefs->game.dkung.marioPosition    % 2) == 0)) {
          gbls->moveNext = moveJump;
        }
      }

      // middle platform
      else
      if (prefs->game.dkung.marioPosition < 22) {

        // special case, jump up to next level
        if ((prefs->game.dkung.marioPosition    >= 18) &&
            (prefs->game.dkung.marioNewPosition >  
              prefs->game.dkung.marioPosition)) {
          gbls->moveNext = moveUp;
        }

        // special cases, move down
        else
        if ((prefs->game.dkung.marioPosition    > 18) &&
            (prefs->game.dkung.marioNewPosition < 
              prefs->game.dkung.marioPosition)) {
          gbls->moveNext = moveDown;
        }

        // special cases, move down level
        else
        if ((prefs->game.dkung.marioPosition    == 10) &&
            (prefs->game.dkung.marioNewPosition  < 10)) {
          gbls->moveNext = moveDown;
        }

        // move left
        else
        if ((prefs->game.dkung.marioPosition    >> 1) <
            (prefs->game.dkung.marioNewPosition >> 1)) {
          gbls->moveNext = moveLeft;
        }

        // move right:
        else
        if ((prefs->game.dkung.marioPosition    >> 1) >
            (prefs->game.dkung.marioNewPosition >> 1)) {
          gbls->moveNext = moveRight;
        }

        // jump up:
        else
        if (((prefs->game.dkung.marioNewPosition % 2) == 1) &&
            ((prefs->game.dkung.marioPosition    % 2) == 0)) {
          gbls->moveNext = moveJump;
        }
      }

      // upper platform
      else {

        // special case, jump for key
        if ((prefs->game.dkung.marioPosition    == 26) &&
            (prefs->game.dkung.marioNewPosition == 27)) {
          gbls->moveNext = moveJumpKey;
        }

        // special case, flick switch
        if ((prefs->game.dkung.marioPosition    == 22) &&
            (prefs->game.dkung.marioNewPosition == 23)) {
          gbls->moveNext = moveUseSwitch;
        }

        // special case, move down
        else
        if ((prefs->game.dkung.marioPosition    == 22) &&
            (prefs->game.dkung.marioNewPosition <  22)) {
          gbls->moveNext = moveDown;
        }

        // move left
        else
        if ((prefs->game.dkung.marioPosition    >> 1) >
            (prefs->game.dkung.marioNewPosition >> 1)) {
          gbls->moveNext = moveLeft;
        }

        // move right:
        else
        if ((prefs->game.dkung.marioPosition    >> 1) <
            (prefs->game.dkung.marioNewPosition >> 1)) {
          gbls->moveNext = moveRight;
        }
      }
    }

    // lets make sure they are allowed to do the move
    if (
        (gbls->moveDelayCount == 0) ||
        (gbls->moveLast != gbls->moveNext)
       ) {
      gbls->moveDelayCount = gbls->gameSpeed[gbls->gameType] - 1;
    }
    else {
      gbls->moveDelayCount--;
      gbls->moveNext = moveNone;
    }

    //
    // move mario into the right position based on his desired move
    //

    // is the player suspended in the air?
    if ((prefs->game.dkung.marioPosition % 2) == 1) {

      // has mario just completed a jump over a barrel?   
      if (prefs->game.dkung.marioJumpWait == 0) {

        Int16  i;
        UInt16 jumpBallPos[]  = { 36, 33, 29, 28 };
        UInt16 jumpMarioPos[] = {  1,  7, 13, 15 };
        UInt16 jumpScore[]    = {  1,  1,  2,  2 };

        for (i=0; i<4; i++) {

          if ((prefs->game.dkung.ballPosition[jumpBallPos[i]] == true) &&
              (prefs->game.dkung.marioPosition == jumpMarioPos[i]))

            // apply the points needed
            GameIncrementScore(prefs, jumpScore[i]);
        }
      }
    }

    // update counter
    prefs->game.dkung.marioCount++;

    // can the player move?
    if (prefs->game.dkung.marioJumpWait == 0) {

      // is the player suspended in the air?
      if ((prefs->game.dkung.marioPosition ==  1) ||
          (prefs->game.dkung.marioPosition ==  7) ||
          (prefs->game.dkung.marioPosition == 13) ||
          (prefs->game.dkung.marioPosition == 15)) {

        // lets make sure they dont jump back up :)
        if (prefs->game.dkung.marioNewPosition == prefs->game.dkung.marioPosition)
          prefs->game.dkung.marioNewPosition = prefs->game.dkung.marioPosition - 1;

        // automagically move down :)
        prefs->game.dkung.marioPosition--;
        gbls->marioChanged               = true;

        gbls->moveNext = moveNone;
      }

      // which direction do they wish to move?
      switch (gbls->moveNext)
      {
        case moveLeft:

             // lower platform
             if (prefs->game.dkung.marioPosition < 10) {

               // lets make sure they can move left
               if ((prefs->game.dkung.marioPosition > 1) &&
                   ((prefs->game.dkung.marioPosition % 2) == 0)) {

                 Int16  i;
                 UInt16 collideBallPos[]  = { 35, 34, 33, 32 };
                 UInt16 collideMarioPos[] = {  2,  4,  6,  8 };

                 // has mario just walked into a barrel?
                 for (i=0; i<4; i++) {

                   gbls->playerDied |=
                     (
                      (prefs->game.dkung.ballPosition[collideBallPos[i]] == true) &&
                      (prefs->game.dkung.marioPosition == collideMarioPos[i])
                     );
                 }

                 // mario did a valid move (no death)?
                 if (!gbls->playerDied) {

                   prefs->game.dkung.marioPosition -= 2;
                   gbls->marioChanged               = true;
                 }
               }
             }

             // middle platform
             else
             if (prefs->game.dkung.marioPosition < 22) {

               // lets make sure they can move left
               if ((prefs->game.dkung.marioPosition < 18) &&
                   ((prefs->game.dkung.marioPosition % 2) == 0)) {

                 Int16  i;
                 UInt16 collideBallPos[]  = { 29, 28, 27, 26 };
                 UInt16 collideMarioPos[] = { 10, 12, 14, 16 };

                 // has mario just walked into a barrel?
                 for (i=0; i<4; i++) {

                   gbls->playerDied |=
                     (
                      (prefs->game.dkung.ballPosition[collideBallPos[i]] == true) &&
                      (prefs->game.dkung.marioPosition == collideMarioPos[i])
                     );
                 }

                 // mario did a valid move (no death)?
                 if (!gbls->playerDied) {

                   prefs->game.dkung.marioPosition += 2;
                   gbls->marioChanged               = true;
                 }
               }
             }

             // upper platform
             else {

               // lets make sure they can move left
               if (prefs->game.dkung.marioPosition > 22) {

                 // mario did a valid move (no death)?
                 if (!gbls->playerDied) {

                   prefs->game.dkung.marioPosition -= 2;
                   gbls->marioChanged               = true;
                 }
               }
             }
             break;

        case moveRight:

             // lower platform
             if (prefs->game.dkung.marioPosition < 10) {

               // lets make sure they can move right
               if ((prefs->game.dkung.marioPosition < 8) &&
                   ((prefs->game.dkung.marioPosition % 2) == 0)) {

                 Int16  i;
                 UInt16 collideBallPos[]  = { 35, 34, 33, 32 };
                 UInt16 collideMarioPos[] = {  0,  2,  4,  6 };

                 // has mario just walked into a barrel?
                 for (i=0; i<4; i++) {

                   gbls->playerDied |=
                     (
                      (prefs->game.dkung.ballPosition[collideBallPos[i]] == true) &&
                      (prefs->game.dkung.marioPosition == collideMarioPos[i])
                     );
                 }

                 // mario did a valid move (no death)?
                 if (!gbls->playerDied) {

                   prefs->game.dkung.marioPosition += 2;
                   gbls->marioChanged               = true;
                 }
               }
             }

             // middle platform
             else
             if (prefs->game.dkung.marioPosition < 22) {

               // lets make sure they can move right
               if ((prefs->game.dkung.marioPosition > 10) &&
                   ((prefs->game.dkung.marioPosition % 2) == 0)) {

                 Int16  i;
                 UInt16 collideBallPos[]  = { 29, 28, 27, 26 };
                 UInt16 collideMarioPos[] = { 12, 14, 16, 18 };

                 // has mario just walked into a barrel?
                 for (i=0; i<4; i++) {

                   gbls->playerDied |=
                     (
                      (prefs->game.dkung.ballPosition[collideBallPos[i]] == true) &&
                      (prefs->game.dkung.marioPosition == collideMarioPos[i])
                     );
                 }

                 // mario did a valid move (no death)?
                 if (!gbls->playerDied) {

                   prefs->game.dkung.marioPosition -= 2;
                   gbls->marioChanged               = true;
                 }
               }
             }

             // upper platform
             else {

               // lets make sure they can move right
               if (prefs->game.dkung.marioPosition < 26) {

                 // mario did a valid move (no death)?
                 if (!gbls->playerDied) {

                   prefs->game.dkung.marioPosition += 2;
                   gbls->marioChanged               = true;
                 }
               }
             }
             break;

        case moveJump:

             // we can only jump if we are on the ground
             if ((prefs->game.dkung.marioPosition % 2) == 0) {

               prefs->game.dkung.marioPosition++;
               prefs->game.dkung.marioJumpWait = 
                 gbls->gameSpeed[gbls->gameType] - 1;
               gbls->marioChanged = true;
             }
             break;

        case moveJumpKey:

             // go for it!
             prefs->game.dkung.marioJumpWait =
               gbls->gameSpeed[gbls->gameType] - 1;
             prefs->game.dkung.marioPosition = 27;
             gbls->marioChanged              = true;

             break;

        case moveUseSwitch:

             {
               RectangleType rect    = { {   0,   0 }, {   0,   0 } };
               RectangleType scrRect = { {   0,   0 }, {   0,   0 } };
               UInt16        i, index;

               //
               // flick that switch :) (if possible)
               //

               if (prefs->game.dkung.switchPosition == 0) {
 
                 // change the position of the switch
                 prefs->game.dkung.switchPosition = 1;

                 // erase the previous switch (if there)
                 if (gbls->switchOnScreen) {

                   index = gbls->switchOnScreenPosition;

                   GameGetSpritePosition(prefs,
                                         spriteSwitch, index, prefs->game.dkung.displaySetting,
                                         &scrRect.topLeft.x, &scrRect.topLeft.y);
                   if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

                     scrRect.extent.x  = 6;
                     scrRect.extent.y  = 5;
                     rect.topLeft.x    = index * scrRect.extent.x;
                     rect.topLeft.y    = 0;
                     rect.extent.x     = scrRect.extent.x;
                     rect.extent.y     = scrRect.extent.y;

                     // invert the old switch bitmap!
                     WinCopyRectangle(gbls->winSwitches, WinGetDrawWindow(),
                                      &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
                     gbls->switchOnScreen = false;
                   }
                 }

                 // draw the switch at the right position
                 index = prefs->game.dkung.switchPosition;

                 GameGetSpritePosition(prefs,
                                       spriteSwitch, index, prefs->game.dkung.displaySetting,
                                       &scrRect.topLeft.x, &scrRect.topLeft.y);
                 if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

                   scrRect.extent.x  = 6;
                   scrRect.extent.y  = 5;
                   rect.topLeft.x    = index * scrRect.extent.x;
                   rect.topLeft.y    = 0;
                   rect.extent.x     = scrRect.extent.x;
                   rect.extent.y     = scrRect.extent.y;
  
                   // save this location, record switch is on screen
                   gbls->switchOnScreen         = true;
                   gbls->switchOnScreenPosition = index;
  
                   // draw the switch bitmap!
                   WinCopyRectangle(gbls->winSwitches, WinGetDrawWindow(),
                                    &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
                 }

                 // dont draw until we need to
                 gbls->switchChanged = false;

                 // start the crane!!
                 prefs->game.dkung.cranePosition = 1;
                 prefs->game.dkung.hookHide      = false; 
                 prefs->game.dkung.hookPosition  = 0; 
                 prefs->game.dkung.hookDirection = 1; 
                 prefs->game.dkung.hookWait      = 7;  // first = longer 

                 gbls->craneChanged              = true;
                 gbls->hookChanged               = true;
               }

               // draw bitmap 23 then 22 :)
               for (i=0; i<2; i++) {

                 // erase the previous mario (if available)
                 if (gbls->marioOnScreen) {
 
                   index = gbls->marioOnScreenPosition;
 
                   // what is the rectangle we need to copy?
                   GameGetSpritePosition(prefs,
                                         spriteMario, 22, prefs->game.dkung.displaySetting,
                                         &scrRect.topLeft.x, &scrRect.topLeft.y);
                   if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {
   
                     scrRect.extent.x  = 20;
                     scrRect.extent.y  = 20;
                     rect.topLeft.x    = (index % 8) * scrRect.extent.x; 
                     rect.topLeft.y    = (index / 8) * scrRect.extent.y; 
                     rect.extent.x     = scrRect.extent.x;
                     rect.extent.y     = scrRect.extent.y;
   
                     // invert the old mario bitmap
                     WinCopyRectangle(gbls->winMarios, WinGetDrawWindow(),
                                      &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
                     gbls->marioOnScreen = false;
                   }
  
                   // restore the background area
                   rect.topLeft.x    = 0;
                   rect.topLeft.y    = 0;
                   rect.extent.x     = scrRect.extent.x;
                   rect.extent.y     = scrRect.extent.y;
                   WinCopyRectangle(gbls->winMarioBackup, WinGetDrawWindow(),
                                    &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
                 }

                 index = 23 - i;

                 // what is the rectangle we need to copy?
                 GameGetSpritePosition(prefs,
                                       spriteMario, 22, prefs->game.dkung.displaySetting,
                                       &scrRect.topLeft.x, &scrRect.topLeft.y);
                 if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {
  
                   scrRect.extent.x  = 20;
                   scrRect.extent.y  = 20;
                   rect.topLeft.x    = (index % 8) * scrRect.extent.x;
                   rect.topLeft.y    = (index / 8) * scrRect.extent.y;
                   rect.extent.x     = scrRect.extent.x;
                   rect.extent.y     = scrRect.extent.y;
    
                   // save this location and backup area, record mario is onscreen
                   gbls->marioOnScreen         = true;
                   gbls->marioOnScreenPosition = index;
                   GameBackupBitmapArea(WinGetDrawWindow(), 
                                        gbls->winMarios, gbls->winMarioBackup,
                                        &scrRect, &rect); 

                   // draw the mario bitmap!
                   WinCopyRectangle(gbls->winMarios, WinGetDrawWindow(),
                                    &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);

                   // dont draw until we need to
                   gbls->marioChanged = false;
                 }

                 // wait one frames
                 SysTaskDelay(SysTicksPerSecond() / GAME_FPS);
               }
             }

             // stay put :)
             prefs->game.dkung.marioNewPosition =
               prefs->game.dkung.marioPosition;

             break;

        case moveUp:

             switch (prefs->game.dkung.marioPosition)
             {
               case 8:  

                    prefs->game.dkung.marioPosition = 10;
                    gbls->marioChanged              = true;
                    break;

               case 18: 

                    prefs->game.dkung.marioPosition = 19; 
                    gbls->marioChanged              = true;

                    // layer switch
                    if (!prefs->handera.device)
                    {
                      prefs->game.dkung.displaySetting = displayMiddle;
                      prefs->game.dkung.displayChanged = true;
                    }
                    break;

               case 19: 

                    prefs->game.dkung.marioPosition = 20; 
                    gbls->marioChanged              = true;

                    // layer switch
                    if (!prefs->handera.device)
                    {
                      prefs->game.dkung.displaySetting = displayUpper;
                      prefs->game.dkung.displayChanged = true;
                    }
                    break;

               case 20: 

                    prefs->game.dkung.marioPosition = 22; 
                    gbls->marioChanged              = true;
                    break;

               default: 
                    break;
             }
             break;

        case moveDown:

             switch (prefs->game.dkung.marioPosition)
             {
               case 10: 

                    prefs->game.dkung.marioPosition = 8;
                    gbls->marioChanged              = true;
                    break;

               case 19: 

                    prefs->game.dkung.marioPosition = 18; 
                    gbls->marioChanged              = true;

                    // layer switch
                    if (!prefs->handera.device)
                    {
                      prefs->game.dkung.displaySetting = displayLower;
                      prefs->game.dkung.displayChanged = true;
                    }
                    break;

               case 20: 

                    prefs->game.dkung.marioPosition = 19; 
                    gbls->marioChanged              = true;

                    // layer switch
                    if (!prefs->handera.device)
                    {
                      prefs->game.dkung.displaySetting = displayMiddle;
                      prefs->game.dkung.displayChanged = true;
                    }
                    break;

               case 22: 

                    prefs->game.dkung.marioPosition = 20; 
                    gbls->marioChanged              = true;
                    break;

               default: 
                    break;
             }
             break;

        default:
             break;
      }

      gbls->moveLast = gbls->moveNext;
      gbls->moveNext = moveNone;
    }
    else 
      prefs->game.dkung.marioJumpWait--;
  }

  // mario reaching for key? falling?
  else {

    Boolean grabbedHook = false;

    // lets determine if mario grabbed the hook
    grabbedHook = (
                    (!prefs->game.dkung.hookHide) &&
                    (prefs->game.dkung.marioPosition == 27) &&
                    (prefs->game.dkung.hookPosition  == 0)  &&
                    (prefs->game.dkung.hookWait      <  
                       ((prefs->game.dkung.hookDirection == 1) ? 6 : 2)) 
                   );

    // mario did not grab the hook :(
    if (!grabbedHook) {

      // can the player move?
      if (prefs->game.dkung.marioJumpWait == 0) {

        switch (prefs->game.dkung.marioPosition)
        {
          case 27:

               prefs->game.dkung.marioPosition  = 36;
               prefs->game.dkung.marioJumpWait  = 2;

               gbls->marioChanged = true;
               break;

          case 32:

               prefs->game.dkung.marioPosition++;
               prefs->game.dkung.marioJumpWait  = 5;

               // release a key
               prefs->game.dkung.keyCount--;

               // adjust the "hook+crane" bitmaps :)
               prefs->game.dkung.hookHide       = true;
               prefs->game.dkung.cranePosition  = 2;

               gbls->keyChanged   = true;
               gbls->marioChanged = true;
               gbls->hookChanged  = true;
               gbls->craneChanged = true;

               break;

          case 33:  // got key :)
               {
                 RectangleType  rect    = { {   0,   0 }, {   0,   0 } };
                 RectangleType  scrRect = { {   0,   0 }, {   0,   0 } };
                 UInt16         i, index, bonus, ticks;
                 SndCommandType plymvSnd 
                   = {sndCmdFreqDurationAmp,0, 768, 5,sndMaxAmp};
                 SndCommandType kongfallSnd 
                   = {sndCmdFreqDurationAmp,0, 512,75,sndMaxAmp};

                 // how many half seconds did it take?
                 bonus = 0;
                 ticks = (prefs->game.dkung.marioCount / (GAME_FPS >> 1));

                 // if < 5 secs +20, > 20 secs +5, between? 5..20 :P
                 if (ticks < 10)  bonus += 20; else
                 if (ticks > 40)  bonus += 5;  else
                                  bonus += 5 + ((40 - ticks) / 2);

                 // apply the bonus
                 for (i=0; i<bonus; i++)
                   GameIncrementScore(prefs, 1);

                 // do we need to trash kong? :)
                 if (prefs->game.dkung.keyCount == 0) {
                 
                   // first beep :)
                   DevicePlaySound(&plymvSnd);
                   SysTaskDelay(50);

                   GameGetSpritePosition(prefs,
                                         spriteBar, 0, prefs->game.dkung.displaySetting,
                                         &scrRect.topLeft.x, &scrRect.topLeft.y);

                   scrRect.extent.x  = 98;
                   scrRect.extent.y  = 26;
                   rect.topLeft.x    = 0;
                   rect.topLeft.y    = 0;
                   rect.extent.x     = scrRect.extent.x;
                   rect.extent.y     = scrRect.extent.y;

                   // flash the "barrier" :)
                   for (i=0; i<5; i++) {

                     // invert the bar bitmap!
                     WinCopyRectangle(gbls->winBars, WinGetDrawWindow(),
                                      &rect, scrRect.topLeft.x, scrRect.topLeft.y, winInvert);

                     if ((i % 2) == 1)
                       DevicePlaySound(&plymvSnd);
                     SysTaskDelay(50);
                   }

                   //
                   // kong falls!!
                   //

                   // erase the previous kong
                   if (gbls->kongOnScreen) {

                     index = gbls->kongOnScreenPosition;

                     // what is the rectangle we need to copy?
                     GameGetSpritePosition(prefs,
                                           spriteKong, index, prefs->game.dkung.displaySetting,
                                           &scrRect.topLeft.x, &scrRect.topLeft.y);
                     if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

                       scrRect.extent.x  = 31;
                       scrRect.extent.y  = 24;
                       rect.topLeft.x    = (index % 3) * scrRect.extent.x; 
                       rect.topLeft.y    = (index / 3) * scrRect.extent.y; 
                       rect.extent.x     = scrRect.extent.x;
                       rect.extent.y     = scrRect.extent.y;

                       // invert the old kong bitmap
                       WinCopyRectangle(gbls->winKongs, WinGetDrawWindow(),
                                        &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
                       gbls->kongOnScreen = false;
                     }

                     // restore the background area
                     rect.topLeft.x    = 0;
                     rect.topLeft.y    = 0;
                     rect.extent.x     = scrRect.extent.x;
                     rect.extent.y     = scrRect.extent.y;
                     WinCopyRectangle(gbls->winKongBackup, WinGetDrawWindow(),
                                      &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
                   }

                   // draw kong at the new position
                   prefs->game.dkung.kongPosition = 6;
                   index = prefs->game.dkung.kongPosition;

                   // what is the rectangle we need to copy?
                   GameGetSpritePosition(prefs,
                                         spriteKong, index, prefs->game.dkung.displaySetting,
                                         &scrRect.topLeft.x, &scrRect.topLeft.y);
                   if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

                     scrRect.extent.x  = 31;
                     scrRect.extent.y  = 24;
                     rect.topLeft.x    = (index % 3) * scrRect.extent.x;
                     rect.topLeft.y    = (index / 3) * scrRect.extent.y;
                     rect.extent.x     = scrRect.extent.x;
                     rect.extent.y     = scrRect.extent.y;
  
                     // save this location and backup area, record kong is onscreen
                     gbls->kongOnScreen         = true;
                     gbls->kongOnScreenPosition = index;
                     GameBackupBitmapArea(WinGetDrawWindow(), 
                                          gbls->winKongs, gbls->winKongBackup,
                                          &scrRect, &rect); 

                     // draw the kong bitmap!
                     WinCopyRectangle(gbls->winKongs, WinGetDrawWindow(),
                                      &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);

                     // dont draw until we need to
                     gbls->kongChanged = false;
                   }

                   //
                   // bars fall..
                   //

                   // what is the rectangle we need to copy?
                   GameGetSpritePosition(prefs,
                                         spriteBar, 1, prefs->game.dkung.displaySetting,
                                         &scrRect.topLeft.x, &scrRect.topLeft.y);
                   if ((scrRect.topLeft.x != -1) && (scrRect.topLeft.y != -1)) {

                     scrRect.extent.x  = 98;
                     scrRect.extent.y  = 26;
                     rect.topLeft.x    = 0;
                     rect.topLeft.y    = scrRect.extent.y;
                     rect.extent.x     = scrRect.extent.x;
                     rect.extent.y     = scrRect.extent.y;
  
                     // backup area
                     GameBackupBitmapArea(WinGetDrawWindow(), 
                                          gbls->winBars, gbls->winBarBackup,
                                          &scrRect, &rect); 
                 
                     // draw the bar bitmap!
                     WinCopyRectangle(gbls->winBars, WinGetDrawWindow(),
                                      &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
               
                     DevicePlaySound(&kongfallSnd);
                     SysTaskDelay(50);

                     // apply the bonus
                     for (i=0; i<20; i++)
                       GameIncrementScore(prefs, 1);

                     // invert the old bar bitmap
                     WinCopyRectangle(gbls->winBars, WinGetDrawWindow(),
                                      &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);

                     // restore the background area
                     rect.topLeft.x    = 0;
                     rect.topLeft.y    = 0;
                     rect.extent.x     = scrRect.extent.x;
                     rect.extent.y     = scrRect.extent.y;
                     WinCopyRectangle(gbls->winBarBackup, WinGetDrawWindow(),
                                      &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
                   }
                 }
               }

               prefs->game.dkung.marioPosition++;
               prefs->game.dkung.marioJumpWait  = 5;

               // adjust the "hook+crane" bitmaps :)
               prefs->game.dkung.hookHide       = false;
               prefs->game.dkung.hookPosition   = 2;
               prefs->game.dkung.cranePosition  = 1;

               gbls->marioChanged = true;
               gbls->hookChanged  = true;
               gbls->craneChanged = true;

               break;

          case 34:

               prefs->game.dkung.marioPosition++;
               prefs->game.dkung.marioJumpWait  = 5;

               // adjust the "hook+crane" bitmaps :)
               prefs->game.dkung.hookHide       = true;
               prefs->game.dkung.cranePosition  = 0;

               // ready for another round of crane activity :)
               prefs->game.dkung.switchPosition = 0;
               gbls->switchChanged              = true;

               gbls->marioChanged = true;
               gbls->hookChanged  = true;
               gbls->craneChanged = true;

               break;

          case 35:  
               GameAdjustLevel(prefs);

               // has the platform been opened?
               if (prefs->game.dkung.keyCount == 0) {

                 GameAdjustmentType adjustType;

                 // define the adjustment
                 adjustType.adjustMode = gameKeyReset;

                 // reset the cage:
                 if (RegisterAdjustGame(prefs, &adjustType)) {
                 }

#ifdef PROTECTION_ON
                 // terminate the game:
                 else {

                   EventType event;

                   // "please register" dialog :)
                   ApplicationDisplayDialog(rbugForm);

                   // GAME OVER - return to main screen
                   MemSet(&event, sizeof(EventType), 0);
                   event.eType            = menuEvent;
                   event.data.menu.itemID = gameMenuItemExit;
                   EvtAddEventToQueue(&event);

                   // stop the game in its tracks
                   prefs->game.gamePlaying = false;
                 }
#endif
               }

               break;

          case 36:

               prefs->game.dkung.marioPosition++;
               prefs->game.dkung.marioJumpWait  = 3;

               gbls->marioChanged = true;
               break;

          case 37:  // mario missed :)
               gbls->playerDied = true;
               break;

          default:
               break;
        }
      }
      else
        prefs->game.dkung.marioJumpWait--;
    }

    // mario DID grab the key :)
    else {

      prefs->game.dkung.marioPosition = 32;   // skip forward
      prefs->game.dkung.marioJumpWait = 5;

      // adjust the "hook+crane" bitmaps :)
      prefs->game.dkung.hookHide      = false;
      prefs->game.dkung.hookPosition  = 2;
      prefs->game.dkung.cranePosition = 1;

      gbls->marioChanged = true;
      gbls->hookChanged  = true;
      gbls->craneChanged = true;
    }
  }

  // do we need to play a movement sound?
  if ((gbls->marioChanged) &&
      (prefs->game.dkung.marioPosition < 34)) {
 
    // normal movement
    if (((prefs->game.dkung.marioPosition % 2) == 0) ||
        (prefs->game.dkung.marioPosition == 19)      || 
        (prefs->game.dkung.marioPosition  > 27)) {  

      // may have just come out of a jump :)
      if (gbls->moveLast != moveNone) {
        DevicePlaySound(&plymvSnd);
      }
    }

    // jump?
    else
    if (prefs->game.dkung.marioPosition != 23) 
      DevicePlaySound(&plyjpSnd);
  }
}

/**
 * Move the obstacles (barrells, irons and kong).
 *
 * @param prefs the global preference data.
 */
static void
GameMoveObstacles(PreferencesType *prefs) 
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  //
  // process special "dying" cases :)
  //

  switch (prefs->game.dkung.marioPosition)
  {
    // 
    // iron deaths
    //

    case 13:
         // hit by iron when in air :)
         gbls->playerDied |= (prefs->game.dkung.ironPosition[1]  == true);
         break;

    case 15:
         // hit by iron when in air :)
         gbls->playerDied |= (prefs->game.dkung.ironPosition[2]  == true);
         break;

    case 19:
         // hit by iron when in air :)
         gbls->playerDied |= (prefs->game.dkung.ironPosition[4]  == true);
         break;

    // 
    // barrell deaths
    //

    case 22:
         // hit by barrell at far left on top
         gbls->playerDied |= (prefs->game.dkung.ballPosition[1]  == true);
         break;

    case 24:
         // hit by barrell at center on top
         gbls->playerDied |= (prefs->game.dkung.ballPosition[7]  == true);
         break;

    case 26:
         // hit by barrell at far right on top
         gbls->playerDied |= (prefs->game.dkung.ballPosition[13] == true);
         break;

    default:
         break;
  }

  // can ball move?
  if (prefs->game.dkung.ballWait == 0) {

    Int16  i;
    UInt16 collideBallPos[]  = { 22, 25, 26, 27, 28, 29, 31, 32, 33, 34, 35 };
    UInt16 collideMarioPos[] = { 20, 18, 16, 14, 12, 10,  8,  6,  4,  2,  0 };

    // process all the collision positions
    for (i=0; i<11; i++) {

      gbls->playerDied |= 
        (
         (prefs->game.dkung.ballPosition[collideBallPos[i]] == true) &&
         (prefs->game.dkung.marioPosition == collideMarioPos[i])
        );
    }
  }

  //
  // move everything if there are no "problems" :P
  //

  // only do this if the player is still alive
  if (!gbls->playerDied) {

    SndCommandType grdmvSnd = {sndCmdFreqDurationAmp,0, 384, 5,sndMaxAmp};
    UInt16         i, kongReferencePosition;
    Boolean        throwBarrellChance;

    // initialize
    kongReferencePosition = prefs->game.dkung.kongPosition / 2;
    throwBarrellChance    = 
      ((SysRandom(0) % ((gbls->gameType == GAME_A) ? 5 : 4)) < 3);

    //
    // move the irons :)
    //

    // can iron move?
    if (prefs->game.dkung.ironWait == 0) {

      // move all the irons as appropriate :)
      for (i=0; i<MAX_IRONS; i++) {

        // is there an iron active right now?
        if (prefs->game.dkung.ironPosition[MAX_IRONS-1-i]) {

          // erase previous iron
          prefs->game.dkung.ironPosition[MAX_IRONS-1-i] = false;

          // move to new position
          switch (MAX_IRONS-1-i)
          {
            case  5: // we do nothing, its gone :)
                     break;

            default: prefs->game.dkung.ironPosition[MAX_IRONS-i] = true;
                     break;
          }

          gbls->ironChanged = true;
        }
      }

      // start a new iron if possible :)
      if ((prefs->game.dkung.ironPosition[0] == false) &&
          (prefs->game.dkung.ironPosition[1] == false) &&
          (gbls->gameType != GAME_A) &&                  // no irons in game A
          ((SysRandom(0) % gbls->gameSpeed[gbls->gameType]) == 0)) {

        // insert iron into the game
        prefs->game.dkung.ironPosition[0] = true;
        gbls->ironChanged = true;
      }

      // must wait a few frames now :P
      prefs->game.dkung.ironWait = gbls->gameSpeed[gbls->gameType] + 2;
    }
    else 
      prefs->game.dkung.ironWait--;

    //
    // move the barrells :)
    //

    // can ball move?
    if (prefs->game.dkung.ballWait == 0) {

      // move all the barrels as appropriate :)
      for (i=0; i<MAX_BALLS; i++) {
    
        // is there a ball active right now?
        if (prefs->game.dkung.ballPosition[MAX_BALLS-1-i]) {

          // erase previous ball
          prefs->game.dkung.ballPosition[MAX_BALLS-1-i] = false;

          // move to new position
          switch (MAX_BALLS-1-i)
          {
            case  2: prefs->game.dkung.ballPosition[22] = true;
                     break;

            case  8: prefs->game.dkung.ballPosition[20] = true;
                     break;

            case 14: prefs->game.dkung.ballPosition[18] = true;
                     break;

            case 36: // we do nothing, its gone :)
                     break;

            default: prefs->game.dkung.ballPosition[MAX_BALLS-i] = true;
                     break;
          }

          gbls->ballChanged = true;
        }
      }

      // drop a barrell if possible :)
      if (((prefs->game.dkung.kongPosition % 2) == 1) &&
          (prefs->game.dkung.kongWait < (gbls->gameSpeed[gbls->gameType]-1))) {

        // we need to do a small filter to prevent two-in-a-row ;)
        if (
            (prefs->game.dkung.ballPosition[ 1] == false) &&
            (prefs->game.dkung.ballPosition[ 7] == false) &&
            (prefs->game.dkung.ballPosition[13] == false) &&
            (prefs->game.dkung.ballPosition[18] == false) &&
            (prefs->game.dkung.ballPosition[20] == false) &&
            (prefs->game.dkung.ballPosition[22] == false) 
           ) {

          // insert barrell into the game
          prefs->game.dkung.ballPosition[kongReferencePosition * 6] = true;
          gbls->ballChanged = true;

          // return to normal position
          prefs->game.dkung.kongPosition--;
          prefs->game.dkung.kongWait    += gbls->gameSpeed[gbls->gameType] + 1;
          gbls->kongChanged = true;
        }
      }

      // must wait a few frames now :P
      prefs->game.dkung.ballWait     = gbls->gameSpeed[gbls->gameType];
    }
    else 
      prefs->game.dkung.ballWait--;

    //
    // move kong
    //

    // we dont do squat if kong is flat on his ass :)
    if (prefs->game.dkung.kongPosition == 6) goto SKIP_KONG_MOVEMENT;

    // can kong move?
    if (prefs->game.dkung.kongWait == 0) {

      // we wont allow movement if about to throw
      if ((prefs->game.dkung.kongPosition % 2) == 0) {
  
        // mario in upper level
        if (prefs->game.dkung.marioPosition > 18) {

          // is the crane active?
          if (prefs->game.dkung.cranePosition != 0) {

            // at far-right position?
            if (kongReferencePosition == 2) {
         
              // move to center?
              if ((SysRandom(0) % 20) < 3) {

                prefs->game.dkung.kongPosition = 2;
                prefs->game.dkung.kongWait     = gbls->gameSpeed[gbls->gameType] + 1;
                gbls->kongChanged = true;
              }
            } 

            // move right?
            else
            if ((SysRandom(0) % 10) < 6) {
  
              prefs->game.dkung.kongPosition = (kongReferencePosition+1) * 2;
              prefs->game.dkung.kongWait     = gbls->gameSpeed[gbls->gameType] + 1;
              gbls->kongChanged = true;
            }
          }

          // kong at far-left position?
          else
          if (kongReferencePosition == 0) {
  
            // move to center?
            if ((SysRandom(0) % 20) < 3) {
  
              prefs->game.dkung.kongPosition = 2;
              prefs->game.dkung.kongWait     = gbls->gameSpeed[gbls->gameType] + 1;
              gbls->kongChanged = true;
            }
          }
  
          // otherwise..
          else {
  
            // move left?
            if ((SysRandom(0) % 10) < 6) {
  
              prefs->game.dkung.kongPosition = (kongReferencePosition-1) * 2;
              prefs->game.dkung.kongWait     = gbls->gameSpeed[gbls->gameType] + 1;
              gbls->kongChanged = true;
            }
          }
  
          // threaten to throw a barrell?
          if ((throwBarrellChance) && (!gbls->kongChanged) &&
              ((prefs->game.dkung.kongPosition % 2) == 0)) {
  
            prefs->game.dkung.kongPosition += ((SysRandom(0) % 2) == 0) ? 1 : 0;
            prefs->game.dkung.kongWait     = gbls->gameSpeed[gbls->gameType] + 1;
            gbls->kongChanged = true;
          }
        }
  
        // mario on lower level
        else {
  
          // kong is in middle, need to shuffle left or right :)
          if (((SysRandom(0) % 20) < 3) && (kongReferencePosition == 1)) {
  
            // shuffle?
            if ((SysRandom(0) % 5) < 1) {
  
              prefs->game.dkung.kongPosition = 4;
              prefs->game.dkung.kongWait     = gbls->gameSpeed[gbls->gameType] + 1;
              gbls->kongChanged = true;
            }
          }
  
          // threaten to throw a barrell?
          if ((throwBarrellChance) && (!gbls->kongChanged) &&
              ((prefs->game.dkung.kongPosition % 2) == 0)) {
  
            prefs->game.dkung.kongPosition += 
              ((SysRandom(0) % ((gbls->gameType == GAME_A) ? 6 : 3)) == 0) ? 1 : 0;
            prefs->game.dkung.kongWait     = gbls->gameSpeed[gbls->gameType] + 1;
            gbls->kongChanged = true;
          }
        }
      }
    }
    else 
      prefs->game.dkung.kongWait--;

SKIP_KONG_MOVEMENT:

    // play the "beep" if needed
    if (gbls->ballChanged)
      DevicePlaySound(&grdmvSnd);
  }
}

/**
 * Backup the area behind the bitmap mask giving the specific rectangles.
 * 
 * @param screen      LCD display handle
 * @param bitmapMask  the mask area to save behind.
 * @param destination the window to store the result in.
 * @param scrRect     the area of the screen we are working with.
 * @param rect        the area of the mask to work with.
 */
static void 
GameBackupBitmapArea(WinHandle     screen,
                     WinHandle     bitmapMask,
                     WinHandle     destination, 
                     RectangleType *scrRect,
                     RectangleType *rect)
{
  // post 3.5 - use API's
  if (DeviceSupportsVersion(romVersion3_5)) {

    WinCopyRectangle(screen, destination,
                     scrRect, 0, 0, winPaint);
    WinCopyRectangle(bitmapMask, destination,
                     rect, 0, 0, winErase);
  }

  // pre 3.5 doesn't handle it correct, manual fix :(
  else {

    UInt16 i, size;
    UInt8  *dest, *scrn, *mask;

    dest = (UInt8 *)destination->displayAddrV20;
    size = MemPtrSize(dest);

    // allocate the two buffers
    scrn = (UInt8 *)MemPtrNew(size * sizeof(UInt8));
    mask = (UInt8 *)MemPtrNew(size * sizeof(UInt8));

    // copy data to the "scrn" buffer
    WinCopyRectangle(screen, destination,
                     scrRect, 0, 0, winPaint);
    for (i=0; i<size; i++) 
      scrn[i] = dest[i];

    // copy data to the "mask" buffer
    WinCopyRectangle(bitmapMask, destination,
                     rect, 0, 0, winPaint);
    for (i=0; i<size; i++) 
      mask[i] = dest[i];

    // do the mask
    for (i=0; i<size; i++) 
      dest[i] = (scrn[i] & mask[i]);

    // free the two buffers
    MemPtrFree(mask);
    MemPtrFree(scrn);
  }
}
