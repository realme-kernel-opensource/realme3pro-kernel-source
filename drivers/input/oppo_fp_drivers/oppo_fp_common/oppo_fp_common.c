/************************************************************************************
 ** File: - SDM670.LA.1.0\android\kernel\msm-4.4\drivers\soc\oppo\oppo_fp_common\oppo_fp_common.c
 ** VENDOR_EDIT
 ** Copyright (C), 2008-2017, OPPO Mobile Comm Corp., Ltd
 **
 ** Description:
 **      fp_common compatibility configuration
 **
 ** Version: 1.0
 ** Date created: 15:03:11,23/05/2018
 ** Author: Ran.Chen@Prd.BaseDrv
 **
 ** --------------------------- Revision History: --------------------------------
 **  <author>         <data>         <desc>
 **  Ran.Chen       2018/05/23     create the file,add goodix_optical_95s
 **  Ran.Chen       2018/05/28     add for fpc1023
 **  Ran.Chen       2018/06/15     add for Silead_Optical_fp
 **  Ran.Chen       2018/06/25     modify fpc1023+2060 to fpc1023_glass
 **  Ziqing.Guo     2018/07/12     add Silead_Optical_fp for 18181 18385
 **  Ziqing.Guo     2018/07/13     set silead fp for 18181 18385 at this moment, will modify later
 **  Ran.Chen       2018/08/01     modify for 18385 fp_id
 **  Long.Liu       2018/09/29     add 18531 FPC1023 fp_id
 **  Dongnan.Wu     2018/10/23	   modify for 17081(android P) fp_id
 **  Mingzhi.guo    2018/11/12     add fpc1511 for 18621
 **  Mingzhi.guo    2018/11/23     temply disable id-check for 18621
 **  Mingzhi.guo    2019/01/22     add fpc1511 for 18637
 ************************************************************************************/

#include <linux/module.h>
#include <linux/proc_fs.h>
#if CONFIG_OPPO_FINGERPRINT_PLATFORM == 6763 || CONFIG_OPPO_FINGERPRINT_PLATFORM == 6771 || CONFIG_OPPO_FINGERPRINT_PLATFORM == 6779
#include <sec_boot_lib.h>
#elif CONFIG_OPPO_FINGERPRINT_PLATFORM == 855
#include <linux/uaccess.h>
#else
#include <soc/qcom/smem.h>
#endif
#include <soc/oppo/oppo_project.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/string.h>
#include "../include/oppo_fp_common.h"

#define CHIP_PRIMAX     "primax"
#define CHIP_CT         "CT"
#define CHIP_OFILM      "ofilm"
#define CHIP_QTECH      "Qtech"
#define CHIP_TRULY      "truly"

#define CHIP_GOODIX     "G"
#define CHIP_FPC        "F"
#define CHIP_SILEAD     "S"

#define CHIP_UNKNOWN    "unknown"

#define ENGINEER_MENU_FPC1140  "-1,-1"  /* content in " " represents SNR,inclination test item in order in engineer menu, and -1/1 means off/on */
#define ENGINEER_MENU_FPC1022  "-1,-1"
#define ENGINEER_MENU_FPC1023  "1,-1"
#define ENGINEER_MENU_FPC1260  "1,-1"
#define ENGINEER_MENU_FPC1270  "-1,-1"
#define ENGINEER_MENU_FPC1511  "-1,-1"
#define ENGINEER_MENU_GOODIX   "1,1"
#define ENGINEER_MENU_GOODIX_3268   "-1,-1"
#define ENGINEER_MENU_GOODIX_5288   "-1,-1"
#define ENGINEER_MENU_GOODIX_5228   "-1,-1"
#define ENGINEER_MENU_GOODIX_OPTICAL   "-1,-1"
#define ENGINEER_MENU_SILEAD_OPTICAL   "-1,-1"
#define ENGINEER_MENU_GOODIX_5298   "-1,-1"
#define ENGINEER_MENU_DEFAULT  "-1,-1"

