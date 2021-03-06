/*****************************************************************************
 * controls.h: MacOS X interface module
 *****************************************************************************
 * Copyright (C) 2002-2011 the VideoLAN team
 * $Id: 65f883e7d4b49fa58fd59885a7da516cf42b1a54 $
 *
 * Authors: Jon Lech Johansen <jon-vl@nanocrew.net>
 *          Christophe Massiot <massiot@via.ecp.fr>
 *          Derk-Jan Hartman <thedj@users.sourceforge.net>
 *          Felix Paul Kühne <fkuehne at videolan org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#import "fspanel.h"

/*****************************************************************************
 * VLCControls interface
 *****************************************************************************/
@interface VLCControls : NSObject
{
    IBOutlet id o_main;

    IBOutlet id o_volumeslider;

    IBOutlet id o_btn_repeat;
    IBOutlet id o_btn_shuffle;

    IBOutlet id o_specificTime_cancel_btn;
    IBOutlet id o_specificTime_enter_fld;
    IBOutlet id o_specificTime_goTo_lbl;
    IBOutlet id o_specificTime_ok_btn;
    IBOutlet id o_specificTime_win;
    IBOutlet id o_specificTime_sec_lbl;
    IBOutlet id o_specificTime_stepper;
    IBOutlet id o_specificTime_mi;

    VLCFSPanel *o_fs_panel;
    BOOL b_lockAspectRatio;
}
- (id)voutView;
- (BOOL)aspectRatioIsLocked;

- (IBAction)play:(id)sender;
- (IBAction)stop:(id)sender;
- (IBAction)faster:(id)sender;
- (IBAction)slower:(id)sender;
- (IBAction)normalSpeed:(id)sender;

- (IBAction)prev:(id)sender;
- (IBAction)next:(id)sender;
- (IBAction)random:(id)sender;
- (IBAction)repeat:(id)sender;
- (IBAction)loop:(id)sender;
- (IBAction)repeatButtonAction:(id)sender;
- (IBAction)quitAfterPlayback:(id)sender;

/* the three ugly helpers again */
- (void)repeatOne;
- (void)repeatAll;
- (void)repeatOff;
- (void)shuffle;

- (IBAction)forward:(id)sender;
- (IBAction)backward:(id)sender;

- (IBAction)volumeUp:(id)sender;
- (IBAction)volumeDown:(id)sender;
- (IBAction)mute:(id)sender;
- (IBAction)volumeSliderUpdated:(id)sender;

- (IBAction)showPosition: (id)sender;
- (IBAction)toogleFullscreen:(id)sender;
- (BOOL)isFullscreen;
- (IBAction)windowAction:(id)sender;

- (IBAction)telxTransparent:(id)sender;
- (IBAction)telxNavLink:(id)sender;
- (IBAction)lockVideosAspectRatio:(id)sender;
- (IBAction)addSubtitleFile:(id)sender;

- (BOOL)keyEvent:(NSEvent *)o_event;
- (void)scrollWheel: (NSEvent *)theEvent;

- (IBAction)goToSpecificTime:(id)sender;

- (id)fspanel;

@end

/*****************************************************************************
 * VLCTimeField interface
 *****************************************************************************
 * we need the implementation to catch our click-event in the controller window
 *****************************************************************************/

@interface VLCTimeField : NSTextField
{
}
@end

