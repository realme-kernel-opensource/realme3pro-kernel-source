
/***************************************************
 * File:touch.c
 * VENDOR_EDIT
 * Copyright (c)  2008- 2030  Oppo Mobile communication Corp.ltd.
 * Description:
 *             tp dev
 * Version:1.0:
 * Date created:2016/09/02
 * Author: hao.wang@Bsp.Driver
 * TAG: BSP.TP.Init
*/

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include "oppo_touchscreen/tp_devices.h"
#include "oppo_touchscreen/touchpanel_common.h"
#include <soc/oppo/oppo_project.h>
#include "touch.h"

#define MAX_LIMIT_DATA_LENGTH         100
int g_tp_dev_vendor = TP_UNKNOWN;
char *g_tp_chip_name;
static bool is_tp_type_got_in_match = false;    /*indicate whether the tp type is got in the process of ic match*/
#define MAX_LIMIT_DATA_LENGTH         100

#define NT36672_NF_CHIP_NAME "NT_NF36672"
#define HX83112A_NF_CHIP_NAME "HX_NF83112A"
#define TD4320_NF_CHIP_NAME "TD4320_NF"


/*if can not compile success, please update vendor/oppo_touchsreen*/
struct tp_dev_name tp_dev_names[] = {
     {TP_OFILM, "OFILM"},
     {TP_BIEL, "BIEL"},
     {TP_TRULY, "TRULY"},
     {TP_BOE, "BOE"},
     {TP_G2Y, "G2Y"},
     {TP_TPK, "TPK"},
     {TP_JDI, "JDI"},
     {TP_TIANMA, "TIANMA"},
     {TP_SAMSUNG, "SAMSUNG"},
     {TP_DSJM, "DSJM"},
     {TP_BOE_B8, "BOEB8"},
     {TP_UNKNOWN, "UNKNOWN"},
};

#define GET_TP_DEV_NAME(tp_type) ((tp_dev_names[tp_type].type == (tp_type))?tp_dev_names[tp_type].name:"UNMATCH")
bool __init tp_judge_ic_match(char * tp_ic_name)
{
    pr_err("[TP] tp_ic_name = %s \n", tp_ic_name);
    pr_err("[TP] boot_command_line = %s \n", boot_command_line);

    switch(get_project()) {
    case 18621:
		pr_err("Project Name is 18621\n");
        is_tp_type_got_in_match = true;
        
            g_tp_dev_vendor = TP_DSJM;
       
            g_tp_chip_name = kzalloc(sizeof(HX83112A_NF_CHIP_NAME), GFP_KERNEL);
            g_tp_chip_name = HX83112A_NF_CHIP_NAME;
            
            return true;
        
    default:
       pr_err("Invalid project\n");
        break;
    }

    pr_err("Lcd module not found\n");
    return false;
}

int tp_util_get_vendor(struct hw_resource *hw_res, struct panel_info *panel_data)
{
    char* vendor;
    int prj_id = 0;
	#ifdef CONFIG_TOUCHPANEL_MULTI_NOFLASH
    if (g_tp_chip_name != NULL) {
        panel_data->chip_name = g_tp_chip_name;
    }
    #endif

    panel_data->test_limit_name = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
    if (panel_data->test_limit_name == NULL) {
        pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
    }

    if (is_project(OPPO_18081) || is_project(OPPO_18085) || is_project(OPPO_18181)) {
        panel_data->tp_type = TP_SAMSUNG;
        prj_id = OPPO_18081;
    } else if (is_project(OPPO_18097)) {
        panel_data->tp_type = TP_SAMSUNG;
        prj_id = OPPO_18097;
    } else if(is_project(OPPO_18621)){
    	panel_data->tp_type = TP_DSJM;
		prj_id = OPPO_18621;
    }
    if (panel_data->tp_type == TP_UNKNOWN) {
        pr_err("[TP]%s type is unknown\n", __func__);
        return 0;
    }

    vendor = GET_TP_DEV_NAME(panel_data->tp_type);

    strcpy(panel_data->manufacture_info.manufacture, vendor);
    snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
            "tp/%d/FW_%s_%s.img",
            prj_id, panel_data->chip_name, vendor);

    if (panel_data->test_limit_name) {
        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
            "tp/%d/LIMIT_%s_%s.img",
            prj_id, panel_data->chip_name, vendor);
    }
    panel_data->manufacture_info.fw_path = panel_data->fw_name;

    pr_info("[TP]vendor:%s fw:%s limit:%s\n",
        vendor,
        panel_data->fw_name,
        panel_data->test_limit_name==NULL?"NO Limit":panel_data->test_limit_name);
	 switch(get_project()) {
    case OPPO_18621:
        panel_data->firmware_headfile.firmware_data = FW_18621_HX83112A_NF_DSJM;
        panel_data->firmware_headfile.firmware_size = sizeof(FW_18621_HX83112A_NF_DSJM);
        break;

    default:
        panel_data->firmware_headfile.firmware_data = NULL;
        panel_data->firmware_headfile.firmware_size = 0;
    }
    return 0;
}
