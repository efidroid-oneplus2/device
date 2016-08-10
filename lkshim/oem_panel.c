/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <debug.h>
#include <string.h>
#include <err.h>
#include <smem.h>
#include <msm_panel.h>
#include <board.h>
#include <mipi_dsi.h>
#include <qtimer.h>
#include <platform.h>

#include "include/panel.h"
#include "panel_display.h"

/*---------------------------------------------------------------------------*/
/* GCDB Panel Database                                                       */
/*---------------------------------------------------------------------------*/
#include "include/panel_jd35695_1080p_video.h"
#include "include/panel_jd35695_1080p_cmd.h"

/*---------------------------------------------------------------------------*/
/* static panel selection variable                                           */
/*---------------------------------------------------------------------------*/
enum {
	JD35695_1080P_VIDEO_PANEL,
	JD35695_1080P_CMD_PANEL,
	UNKNOWN_PANEL
};

/*
 * The list of panels that are supported on this target.
 * Any panel in this list can be selected using fastboot oem command.
 */
static struct panel_list supp_panels[] = {
	{"jd35695_1080p_video", JD35695_1080P_VIDEO_PANEL},
	{"jd35695_1080p_cmd",   JD35695_1080P_CMD_PANEL},
};

static uint32_t panel_id;

int oem_panel_rotation() {
	return NO_ERROR;
}

int oem_panel_on() {
	return NO_ERROR;
}

int oem_panel_off() {
	return NO_ERROR;
}

static bool init_panel_data(struct panel_struct *panelstruct,
			struct msm_panel_info *pinfo,
			struct mdss_dsi_phy_ctrl *phy_db)
{
	int pan_type;

	phy_db->is_pll_20nm = 1;
	
	// TODO: handle both VIDEO and CMD panel
	panel_id = JD35695_1080P_VIDEO_PANEL;

	switch (panel_id) {
		case JD35695_1080P_CMD_PANEL:
			pan_type = PANEL_TYPE_DSI;
			pinfo->lcd_reg_en = 0;
			panelstruct->paneldata    = &jd35695_1080p_cmd_panel_data;
			panelstruct->panelres     = &jd35695_1080p_cmd_panel_res;
			panelstruct->color        = &jd35695_1080p_cmd_color;
			panelstruct->videopanel   = &jd35695_1080p_cmd_video_panel;
			panelstruct->commandpanel = &jd35695_1080p_cmd_command_panel;
			panelstruct->state        = &jd35695_1080p_cmd_state;
			panelstruct->laneconfig   = &jd35695_1080p_cmd_lane_config;
			panelstruct->paneltiminginfo
				= &jd35695_1080p_cmd_timing_info;
			panelstruct->panelresetseq
						 = &jd35695_1080p_cmd_panel_reset_seq;
			panelstruct->backlightinfo = &jd35695_1080p_cmd_backlight;
			pinfo->mipi.panel_on_cmds
				= jd35695_1080p_cmd_on_command;
			pinfo->mipi.num_of_panel_on_cmds
				= JD35695_1080P_CMD_ON_COMMAND;
			pinfo->mipi.panel_off_cmds
				= jd35695_1080p_cmd_off_command;
			pinfo->mipi.num_of_panel_off_cmds
				= JD35695_1080P_CMD_OFF_COMMAND;
			memcpy(phy_db->timing,
				jd35695_1080p_cmd_timings, TIMING_SIZE);
			break;
		case JD35695_1080P_VIDEO_PANEL:
			pan_type = PANEL_TYPE_DSI;
			pinfo->lcd_reg_en = 1;
			panelstruct->paneldata    = &jd35695_1080p_video_panel_data;
			panelstruct->panelres     = &jd35695_1080p_video_panel_res;
			panelstruct->color        = &jd35695_1080p_video_color;
			panelstruct->videopanel   = &jd35695_1080p_video_video_panel;
			panelstruct->commandpanel = &jd35695_1080p_video_command_panel;
			panelstruct->state        = &jd35695_1080p_video_state;
			panelstruct->laneconfig   = &jd35695_1080p_video_lane_config;
			panelstruct->paneltiminginfo
				= &jd35695_1080p_video_timing_info;
			panelstruct->panelresetseq
						 = &jd35695_1080p_video_panel_reset_seq;
			panelstruct->backlightinfo = &jd35695_1080p_video_backlight;
			pinfo->mipi.panel_on_cmds
				= jd35695_1080p_video_on_command;
			pinfo->mipi.num_of_panel_on_cmds
				= JD35695_1080P_VIDEO_ON_COMMAND;
			pinfo->mipi.panel_off_cmds
				= jd35695_1080p_video_off_command;
			pinfo->mipi.num_of_panel_off_cmds
				= JD35695_1080P_VIDEO_OFF_COMMAND;
			memcpy(phy_db->timing,
				jd35695_1080p_video_timings, TIMING_SIZE);
			break;
		default:
		case UNKNOWN_PANEL:
			pan_type = PANEL_TYPE_UNKNOWN;
			break;
	}
	return pan_type;
}

int oem_panel_select(const char *panel_name, struct panel_struct *panelstruct,
			struct msm_panel_info *pinfo,
			struct mdss_dsi_phy_ctrl *phy_db)
{
	// TODO: handle both cmd and video panel
	panel_name = "jd35695_1080p_video";
	uint32_t hw_id = board_hardware_id();
	int32_t panel_override_id;

	if (panel_name) {
		panel_override_id = panel_name_to_id(supp_panels,
				ARRAY_SIZE(supp_panels), panel_name);

		if (panel_override_id < 0) {
			dprintf(CRITICAL, "Not able to search the panel:%s\n",
					 panel_name);
		} else if (panel_override_id < UNKNOWN_PANEL) {
			/* panel override using fastboot oem command */
			panel_id = panel_override_id;

			dprintf(INFO, "OEM panel override:%s\n",
					panel_name);
		}
	}

	return init_panel_data(panelstruct, pinfo, phy_db);
}
