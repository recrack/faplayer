/*****************************************************************************
 * output.c : internal management of output streams for the audio output
 *****************************************************************************
 * Copyright (C) 2002-2004 the VideoLAN team
 * $Id: d2626c3c4138b7783411cdc7ae2fcc6570632d14 $
 *
 * Authors: Christophe Massiot <massiot@via.ecp.fr>
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

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_aout.h>
#include <vlc_cpu.h>
#include <vlc_modules.h>

#include "aout_internal.h"

/*****************************************************************************
 * aout_OutputNew : allocate a new output and rework the filter pipeline
 *****************************************************************************
 * This function is entered with the mixer lock.
 *****************************************************************************/
int aout_OutputNew( aout_instance_t * p_aout,
                    const audio_sample_format_t * p_format )
{
    p_aout->output.output = *p_format;

    /* Retrieve user defaults. */
    int i_rate = var_InheritInteger( p_aout, "aout-rate" );
    if ( i_rate != 0 )
        p_aout->output.output.i_rate = i_rate;
    aout_FormatPrepare( &p_aout->output.output );

    /* Find the best output plug-in. */
    p_aout->output.p_module = module_need( p_aout, "audio output", "$aout", false );
    if ( p_aout->output.p_module == NULL )
    {
        msg_Err( p_aout, "no suitable audio output module" );
        return -1;
    }

    if ( var_Type( p_aout, "audio-channels" ) ==
             (VLC_VAR_INTEGER | VLC_VAR_HASCHOICE) )
    {
        /* The user may have selected a different channels configuration. */
        switch( var_InheritInteger( p_aout, "audio-channels" ) )
        {
            case AOUT_VAR_CHAN_RSTEREO:
                p_aout->output.output.i_original_channels |=
                                                       AOUT_CHAN_REVERSESTEREO;
                break;
            case AOUT_VAR_CHAN_STEREO:
                p_aout->output.output.i_original_channels =
                                              AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT;
                break;
            case AOUT_VAR_CHAN_LEFT:
                p_aout->output.output.i_original_channels = AOUT_CHAN_LEFT;
                break;
            case AOUT_VAR_CHAN_RIGHT:
                p_aout->output.output.i_original_channels = AOUT_CHAN_RIGHT;
                break;
            case AOUT_VAR_CHAN_DOLBYS:
                p_aout->output.output.i_original_channels =
                      AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT | AOUT_CHAN_DOLBYSTEREO;
                break;
        }
    }
    else if ( p_aout->output.output.i_physical_channels == AOUT_CHAN_CENTER
              && (p_aout->output.output.i_original_channels
                   & AOUT_CHAN_PHYSMASK) == (AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT) )
    {
        vlc_value_t val, text;

        /* Mono - create the audio-channels variable. */
        var_Create( p_aout, "audio-channels",
                    VLC_VAR_INTEGER | VLC_VAR_HASCHOICE );
        text.psz_string = _("Audio Channels");
        var_Change( p_aout, "audio-channels", VLC_VAR_SETTEXT, &text, NULL );

        val.i_int = AOUT_VAR_CHAN_STEREO; text.psz_string = _("Stereo");
        var_Change( p_aout, "audio-channels", VLC_VAR_ADDCHOICE, &val, &text );
        val.i_int = AOUT_VAR_CHAN_LEFT; text.psz_string = _("Left");
        var_Change( p_aout, "audio-channels", VLC_VAR_ADDCHOICE, &val, &text );
        val.i_int = AOUT_VAR_CHAN_RIGHT; text.psz_string = _("Right");
        var_Change( p_aout, "audio-channels", VLC_VAR_ADDCHOICE, &val, &text );
        if ( p_aout->output.output.i_original_channels & AOUT_CHAN_DUALMONO )
        {
            /* Go directly to the left channel. */
            p_aout->output.output.i_original_channels = AOUT_CHAN_LEFT;
            var_SetInteger( p_aout, "audio-channels", AOUT_VAR_CHAN_LEFT );
        }
        var_AddCallback( p_aout, "audio-channels", aout_ChannelsRestart,
                         NULL );
    }
    else if ( p_aout->output.output.i_physical_channels ==
               (AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT)
                && (p_aout->output.output.i_original_channels &
                     (AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT)) )
    {
        vlc_value_t val, text;

        /* Stereo - create the audio-channels variable. */
        var_Create( p_aout, "audio-channels",
                    VLC_VAR_INTEGER | VLC_VAR_HASCHOICE );
        text.psz_string = _("Audio Channels");
        var_Change( p_aout, "audio-channels", VLC_VAR_SETTEXT, &text, NULL );

        if ( p_aout->output.output.i_original_channels & AOUT_CHAN_DOLBYSTEREO )
        {
            val.i_int = AOUT_VAR_CHAN_DOLBYS;
            text.psz_string = _("Dolby Surround");
        }
        else
        {
            val.i_int = AOUT_VAR_CHAN_STEREO;
            text.psz_string = _("Stereo");
        }
        var_Change( p_aout, "audio-channels", VLC_VAR_ADDCHOICE, &val, &text );
        val.i_int = AOUT_VAR_CHAN_LEFT; text.psz_string = _("Left");
        var_Change( p_aout, "audio-channels", VLC_VAR_ADDCHOICE, &val, &text );
        val.i_int = AOUT_VAR_CHAN_RIGHT; text.psz_string = _("Right");
        var_Change( p_aout, "audio-channels", VLC_VAR_ADDCHOICE, &val, &text );
        val.i_int = AOUT_VAR_CHAN_RSTEREO; text.psz_string=_("Reverse stereo");
        var_Change( p_aout, "audio-channels", VLC_VAR_ADDCHOICE, &val, &text );
        if ( p_aout->output.output.i_original_channels & AOUT_CHAN_DUALMONO )
        {
            /* Go directly to the left channel. */
            p_aout->output.output.i_original_channels = AOUT_CHAN_LEFT;
            var_SetInteger( p_aout, "audio-channels", AOUT_VAR_CHAN_LEFT );
        }
        var_AddCallback( p_aout, "audio-channels", aout_ChannelsRestart,
                         NULL );
    }
    var_TriggerCallback( p_aout, "intf-change" );

    aout_FormatPrepare( &p_aout->output.output );

    aout_lock_output_fifo( p_aout );

    /* Prepare FIFO. */
    aout_FifoInit( p_aout, &p_aout->output.fifo, p_aout->output.output.i_rate );

    aout_unlock_output_fifo( p_aout );

    aout_FormatPrint( p_aout, "output", &p_aout->output.output );

    /* Calculate the resulting mixer output format. */
    p_aout->mixer_format = p_aout->output.output;
    if ( !AOUT_FMT_NON_LINEAR(&p_aout->output.output) )
    {
        /* Non-S/PDIF mixer only deals with float32 or fixed32. */
        p_aout->mixer_format.i_format
                     = HAVE_FPU ? VLC_CODEC_FL32 : VLC_CODEC_FI32;
        aout_FormatPrepare( &p_aout->mixer_format );
    }
    else
    {
        p_aout->mixer_format.i_format = p_format->i_format;
    }

    aout_FormatPrint( p_aout, "mixer", &p_aout->mixer_format );

    /* Create filters. */
    p_aout->output.i_nb_filters = 0;
    if ( aout_FiltersCreatePipeline( p_aout, p_aout->output.pp_filters,
                                     &p_aout->output.i_nb_filters,
                                     &p_aout->mixer_format,
                                     &p_aout->output.output ) < 0 )
    {
        msg_Err( p_aout, "couldn't create audio output pipeline" );
        module_unneed( p_aout, p_aout->output.p_module );
        p_aout->output.p_module = NULL;
        return -1;
    }
    return 0;
}