static struct proc_dir_entry *fp_id_dir = NULL;
static char *fp_id_name = "fp_id";
static struct proc_dir_entry *oppo_fp_common_dir = NULL;
static char *oppo_fp_common_dir_name = "oppo_fp_common";

static char fp_manu[FP_ID_MAX_LENGTH] = CHIP_UNKNOWN; /* the length of this string should be less than FP_ID_MAX_LENGTH */

static struct fp_data *fp_data_ptr = NULL;
char g_engineermode_menu_config[ENGINEER_MENU_SELECT_MAXLENTH] = ENGINEER_MENU_DEFAULT;
#ifdef CONFIG_OPTICAL_IRQ_HANDLER
extern int silfp_opticalfp_irq_handler(struct fp_underscreen_info* tp_info);
extern int gf_opticalfp_irq_handler(struct fp_underscreen_info* tp_info);
#endif
#ifdef CONFIG_OPTICAL_IRQ_HANDLER_SILEAD
extern int silfp_opticalfp_irq_handler(struct fp_underscreen_info* tp_info);
#endif
#if CONFIG_OPPO_FINGERPRINT_PROJCT == 18531 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18151 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18161
fp_module_config_t fp_module_config_list[] = {
    {{0, -1, -1},  FP_FPC_1023_GLASS,    	CHIP_FPC,     ENGINEER_MENU_FPC1023},
    {{1, -1, -1},  FP_FPC_1511,		CHIP_FPC,	  ENGINEER_MENU_FPC1511},
};
#elif CONFIG_OPPO_FINGERPRINT_PROJCT == 17081
fp_module_config_t fp_module_config_list[] = {
    {{0, 0, 0},  FP_FPC_1270, CHIP_FPC, ENGINEER_MENU_FPC1270},
    {{0, 1, 0},  FP_FPC_1023, CHIP_FPC, ENGINEER_MENU_FPC1023},
    {{0, 1, 1},  FP_FPC_1022, CHIP_FPC, ENGINEER_MENU_FPC1022},
    {{1, 0, 0},  FP_FPC_1023, CHIP_FPC, ENGINEER_MENU_FPC1023},
    {{1, 0, 1},  FP_GOODIX_3268, CHIP_GOODIX, ENGINEER_MENU_GOODIX_3268},
    {{1, 1, 0},  FP_GOODIX_5288, CHIP_GOODIX, ENGINEER_MENU_GOODIX_5288},
    {{1, 1, 1},  FP_FPC_1022, CHIP_FPC, ENGINEER_MENU_FPC1022},
};
#elif CONFIG_OPPO_FINGERPRINT_PROJCT == 18621 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18637
fp_module_config_t fp_module_config_list[] = {
    {{1, 1, 0},  FP_FPC_1511, CHIP_FPC, ENGINEER_MENU_FPC1511},
};
#else
fp_module_config_t fp_module_config_list[] = {
    {{0, 0, 1},  FP_FPC_1023_GLASS,    	CHIP_FPC,     ENGINEER_MENU_FPC1023},
    {{0, 1, 0},  FP_FPC_1023,    		CHIP_FPC,     ENGINEER_MENU_FPC1023},//for EVT
    //{{1, 0, 1},  FP_GOODIX_5228, 		CHIP_GOODIX,  ENGINEER_MENU_GOODIX_5228},
    {{1, 0, 1},  FP_GOODIX_5298_GLASS, CHIP_GOODIX, ENGINEER_MENU_GOODIX_5298},
    {{1, 0, 0},  FP_GOODIX_5298, CHIP_GOODIX, ENGINEER_MENU_GOODIX_5298},
    {{1, 1, 0},  FP_SILEAD_OPTICAL_70,  CHIP_SILEAD,  ENGINEER_MENU_SILEAD_OPTICAL},
    {{1, 1, 1},  FP_GOODIX_OPTICAL_95, 	CHIP_GOODIX,  ENGINEER_MENU_GOODIX_OPTICAL},//add for T0

};
#endif

