// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * iG-Rainbow-G58M Platform Info Driver
 * Copyright (C) 2025 iWave Global
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/platform_device.h>

#define BSP_VERSION   		"iG-PRHSZ-SC-01-R2.0-REL1.0-SD2.0-Q25.1-L6.12.11"
#define SOM_I2C_BUS        	CONFIG_SOM_I2C_BUS
#define SOM_EEPROM_ADDR      	CONFIG_SOM_EEPROM_ADDR
#define CC_I2C_BUS		CONFIG_CC_I2C_BUS
#define CC_EEPROM_ADDR		CONFIG_CC_EEPROM_ADDR


void print_carrier_board_version(void);
void print_som_version(void);

void print_carrier_board_version(void)
{
	char crb_ver[17] = {0}; // Buffer size plus one for null termination
	char crb_num[13] = {0}; // Buffer size plus one for null termination
	int ret;
	struct i2c_client *client;
	struct i2c_adapter *adapter;

	adapter = i2c_get_adapter(CC_I2C_BUS);
	if (!adapter) {
		pr_err("I2C: Failed to get adapter\n");
		return;
	}

	client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
	if (!client) {
		pr_err("Failed to allocate memory for I2C client\n");
		i2c_put_adapter(adapter);
		return;
	}

	strncpy(client->name, "dummy", I2C_NAME_SIZE);
	client->adapter = adapter;
	client->addr = CC_EEPROM_ADDR;

	/* Read the carrier board version */
	ret = i2c_smbus_read_i2c_block_data(client, 0x20, 16, crb_ver);
	if (ret < 0) {
		pr_err("I2C: Failed to read carrier board version\n");
		goto cleanup;
	}

	/* Read the carrier board number */
	ret = i2c_smbus_read_i2c_block_data(client, 0x30, 12, crb_num);
	if (ret < 0) {
		pr_err("I2C: Failed to read carrier board number\n");
		goto cleanup;
	}

	printk("\tCarrier Version\t: %s-%s\n", crb_ver, crb_num);

cleanup:
	kfree(client);
	i2c_put_adapter(adapter);
}

void print_som_version(void)
  {
	char bsp_ver1[33] = {0}; // Buffer size plus one for null termination
	char bsp_ver2[18] = {0}; // Buffer size plus one for null termination
	char som_serial_ver[20] = {0}; // Buffer size plus one for null termination
	char som_num[20] = {0}; // Buffer size plus one for null termination
	int ret;
	struct i2c_client *client;
	struct i2c_adapter *adapter;


	adapter = i2c_get_adapter(SOM_I2C_BUS);
	if (!adapter)
	    {
		pr_err("I2C: Failed to get adapter\n");
		return;
	    }

	client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
	if (!client)
	    {
		pr_err("Failed to allocate memory for I2C client\n");
		i2c_put_adapter(adapter);
		return;
	    }

	i2c_set_clientdata(client, NULL);
	strncpy(client->name, "dummy", I2C_NAME_SIZE);
	client->adapter = adapter;
	client->addr = SOM_EEPROM_ADDR;
	client->flags = 0;
	client->irq = 0;

	ret = i2c_smbus_read_i2c_block_data(client, 0x40, 32, bsp_ver1);
	if (ret < 0)
	    {
		pr_err("I2C: Failed to read BSP version\n");
		goto cleanup;
	    }

	ret = i2c_smbus_read_i2c_block_data(client, 0x60, 17, bsp_ver2);
	if (ret < 0)
	    {
		pr_err("I2C: Failed to read BSP version\n");
		goto cleanup;
	    }

	ret = i2c_smbus_read_i2c_block_data(client, 0x20, 16, som_serial_ver);
	if (ret < 0)
	    {
		pr_err("I2C: Failed to read SOM version\n");
		goto cleanup;
	    }

	ret = i2c_smbus_read_i2c_block_data(client, 0x30, 12, som_num);
	if (ret < 0)
	    {
		pr_err("I2C: Failed to read SOM Number\n");
		goto cleanup;
	    }

	pr_info("AF4 V48\n");
	pr_info("Board Info:\n");
	pr_info("SOM Version\t: %s-%s\n", som_serial_ver, som_num);
	pr_info("BSP Version (Shipped)\t: %s%s\n", bsp_ver1,bsp_ver2);

cleanup:
	kfree(client);
	i2c_put_adapter(adapter);
}

static int iG58m_probe(struct platform_device *pdev)
    {
	printk("\nBSP Info:\n");
#ifdef CONFIG_BSP_VER_INFO
	printk("\tBSP Version\t: %s\n", BSP_VERSION);
#endif

#ifdef CONFIG_SOM_REV_INFO
	print_som_version();
#endif

#ifdef CONFIG_CC_REV_INFO
	print_carrier_board_version();
#endif
	printk("\n");

	return 0;
   }

static const struct of_device_id iG58m_info_match[] = {
	{ .compatible = "iwave,iG58m-info" },
	{ },
};
MODULE_DEVICE_TABLE(of, iG58m_info_match);

static struct platform_driver iG58m_driver = {
	.driver = {
		.name = "iG58m_info",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(iG58m_info_match),
	},
	.probe = iG58m_probe,
};

static int __init iG58m_info_init(void)
   {
	return platform_driver_register(&iG58m_driver);
   }

static void __exit iG58m_info_exit(void)
   {
	platform_driver_unregister(&iG58m_driver);
   }

late_initcall(iG58m_info_init);
module_exit(iG58m_info_exit);

MODULE_AUTHOR("iWave Global");
MODULE_DESCRIPTION("IG58m Board Info driver");
MODULE_LICENSE("GPL v2");
