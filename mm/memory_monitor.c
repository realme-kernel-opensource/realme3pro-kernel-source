/*
 *Copyright (c)  2018  Guangdong OPPO Mobile Comm Corp., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include "internal.h"
#include  <linux/memory_monitor.h>

struct alloc_wait_para allocwait_para = {0,0,0,0,0,0,0,0};

static bool mem_monitor_enable = false;
static int alloc_wait_h_ms = 500;
static int alloc_wait_l_ms = 100;

void memory_alloc_monitor(unsigned int order, u64 wait_ms)
{
	if (!mem_monitor_enable)
		return;

	if (current_is_fg()) {
		if (wait_ms >= alloc_wait_h_ms)
			allocwait_para.fg_alloc_wait_h_cnt++;
		else if (wait_ms >= alloc_wait_l_ms)
			allocwait_para.fg_alloc_wait_l_cnt++;
		if (allocwait_para.fg_alloc_wait_max_ms < wait_ms) {
			allocwait_para.fg_alloc_wait_max_ms = wait_ms;
			allocwait_para.fg_alloc_wait_max_order = order;
		}
	}

	if (wait_ms >= alloc_wait_h_ms)
		allocwait_para.total_alloc_wait_h_cnt++;
	else if (wait_ms >= alloc_wait_l_ms)
		allocwait_para.total_alloc_wait_l_cnt++;
	if (allocwait_para.total_alloc_wait_max_ms < wait_ms) {
		allocwait_para.total_alloc_wait_max_ms = wait_ms;
		allocwait_para.total_alloc_wait_max_order = order;
	}
}

module_param_named(mem_monitor_enable, mem_monitor_enable, bool, S_IRUGO | S_IWUSR);
module_param_named(alloc_wait_h_ms, alloc_wait_h_ms, int, S_IRUGO | S_IWUSR);
module_param_named(alloc_wait_l_ms, alloc_wait_l_ms, int, S_IRUGO | S_IWUSR);
