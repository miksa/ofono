/*
 *
 *  oFono - Open Source Telephony
 *
 *  Copyright (C) 2008-2010  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <libudev.h>

#include <glib.h>

#define OFONO_API_SUBJECT_TO_CHANGE
#include <ofono/plugin.h>
#include <ofono/modem.h>
#include <ofono/log.h>

struct modem_info {
	char *syspath;
	char *devname;
	char *driver;
	GSList *devices;
	struct ofono_modem *modem;
};

struct device_info {
	char *devpath;
	char *devnode;
	char *interface;
	char *number;
	char *label;
};

static gboolean setup_gobi(struct modem_info *modem)
{
	const char *device = NULL;
	GSList *list;

	DBG("%s", modem->syspath);

	for (list = modem->devices; list; list = list->next) {
		struct device_info *info = list->data;

		DBG("%s %s %s %s", info->devnode, info->interface,
						info->number, info->label);

		if (g_strcmp0(info->interface, "255/255/255") == 0 &&
					g_strcmp0(info->number, "02") == 0)
			device = info->devnode;
	}

	if (device == NULL)
		return FALSE;

	DBG("device=%s", device);

	ofono_modem_set_string(modem->modem, "Device", device);

	return TRUE;
}

static gboolean setup_sierra(struct modem_info *modem)
{
	const char *device = NULL;
	GSList *list;

	DBG("%s", modem->syspath);

	for (list = modem->devices; list; list = list->next) {
		struct device_info *info = list->data;

		DBG("%s %s %s %s", info->devnode, info->interface,
						info->number, info->label);

		if (g_strcmp0(info->interface, "255/255/255") == 0 &&
					g_strcmp0(info->number, "03") == 0)
			device = info->devnode;
	}

	if (device == NULL)
		return FALSE;

	DBG("device=%s", device);

	ofono_modem_set_string(modem->modem, "Device", device);

	return TRUE;
}

static gboolean setup_huawei(struct modem_info *modem)
{
	const char *mdm = NULL, *pcui = NULL;
	GSList *list;

	DBG("%s", modem->syspath);

	for (list = modem->devices; list; list = list->next) {
		struct device_info *info = list->data;

		DBG("%s %s %s %s", info->devnode, info->interface,
						info->number, info->label);

		if (g_strcmp0(info->label, "modem") == 0 ||
				g_strcmp0(info->interface, "255/1/1") == 0 ||
				g_strcmp0(info->interface, "255/2/1") == 0)
			mdm = info->devnode;
		else if (g_strcmp0(info->label, "pcui") == 0 ||
				g_strcmp0(info->interface, "255/1/2") == 0 ||
				g_strcmp0(info->interface, "255/2/2") == 0)
			pcui = info->devnode;
		else if (g_strcmp0(info->interface, "255/255/255") == 0) {
			if (g_strcmp0(info->number, "00") == 0)
				mdm = info->devnode;
			else if (g_strcmp0(info->number, "02") == 0)
				pcui = info->devnode;
			else if (g_strcmp0(info->number, "03") == 0)
				pcui = info->devnode;
			else if (g_strcmp0(info->number, "04") == 0)
				pcui = info->devnode;
		}
	}

	if (mdm == NULL || pcui == NULL)
		return FALSE;

	DBG("modem=%s pcui=%s", mdm, pcui);

	ofono_modem_set_string(modem->modem, "Modem", mdm);
	ofono_modem_set_string(modem->modem, "Pcui", pcui);

	return TRUE;
}

static gboolean setup_novatel(struct modem_info *modem)
{
	const char *aux = NULL, *mdm = NULL;
	GSList *list;

	DBG("%s", modem->syspath);

	for (list = modem->devices; list; list = list->next) {
		struct device_info *info = list->data;

		DBG("%s %s %s %s", info->devnode, info->interface,
						info->number, info->label);

		if (g_strcmp0(info->label, "aux") == 0)
			aux = info->devnode;
		else if (g_strcmp0(info->label, "modem") == 0)
			mdm = info->devnode;
		else if (g_strcmp0(info->interface, "255/255/255") == 0) {
			if (g_strcmp0(info->number, "00") == 0)
				aux = info->devnode;
			else if (g_strcmp0(info->number, "01") == 0)
				mdm = info->devnode;
		}
	}

	if (aux == NULL || mdm == NULL)
		return FALSE;

	DBG("aux=%s modem=%s", aux, mdm);

	ofono_modem_set_string(modem->modem, "PrimaryDevice", aux);
	ofono_modem_set_string(modem->modem, "SecondaryDevice", mdm);

	return TRUE;
}

static gboolean setup_zte(struct modem_info *modem)
{
	const char *aux = NULL, *mdm = NULL;
	GSList *list;

	DBG("%s", modem->syspath);

	for (list = modem->devices; list; list = list->next) {
		struct device_info *info = list->data;

		DBG("%s %s %s %s", info->devnode, info->interface,
						info->number, info->label);

		if (g_strcmp0(info->label, "aux") == 0)
			aux = info->devnode;
		else if (g_strcmp0(info->label, "modem") == 0)
			mdm = info->devnode;
		else if (g_strcmp0(info->interface, "255/255/255") == 0) {
			if (g_strcmp0(info->number, "01") == 0)
				aux = info->devnode;
			else if (g_strcmp0(info->number, "02") == 0)
				mdm = info->devnode;
			else if (g_strcmp0(info->number, "03") == 0)
				mdm = info->devnode;
		}
	}

	if (aux == NULL || mdm == NULL)
		return FALSE;

	DBG("aux=%s modem=%s", aux, mdm);

	ofono_modem_set_string(modem->modem, "Aux", aux);
	ofono_modem_set_string(modem->modem, "Modem", mdm);

	return TRUE;
}

static struct {
	const char *name;
	gboolean (*setup)(struct modem_info *modem);
} driver_list[] = {
	{ "gobi",	setup_gobi	},
	{ "sierra",	setup_sierra	},
	{ "huawei",	setup_huawei	},
	{ "huaweicdma",	setup_huawei	},
	{ "novatel",	setup_novatel	},
	{ "zte",	setup_zte	},
	{ }
};

static GHashTable *modem_list;

static void destroy_modem(gpointer data)
{
	struct modem_info *modem = data;
	GSList *list;

	DBG("%s", modem->syspath);

	ofono_modem_remove(modem->modem);

	for (list = modem->devices; list; list = list->next) {
		struct device_info *info = list->data;

		DBG("%s", info->devnode);

		g_free(info->devpath);
		g_free(info->devnode);
		g_free(info->interface);
		g_free(info->number);
		g_free(info->label);
		g_free(info);

		list->data = NULL;
	}

	g_slist_free(modem->devices);

	g_free(modem->syspath);
	g_free(modem->devname);
	g_free(modem->driver);
	g_free(modem);
}

static gboolean check_remove(gpointer key, gpointer value, gpointer user_data)
{
	struct modem_info *modem = value;
	const char *devpath = user_data;
	GSList *list;

	for (list = modem->devices; list; list = list->next) {
		struct device_info *info = list->data;

		if (g_strcmp0(info->devpath, devpath) == 0)
			return TRUE;
	}

	return FALSE;
}

static void remove_device(struct udev_device *device)
{
	const char *syspath;

	syspath = udev_device_get_syspath(device);
	if (syspath == NULL)
		return;

	DBG("%s", syspath);

	g_hash_table_foreach_remove(modem_list, check_remove, (char *) syspath);
}

static gint compare_device(gconstpointer a, gconstpointer b)
{
	const struct device_info *info1 = a;
	const struct device_info *info2 = b;

	return g_strcmp0(info1->devnode, info2->devnode);
}

static void add_device(const char *syspath, const char *devname,
			const char *driver, struct udev_device *device)
{
	struct udev_device *intf;
	const char *devpath, *devnode, *interface, *number, *label;
	struct modem_info *modem;
	struct device_info *info;

	devpath = udev_device_get_syspath(device);
	if (devpath == NULL)
		return;

	devnode = udev_device_get_devnode(device);
	if (devnode == NULL)
		return;

	intf = udev_device_get_parent_with_subsystem_devtype(device,
						"usb", "usb_interface");
	if (intf == NULL)
		return;

	interface = udev_device_get_property_value(intf, "INTERFACE");
	number = udev_device_get_sysattr_value(intf, "bInterfaceNumber");

	label = udev_device_get_property_value(device, "OFONO_LABEL");

	DBG("%s", devpath);
	DBG("%s (%s) %s [%s] ==> %s", devnode, driver,
					interface, number, label);

	modem = g_hash_table_lookup(modem_list, syspath);
	if (modem == NULL) {
		modem = g_try_new0(struct modem_info, 1);
		if (modem == NULL)
			return;

		modem->syspath = g_strdup(syspath);
		modem->devname = g_strdup(devname);
		modem->driver = g_strdup(driver);

		g_hash_table_replace(modem_list, modem->syspath, modem);
	}

	info = g_try_new0(struct device_info, 1);
	if (info == NULL)
		return;

	info->devpath = g_strdup(devpath);
	info->devnode = g_strdup(devnode);
	info->interface = g_strdup(interface);
	info->number = g_strdup(number);
	info->label = g_strdup(label);

	modem->devices = g_slist_insert_sorted(modem->devices, info,
							compare_device);
}

static void check_device(struct udev_device *device)
{
	struct udev_device *usb_device;
	const char *bus, *driver, *syspath, *devname;

	bus = udev_device_get_property_value(device, "ID_BUS");
	if (bus == NULL)
		return;

	if (g_str_equal(bus, "usb") == FALSE)
		return;

	usb_device = udev_device_get_parent_with_subsystem_devtype(device,
							"usb", "usb_device");
	if (usb_device == NULL)
		return;

	syspath = udev_device_get_syspath(usb_device);
	if (syspath == NULL)
		return;

	devname = udev_device_get_devnode(usb_device);
	if (devname == NULL)
		return;

	driver = udev_device_get_property_value(usb_device, "OFONO_DRIVER");
	if (driver == NULL)
		return;

	add_device(syspath, devname, driver, device);
}

static void create_modem(gpointer key, gpointer value, gpointer user_data)
{
	struct modem_info *modem = value;
	const char *syspath = key;
	unsigned int i;

	if (modem->devices == NULL)
		return;

	if (modem->modem != NULL)
		return;

	DBG("%s", syspath);

	modem->modem = ofono_modem_create(NULL, modem->driver);
	if (modem->modem == NULL)
		return;

	for (i = 0; driver_list[i].name; i++) {
		if (g_str_equal(driver_list[i].name, modem->driver) == FALSE)
			continue;

		if (driver_list[i].setup(modem) == TRUE) {
			ofono_modem_register(modem->modem);
			return;
		}
	}

	ofono_modem_remove(modem->modem);
	modem->modem = NULL;
}

static void enumerate_devices(struct udev *context)
{
	struct udev_enumerate *enumerate;
	struct udev_list_entry *entry;

	DBG("");

	enumerate = udev_enumerate_new(context);
	if (enumerate == NULL)
		return;

	udev_enumerate_add_match_subsystem(enumerate, "tty");
	udev_enumerate_add_match_subsystem(enumerate, "net");

	udev_enumerate_scan_devices(enumerate);

	entry = udev_enumerate_get_list_entry(enumerate);
	while (entry) {
		const char *syspath = udev_list_entry_get_name(entry);
		struct udev_device *device;

		device = udev_device_new_from_syspath(context, syspath);
		if (device != NULL) {
			check_device(device);
			udev_device_unref(device);
		}

		entry = udev_list_entry_get_next(entry);
	}

	udev_enumerate_unref(enumerate);

	g_hash_table_foreach(modem_list, create_modem, NULL);
}

static struct udev *udev_ctx;
static struct udev_monitor *udev_mon;
static guint udev_watch = 0;
static guint udev_delay = 0;

static gboolean check_modem_list(gpointer user_data)
{
	udev_delay = 0;

	DBG("");

	g_hash_table_foreach(modem_list, create_modem, NULL);

	return FALSE;
}

static gboolean udev_event(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	struct udev_device *device;
	const char *action;

	if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) {
		ofono_warn("Error with udev monitor channel");
		udev_watch = 0;
		return FALSE;
	}

	device = udev_monitor_receive_device(udev_mon);
	if (device == NULL)
		return TRUE;

	action = udev_device_get_action(device);
	if (action == NULL)
		return TRUE;

	if (g_str_equal(action, "add") == TRUE) {
		if (udev_delay > 0)
			g_source_remove(udev_delay);

		check_device(device);

		udev_delay = g_timeout_add_seconds(1, check_modem_list, NULL);
	} else if (g_str_equal(action, "remove") == TRUE)
		remove_device(device);

	udev_device_unref(device);

	return TRUE;
}

static void udev_start(void)
{
	GIOChannel *channel;
	int fd;

	DBG("");

	if (udev_monitor_enable_receiving(udev_mon) < 0) {
		ofono_error("Failed to enable udev monitor");
		return;
	}

	enumerate_devices(udev_ctx);

	fd = udev_monitor_get_fd(udev_mon);

	channel = g_io_channel_unix_new(fd);
	if (channel == NULL)
		return;

	udev_watch = g_io_add_watch(channel,
				G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
							udev_event, NULL);

	g_io_channel_unref(channel);
}

static int detect_init(void)
{
	udev_ctx = udev_new();
	if (udev_ctx == NULL) {
		ofono_error("Failed to create udev context");
		return -EIO;
	}

	udev_mon = udev_monitor_new_from_netlink(udev_ctx, "udev");
	if (udev_mon == NULL) {
		ofono_error("Failed to create udev monitor");
		udev_unref(udev_ctx);
		udev_ctx = NULL;
		return -EIO;
	}

	modem_list = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, destroy_modem);

	udev_monitor_filter_add_match_subsystem_devtype(udev_mon, "tty", NULL);
	udev_monitor_filter_add_match_subsystem_devtype(udev_mon, "net", NULL);

	udev_monitor_filter_update(udev_mon);

	udev_start();

	return 0;
}

static void detect_exit(void)
{
	if (udev_delay > 0)
		g_source_remove(udev_delay);

	if (udev_watch > 0)
		g_source_remove(udev_watch);

	if (udev_ctx == NULL)
		return;

	udev_monitor_filter_remove(udev_mon);

	g_hash_table_destroy(modem_list);

	udev_monitor_unref(udev_mon);
	udev_unref(udev_ctx);
}

OFONO_PLUGIN_DEFINE(udevng, "udev hardware detection", VERSION,
		OFONO_PLUGIN_PRIORITY_DEFAULT, detect_init, detect_exit)