static int fp_request_named_gpio(struct fp_data *fp_data,
        const char *label, int *gpio)
{
    struct device *dev = fp_data->dev;
    struct device_node *np = dev->of_node;

    int ret = of_get_named_gpio(np, label, 0);
    if (ret < 0) {
        dev_err(dev, "failed to get '%s'\n", label);
        return FP_ERROR_GPIO;
    }

    *gpio = ret;
    ret = devm_gpio_request(dev, *gpio, label);
    if (ret) {
        dev_err(dev, "failed to request gpio %d\n", *gpio);
        devm_gpio_free(dev, *gpio);
        return FP_ERROR_GPIO;
    }

    dev_err(dev, "%s - gpio: %d\n", label, *gpio);
    return FP_OK;
}

static int fp_gpio_parse_dts(struct fp_data *fp_data)
{
    int ret = FP_OK;

    if (!fp_data) {
        ret = FP_ERROR_GENERAL;
        goto exit;
    }

#if CONFIG_OPPO_FINGERPRINT_PROJCT == 18531 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18151 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18161 ||CONFIG_OPPO_FINGERPRINT_PROJCT == 18073
    ret = fp_request_named_gpio(fp_data, "oppo,fp-id0",
            &fp_data->gpio_id0);
    if (ret) {
        ret = FP_ERROR_GPIO;
        goto exit;
    }
#elif CONFIG_OPPO_FINGERPRINT_PROJCT == 18097 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18099 ||CONFIG_OPPO_FINGERPRINT_PROJCT == 18397 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18041 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18181 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18115 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18501 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18503 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18539
    ret = fp_request_named_gpio(fp_data, "oppo,fp-id2",
            &fp_data->gpio_id2);
    if (ret) {
        ret = FP_ERROR_GPIO;
        goto exit;
    }
#elif CONFIG_OPPO_FINGERPRINT_PROJCT == 18621 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18637
    ret = fp_request_named_gpio(fp_data, "oppo,fp-id2",
            &fp_data->gpio_id2);
    if (ret) {
        ret = FP_ERROR_GPIO;
        goto exit;
    }
#elif CONFIG_OPPO_FINGERPRINT_PROJCT == 17081
    ret = fp_request_named_gpio(fp_data, "oppo,fp-id1",
            &fp_data->gpio_id1);
    if (ret) {
        ret = FP_ERROR_GPIO;
        goto exit;
    }

    ret = fp_request_named_gpio(fp_data, "oppo,fp-id2",
            &fp_data->gpio_id2);
    if (ret) {
        ret = FP_ERROR_GPIO;
        goto exit;
    }

    ret = fp_request_named_gpio(fp_data, "oppo,fp-id3"
        ,&fp_data->gpio_id3);
    if (ret) {
        ret = FP_ERROR_GPIO;
        goto exit;
    }
#else
    ret = fp_request_named_gpio(fp_data, "oppo,fp-id0",
            &fp_data->gpio_id0);
    if (ret) {
        ret = FP_ERROR_GPIO;
        goto exit;
    }

    ret = fp_request_named_gpio(fp_data, "oppo,fp-id1",
            &fp_data->gpio_id1);
    if (ret) {
        ret = FP_ERROR_GPIO;
        goto exit;
    }

    ret = fp_request_named_gpio(fp_data, "oppo,fp-id2",
            &fp_data->gpio_id2);
    if (ret) {
        ret = FP_ERROR_GPIO;
        goto exit;
    }
#endif

exit:
    return ret;
}