/*****************************************************************************
 * aout_OutputDelete : delete the output
 *****************************************************************************
 * This function is entered with the mixer lock.
 *****************************************************************************/
void aout_OutputDelete( aout_instance_t * p_aout )
{
    if( p_aout->output.p_module == NULL )
        return;
    module_unneed( p_aout, p_aout->output.p_module );
    p_aout->output.p_module = NULL;

    aout_FiltersDestroyPipeline( p_aout->output.pp_filters,
                                 p_aout->output.i_nb_filters );

    aout_lock_output_fifo( p_aout );
    aout_FifoDestroy( &p_aout->output.fifo );
    aout_unlock_output_fifo( p_aout );
}

/*****************************************************************************
 * aout_OutputPlay : play a buffer
 *****************************************************************************
 * This function is entered with the mixer lock.
 *****************************************************************************/
void aout_OutputPlay( aout_instance_t * p_aout, aout_buffer_t * p_buffer )
{
    aout_FiltersPlay( p_aout->output.pp_filters, p_aout->output.i_nb_filters,
                      &p_buffer );

    if( !p_buffer )
        return;
    if( p_buffer->i_buffer == 0 )
    {
        block_Release( p_buffer );
        return;
    }

    aout_lock_output_fifo( p_aout );
    aout_FifoPush( &p_aout->output.fifo, p_buffer );
    p_aout->output.pf_play( p_aout );
    aout_unlock_output_fifo( p_aout );
}

