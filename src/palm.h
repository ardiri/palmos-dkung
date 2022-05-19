/*
 * @(#)palm.h
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

#ifndef _PALM_H
#define _PALM_H

// system includes
#include <PalmOS.h>
#include <System/DLServer.h>

// resource "include" :P
#include "resource.h"

// special includes (additional hardware etc)
#include "hardware/GPDLib.h"
#include "hardware/HanderaVGA.h"

// application constants and structures
#define appCreator 'DKNG'
#define __REGISTER__ __attribute__ ((section ("register"))) // code0002.bin
#define __DEVICE__   __attribute__ ((section ("device")))   // code0003.bin
#define __GAME__     __attribute__ ((section ("game")))     // code0004.bin
#define __HELP__     __attribute__ ((section ("help")))     // code0005.bin
#define __SAFE0001__ __attribute__ ((section ("safe0001"))) // code0006.bin
#define __SAFE0002__ __attribute__ ((section ("safe0002"))) // code0007.bin

#define ftrGlobals             1000
#define ftrGlobalsCfgActiveVol 1001
#define ftrDeviceGlobals       2000
#define ftrGameGlobals         3000
#define ftrHelpGlobals         4000
#define ftrRegisterGlobals     1004 // <-- must be, as original code0007.bin
//#define ftrRegisterGlobals   5000

#define GAME_FPS  8  // 8 frames per second
#define VERSION   1

#define MAX_BALLS 42
#define MAX_IRONS 5

typedef struct
{
  struct 
  {
    UInt8     signatureVersion;         // a preference version number
    Char      signature[16];            // a "signature" for decryption
    Char      *hotSyncUsername;         // the HotSync user name of the user
  } system;

  struct 
  {
    UInt16    ctlKeyJump;               // key definition for jump
    UInt16    ctlKeyUp;                 // key definition for move up
    UInt16    ctlKeyDown;               // key definition for move down
    UInt16    ctlKeyLeft;               // key definition for move left
    UInt16    ctlKeyRight;              // key definition for move right

    Boolean   sndMute;                  // sound is muted?
    UInt16    sndVolume;                // the volume setting for sound

    UInt8     lgray;                    // the light gray configuration setting
    UInt8     dgray;                    // the dark gray configuration setting
  } config;

  struct 
  {
    Boolean   gamePlaying;              // is there a game in play?
    Boolean   gamePaused;               // is the game currently paused?
    Boolean   gameWait;                 // are we waiting for user? 
    UInt16    gameAnimationCount;       // a ticking counter
    UInt16    gameScore;                // the score of the player
    UInt16    highScore[2];             // a high score list (score only)
    UInt16    gameLives;                // the number of lives the player has

    struct 
    {
      Int8    gameType;                 // what type of game are we playing?
      Boolean bonusAvailable;           // is the bonus available?
      Boolean bonusScoring;             // are we currently in BONUS mode?

      Int8    displaySetting;           // which display area is being shown?
      Boolean displayChanged;           // has the display been changed?

      UInt16  cranePosition;            // the position of the crane

      UInt16  switchPosition;           // the position of the switch

      UInt16  keyCount;                 // the number of keys visible

      Boolean hookHide;                 // should the hook be hidden?
      Int16   hookDirection;            // the direction of the swing
      UInt16  hookPosition;             // the position of the switch
      UInt16  hookWait;                 // the delay between hook swings

      UInt16  kongPosition;             // the position of kong
      UInt16  kongWait;                 // the delay between kong movements

      Boolean ballPosition[MAX_BALLS];  // the status of the barrell :)
      UInt16  ballWait;                 // the delay between barrell movements

      Boolean ironPosition[MAX_IRONS];  // the status of the iron :)
      UInt16  ironWait;                 // the delay between iron movements

      UInt16  marioCount;               // the frame count for the player
      UInt16  marioPosition;            // the position of mario
      UInt16  marioNewPosition;         // the desired position of mario
      UInt16  marioJumpWait;            // the delay mario is in the air

    } dkung;

  } game;
  
  struct
  {

    Boolean device;                     // are we running on handera device?
    VgaScreenModeType scrMode;          // current mode (scale, 1:1 etc)
    VgaRotateModeType scrRotate;        // current display rotation

  } handera;

} PreferencesType;

// this is our 'double check' for decryption - make sure it worked :P
#define CHECK_SIGNATURE(x) (StrCompare(x->system.signature, "|HaCkMe|") == 0)

// local includes
#include "device.h"
#include "help.h"
#include "game.h"
#include "register.h"
#include "gccfix.h"

// functions
extern UInt32  PilotMain(UInt16, MemPtr, UInt16);
extern void    InitApplication(void);
extern Boolean ApplicationHandleEvent(EventType *);
extern void    ApplicationDisplayDialog(UInt16);
extern void    EventLoop(void);
extern void    EndApplication(void);

#endif 