static ssize_t fp_id_node_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    char page[15] = { 0 };
    char *p = page;
    int len = 0;

    p += sprintf(p, "%s", fp_manu);
    len = p - page;
    if (len > *pos) {
        len -= *pos;
    }
    else {
        len = 0;
    }

    if (copy_to_user(buf, page, len < count ? len  : count)) {
        return -EFAULT;
    }

    *pos = *pos + (len < count ? len  : count);

    return len < count ? len  : count;
}

static ssize_t fp_id_node_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    size_t local_count;
    if (count <= 0) {
        return 0;
    }
    strcpy(fp_manu, CHIP_UNKNOWN);

    local_count = (FP_ID_MAX_LENGTH - 1) < count ? (FP_ID_MAX_LENGTH - 1) : count;
    if (copy_from_user(fp_manu , buf, local_count) != 0) {
        dev_err(fp_data_ptr->dev, "write fp manu value fail\n");
        return -EFAULT;
    }
    fp_manu[local_count] = '\0';
    dev_info(fp_data_ptr->dev, "write fp manu = %s\n", fp_manu);
    return count;
}

static struct file_operations fp_id_node_ctrl = {
    .read = fp_id_node_read,
    .write = fp_id_node_write,
};

static int fp_get_matched_chip_module(struct device *dev, int fp_id1, int fp_id2, int fp_id3)
{
    int i;
    for (i = 0; i < sizeof(fp_module_config_list)/sizeof(fp_module_config_t); ++i) {
        if ((fp_module_config_list[i].gpio_id_config_list[0] == fp_id1) &&
                (fp_module_config_list[i].gpio_id_config_list[1] == fp_id2) &&
                (fp_module_config_list[i].gpio_id_config_list[2] == fp_id3)) {
            switch (fp_module_config_list[i].fp_vendor_chip) {
                case FP_FPC_1022:
                    strcpy(fp_manu, CHIP_FPC);
                    strcat(fp_manu, "_1022");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_FPC_1022;
                case FP_FPC_1023:
                    strcpy(fp_manu, CHIP_FPC);
                    strcat(fp_manu, "_1023");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_FPC_1023;
                case FP_FPC_1140:
                    strcpy(fp_manu, CHIP_FPC);
                    strcat(fp_manu, "_1140");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_FPC_1140;
                case FP_FPC_1260:
                    strcpy(fp_manu, CHIP_FPC);
                    strcat(fp_manu, "_1260");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_FPC_1260;
                case FP_FPC_1270:
                    strcpy(fp_manu, CHIP_FPC);
                    strcat(fp_manu, "_1270");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_FPC_1270;
                case FP_GOODIX_3268:
                    strcpy(fp_manu, CHIP_GOODIX);
                    strcat(fp_manu, "_3268");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_GOODIX_3268;
                case FP_GOODIX_5288:
                    strcpy(fp_manu, CHIP_GOODIX);
                    strcat(fp_manu, "_5288");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_GOODIX_5288;
                case FP_GOODIX_5298:
                    strcpy(fp_manu, CHIP_GOODIX);
                    strcat(fp_manu, "_5298");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_GOODIX_5298;
                case FP_GOODIX_5298_GLASS:
                    strcpy(fp_manu, CHIP_GOODIX);
                    strcat(fp_manu, "_5298_GLASS");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_GOODIX_5298;
                case FP_GOODIX_5228:
                    strcpy(fp_manu, CHIP_GOODIX);
                    strcat(fp_manu, "_5228");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_GOODIX_5228;
                case FP_GOODIX_OPTICAL_95:
                    strcpy(fp_manu, CHIP_GOODIX);
                    strcat(fp_manu, "_OPTICAL_95");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_GOODIX_OPTICAL_95;
                case FP_SILEAD_OPTICAL_70:
                    strcpy(fp_manu, CHIP_SILEAD);
                    strcat(fp_manu, "_OPTICAL_70");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_SILEAD_OPTICAL_70;
                case FP_FPC_1023_GLASS:
                    strcpy(fp_manu, CHIP_FPC);
                    strcat(fp_manu, "_1023_GLASS");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
                    return FP_FPC_1023_GLASS;
                case FP_FPC_1511:
                    strcpy(fp_manu, CHIP_FPC);
                    strcat(fp_manu, "_1511");
                    strcpy(g_engineermode_menu_config, fp_module_config_list[i].engineermode_menu_config);
					dev_err(dev, "FP_FPC_1511 matched vendor chip!");
                    return FP_FPC_1511;

                default:
                    dev_err(dev, "gpio ids matched but no matched vendor chip!");
                    return FP_UNKNOWN;
            }
        }
    }
    strcpy(fp_manu, CHIP_UNKNOWN);
    strcpy(g_engineermode_menu_config, ENGINEER_MENU_DEFAULT);
    return FP_UNKNOWN;
}