/*****************************************************************************
 * aout_OutputNextBuffer : give the audio output plug-in the right buffer
 *****************************************************************************
 * If b_can_sleek is 1, the aout core functions won't try to resample
 * new buffers to catch up - that is we suppose that the output plug-in can
 * compensate it by itself. S/PDIF outputs should always set b_can_sleek = 1.
 * This function is entered with no lock at all :-).
 *****************************************************************************/
aout_buffer_t * aout_OutputNextBuffer( aout_instance_t * p_aout,
                                       mtime_t start_date,
                                       bool b_can_sleek )
{
    aout_fifo_t *p_fifo = &p_aout->output.fifo;
    aout_buffer_t * p_buffer;
    mtime_t now = mdate();

    aout_lock_output_fifo( p_aout );

    /* Drop the audio sample if the audio output is really late.
     * In the case of b_can_sleek, we don't use a resampler so we need to be
     * a lot more severe. */
    while( ((p_buffer = p_fifo->p_first) != NULL)
     && p_buffer->i_pts < (b_can_sleek ? start_date : now) - AOUT_PTS_TOLERANCE )
    {
        msg_Dbg( p_aout, "audio output is too slow (%"PRId64"), "
                 "trashing %"PRId64"us", now - p_buffer->i_pts,
                 p_buffer->i_length );
        aout_BufferFree( aout_FifoPop( p_fifo ) );
    }

    if( p_buffer == NULL )
    {
#if 0 /* This is bad because the audio output might just be trying to fill
       * in its internal buffers. And anyway, it's up to the audio output
       * to deal with this kind of starvation. */

        /* Set date to 0, to allow the mixer to send a new buffer ASAP */
        aout_FifoSet( &p_aout->output.fifo, 0 );
        if ( !p_aout->output.b_starving )
            msg_Dbg( p_aout,
                 "audio output is starving (no input), playing silence" );
        p_aout->output.b_starving = true;
#endif
        aout_unlock_output_fifo( p_aout );
        return NULL;
    }

    mtime_t delta = start_date - p_buffer->i_pts;
    /* Here we suppose that all buffers have the same duration - this is
     * generally true, and anyway if it's wrong it won't be a disaster.
     */
    if ( 0 > delta + p_buffer->i_length )
    /*
     *                   + AOUT_PTS_TOLERANCE )
     * There is no reason to want that, it just worsen the scheduling of
     * an audio sample after an output starvation (ie. on start or on resume)
     * --Gibalou
     */
    {
        if ( !p_aout->output.b_starving )
            msg_Dbg( p_aout, "audio output is starving (%"PRId64"), "
                     "playing silence", -delta );
        p_aout->output.b_starving = true;
        aout_unlock_output_fifo( p_aout );
        return NULL;
    }

    p_aout->output.b_starving = false;
    p_buffer = aout_FifoPop( p_fifo );

    if( !b_can_sleek )
    {
        if( delta > AOUT_PTS_TOLERANCE || delta < -AOUT_PTS_TOLERANCE )
        {
            aout_unlock_output_fifo( p_aout );
            /* Try to compensate the drift by doing some resampling. */
            msg_Warn( p_aout, "output date isn't PTS date, requesting "
                      "resampling (%"PRId64")", delta );

            aout_lock_input_fifos( p_aout );
            aout_lock_output_fifo( p_aout );
            aout_FifoMoveDates( &p_aout->p_input->mixer.fifo, delta );
            aout_FifoMoveDates( p_fifo, delta );
            aout_unlock_input_fifos( p_aout );
        }
    }
    aout_unlock_output_fifo( p_aout );
    return p_buffer;
}