#ifdef CONFIG_OPTICAL_IRQ_HANDLER
int opticalfp_irq_handler(struct fp_underscreen_info* tp_info)
{
    fp_vendor_t fpsensor_type = get_fpsensor_type();
    if (FP_SILEAD_OPTICAL_70 == fpsensor_type) {
        return silfp_opticalfp_irq_handler(tp_info);
    } else if (FP_GOODIX_OPTICAL_95 == fpsensor_type) {
        return gf_opticalfp_irq_handler(tp_info);
    } else {
        return FP_UNKNOWN;
    }
}
#endif
#ifdef CONFIG_OPTICAL_IRQ_HANDLER_SILEAD
int opticalfp_irq_handler(struct fp_underscreen_info* tp_info)
{
    fp_vendor_t fpsensor_type = get_fpsensor_type();
    if (FP_SILEAD_OPTICAL_70 == fpsensor_type) {
        return silfp_opticalfp_irq_handler(tp_info);
    } else {
        return FP_UNKNOWN;
    }
}
#endif
static int fp_register_proc_fs(struct fp_data *fp_data)
{
    uint32_t fp_id_retry;
    fp_id_retry = 0;
#if CONFIG_OPPO_FINGERPRINT_PROJCT == 18181 ||  CONFIG_OPPO_FINGERPRINT_PROJCT == 18385 ||  CONFIG_OPPO_FINGERPRINT_PROJCT == 18097 ||  CONFIG_OPPO_FINGERPRINT_PROJCT == 18099 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18397 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18041 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18115 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18501 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18503 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18539
    fp_data->fp_id0 = 1;
    fp_data->fp_id1 = 1;
    fp_data->fp_id2 = gpio_get_value(fp_data->gpio_id2);
#elif CONFIG_OPPO_FINGERPRINT_PROJCT == 18621 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18637
    fp_data->fp_id0 = 1;
    fp_data->fp_id1 = 1;
    fp_data->fp_id2 = gpio_get_value(fp_data->gpio_id2);
    //dev_err(fp_data->dev, "read fp_id_gpio fp_id2= %d \n", fp_data->fp_id2);
    //fp_data->fp_id2 = 1;
#elif CONFIG_OPPO_FINGERPRINT_PROJCT == 18531 || CONFIG_OPPO_FINGERPRINT_PROJCT == 18151
    fp_data->fp_id0 = gpio_get_value(fp_data->gpio_id0);
    fp_data->fp_id1 = -1;
    fp_data->fp_id2 = -1;
#elif CONFIG_OPPO_FINGERPRINT_PROJCT == 17081
    fp_data->fp_id1 = gpio_get_value(fp_data->gpio_id1);
    fp_data->fp_id2 = gpio_get_value(fp_data->gpio_id2);
    fp_data->fp_id3 = gpio_get_value(fp_data->gpio_id3);
#else
    fp_data->fp_id0 = gpio_get_value(fp_data->gpio_id0);
    fp_data->fp_id1 = gpio_get_value(fp_data->gpio_id1);
    fp_data->fp_id2 = gpio_get_value(fp_data->gpio_id2);
#endif

    dev_err(fp_data->dev, "fp_register_proc_fs check: fp_id0= %d, fp_id1= %d, fp_id2= %d, fp_id3 = %d, fp_id_retry= %d\n", \
            fp_data->fp_id0, fp_data->fp_id1, fp_data->fp_id2, fp_data->fp_id3, fp_id_retry);
#if CONFIG_OPPO_FINGERPRINT_PROJCT == 17081
	fp_data->fpsensor_type = fp_get_matched_chip_module(fp_data->dev, fp_data->fp_id1, fp_data->fp_id2, fp_data->fp_id3);
#else
    fp_data->fpsensor_type = fp_get_matched_chip_module(fp_data->dev, fp_data->fp_id0, fp_data->fp_id1, fp_data->fp_id2);
#endif
    /*  make the proc /proc/fp_id  */
    fp_id_dir = proc_create(fp_id_name, 0666, NULL, &fp_id_node_ctrl);
    if (fp_id_dir == NULL) {
        return FP_ERROR_GENERAL;
    }

    return FP_OK;
}

fp_vendor_t get_fpsensor_type(void)
{
    fp_vendor_t fpsensor_type = FP_UNKNOWN;

    if (NULL == fp_data_ptr) {
        pr_err("%s no device", __func__);
        return FP_UNKNOWN;
    }

    fpsensor_type = fp_data_ptr->fpsensor_type;

    return fpsensor_type;
}


static int oppo_fp_common_probe(struct platform_device *fp_dev)
{
    int ret = 0;
    struct device *dev = &fp_dev->dev;
    struct fp_data *fp_data = NULL;

    fp_data = devm_kzalloc(dev, sizeof(struct fp_data), GFP_KERNEL);
    if (fp_data == NULL) {
        dev_err(dev, "fp_data kzalloc failed\n");
        ret = -ENOMEM;
        goto exit;
    }

    fp_data->dev = dev;
    fp_data_ptr = fp_data;
    ret = fp_gpio_parse_dts(fp_data);
    if (ret) {
        goto exit;
    }

#if CONFIG_OPPO_FINGERPRINT_PLATFORM == 6763 || CONFIG_OPPO_FINGERPRINT_PLATFORM == 6771 || CONFIG_OPPO_FINGERPRINT_PLATFORM == 6779
    msleep(20);
#endif

    ret = fp_register_proc_fs(fp_data);
    if (ret) {
        goto exit;
    }

    return FP_OK;

exit:

    if (oppo_fp_common_dir) {
        remove_proc_entry(oppo_fp_common_dir_name, NULL);
    }

    if (fp_id_dir) {
        remove_proc_entry(fp_id_name, NULL);
    }

    dev_err(dev, "fp_data probe failed ret = %d\n", ret);
    if (fp_data) {
        devm_kfree(dev, fp_data);
    }

    return ret;
}

static int oppo_fp_common_remove(struct platform_device *pdev)
{
    return FP_OK;
}

static struct of_device_id oppo_fp_common_match_table[] = {
    {   .compatible = "oppo,fp_common", },
    {}
};

static struct platform_driver oppo_fp_common_driver = {
    .probe = oppo_fp_common_probe,
    .remove = oppo_fp_common_remove,
    .driver = {
        .name = "oppo_fp_common",
        .owner = THIS_MODULE,
        .of_match_table = oppo_fp_common_match_table,
    },
};

static int __init oppo_fp_common_init(void)
{
    return platform_driver_register(&oppo_fp_common_driver);
}

static void __exit oppo_fp_common_exit(void)
{
    platform_driver_unregister(&oppo_fp_common_driver);
}

#ifdef CONFIG_OPTICAL_IRQ_HANDLER
EXPORT_SYMBOL(opticalfp_irq_handler);
#endif
subsys_initcall(oppo_fp_common_init);
module_exit(oppo_fp_common_exit)
