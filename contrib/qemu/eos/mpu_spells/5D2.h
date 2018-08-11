/**
 * 5D2 MPU spells
 */
static struct mpu_init_spell mpu_init_spells_5D2[] = {
    { { 0x06, 0x04, 0x02, 0x00, 0x00 }, .description = "Init", .out_spells = { /* spell #1 */
        { 0x08, 0x07, 0x01, 0x33, 0x03, 0x03, 0x03, 0x00 },     /* reply #1.1, PROP 80000029 */
        { 0x06, 0x05, 0x01, 0x20, 0x01, 0x00 },                 /* reply #1.2, PROP_CARD1_EXISTS */
        { 0x06, 0x05, 0x01, 0x21, 0x00, 0x00 },                 /* reply #1.3, PROP_CARD2_EXISTS */
        { 0x06, 0x05, 0x01, 0x22, 0x00, 0x00 },                 /* reply #1.4, PROP_CARD3_EXISTS */
        { 0x06, 0x05, 0x03, 0x0c, 0x01, 0x00 },                 /* reply #1.5, PROP_CARD1_RECORD */
        { 0x06, 0x05, 0x03, 0x0d, 0x01, 0x00 },                 /* reply #1.6, PROP_CARD2_RECORD */
        { 0x06, 0x05, 0x03, 0x0e, 0x01, 0x00 },                 /* reply #1.7, PROP_CARD3_RECORD */
        { 0x08, 0x06, 0x01, 0x23, 0x00, 0x00, 0x00 },           /* reply #1.8, PROP_CARD1_STATUS(00) */
        { 0x08, 0x06, 0x01, 0x24, 0x00, 0x00, 0x00 },           /* reply #1.9, PROP_CARD2_STATUS(00) */
        { 0x08, 0x06, 0x01, 0x25, 0x00, 0x00, 0x00 },           /* reply #1.10, PROP_CARD3_STATUS(00) */
        { 0x06, 0x05, 0x01, 0x2e, 0x01, 0x00 },                 /* reply #1.11, PROP_SAVE_MODE */
        { 0x06, 0x05, 0x01, 0x2c, 0x01, 0x00 },                 /* reply #1.12, PROP_CURRENT_MEDIA */
        { 0x06, 0x05, 0x03, 0x20, 0x01, 0x00 },                 /* reply #1.13, PROP_STARTUP_CONDITION */
        { 0x06, 0x05, 0x01, 0x3d, 0x00, 0x00 },                 /* reply #1.14, PROP_TEMP_STATUS */
        { 0x06, 0x05, 0x01, 0x42, 0x00, 0x00 },                 /* reply #1.15, PROP_PHOTO_STUDIO_MODE */
        { 0x06, 0x05, 0x01, 0x43, 0x00, 0x00 },                 /* reply #1.16, PROP 80040017 */
        { 0x06, 0x05, 0x01, 0x46, 0x00, 0x00 },                 /* reply #1.17, PROP_PHOTO_STUDIO_ENABLE_ISOCOMP */
        { 0x06, 0x05, 0x01, 0x44, 0x00, 0x00 },                 /* reply #1.18, PROP 80040018 */
        { 0x06, 0x05, 0x01, 0x00, 0x03, 0x00 },                 /* reply #1.19, PROP_SHOOTING_MODE(03) */
        { 0x2c, 0x2a, 0x02, 0x00, 0x03, 0x03, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x09, 0x17, 0x70, 0x00, 0x00, 0xfe, 0x00, 0x85, 0x07, 0x08, 0x01, 0x03, 0x06, 0x00, 0x00, 0x04, 0x07, 0x08, 0x01, 0x03, 0x01, 0x00, 0x00, 0x07, 0x08, 0x00, 0x73, 0x3b, 0x01 },/* reply #1.20, Init group */
        { 0x06, 0x05, 0x01, 0x37, 0x00, 0x00 },                 /* reply #1.21, PROP_CARD_EXTENSION */
        { 0x06, 0x05, 0x01, 0x49, 0x00, 0x00 },                 /* reply #1.22, PROP_LIVE_VIEW_AF_SYSTEM */
        { 0x06, 0x05, 0x01, 0x3e, 0x00, 0x00 },                 /* reply #1.23, PROP_ELECTRIC_SHUTTER_MODE */
        { 0x08, 0x06, 0x01, 0x45, 0x00, 0x10, 0x00 },           /* reply #1.24, PROP_METERING_TIMER_FOR_LV */
        { 0x06, 0x05, 0x01, 0x48, 0x02, 0x00 },                 /* reply #1.25, PROP_LIVE_VIEW_MOVIE_SELECT */
        { 0x06, 0x05, 0x01, 0x4b, 0x02, 0x00 },                 /* reply #1.26, PROP_LIVE_VIEW_VIEWTYPE_SELECT */
        { 0x06, 0x05, 0x01, 0x40, 0x00, 0x00 },                 /* reply #1.27, PROP_STROBO_ETTLMETER */
        { 0x06, 0x05, 0x01, 0x41, 0x00, 0x00 },                 /* reply #1.28, PROP_STROBO_CURTAIN */
        { 0x06, 0x05, 0x01, 0x3f, 0x00, 0x00 },                 /* reply #1.29, PROP_FLASH_ENABLE */
        { 0x16, 0x14, 0x01, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x0f, 0x00 },/* reply #1.30, PROP_VIDEO_MODE */
        { 0x0e, 0x0c, 0x02, 0x05, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 },/* reply #1.31, PROP_CFN_1 */
        { 0x0c, 0x0a, 0x02, 0x06, 0x06, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00 },/* reply #1.32, PROP_CFN_2 */
        { 0x14, 0x13, 0x02, 0x07, 0x09, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x06, 0x08, 0x00, 0x00, 0x02, 0x00, 0x00 },/* reply #1.33, PROP_CFN_3 */
        { 0x0e, 0x0c, 0x02, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 },/* reply #1.34, PROP_CFN_4 */
        { 0x0a, 0x08, 0x03, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00 },/* reply #1.35, PROP_SPECIAL_OPTION */
        { 0x06, 0x05, 0x03, 0x05, 0x02, 0x00 },                 /* reply #1.36, PROP_POWER_LEVEL */
        { 0x1e, 0x1c, 0x03, 0x30, 0x53, 0x65, 0x53, 0x65, 0x65, 0x65, 0x65, 0x65, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },/* reply #1.37, PROP 8003002A */
        { 0x0e, 0x0c, 0x03, 0x2e, 0x00, 0x00, 0x7c, 0x46, 0x00, 0x00, 0x7e, 0x44, 0x00 },/* reply #1.38, PROP_SHUTTER_COUNTER */
        { 0x06, 0x05, 0x03, 0x35, 0x01, 0x00 },                 /* reply #1.39, PROP_BATTERY_REPORT_COUNTER */
        { 0x1c, 0x1b, 0x03, 0x1d, 0x4f, 0x03, 0x00, 0x00, 0x00, 0x18, 0x00, 0x4c, 0x50, 0x2d, 0x45, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x7e, 0x5b, 0x1c, 0x57, 0x00 },/* reply #1.40, PROP_BATTERY_REPORT */
        { 0x06, 0x04, 0x03, 0x36, 0x00 },                       /* reply #1.41, PROP_BATTERY_REPORT_FINISHED */
        { 0x06, 0x05, 0x01, 0x04, 0x00, 0x00 },                 /* reply #1.42, PROP_AF_MODE */
        { 0x06, 0x05, 0x01, 0x06, 0x3b, 0x00 },                 /* reply #1.43, PROP_APERTURE */
        { 0 } } },
    { { 0x08, 0x06, 0x00, 0x00, 0x02, 0x00, 0x00 }, .description = "Complete WaitID = 0x80000001 Init", .out_spells = { /* spell #2 */
        { 0 } } },
    { { 0x06, 0x05, 0x03, 0x0e, 0x00, 0x00 }, .description = "PROP_CARD3_RECORD", .out_spells = { /* spell #3 */
        { 0 } } },
    { { 0x06, 0x05, 0x01, 0x22, 0x00, 0x00 }, .description = "PROP_CARD3_EXISTS", .out_spells = { /* spell #4 */
        { 0x14, 0x13, 0x02, 0x07, 0x09, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x06, 0x08, 0x00, 0x00, 0x02, 0x00, 0x00 },/* reply #4.1, PROP_CFN_3 */
        { 0x06, 0x05, 0x03, 0x23, 0x08, 0x00 },                 /* reply #4.2 */
        { 0x0e, 0x0c, 0x03, 0x24, 0x31, 0x39, 0x2d, 0x33, 0x35, 0x6d, 0x6d, 0x00, 0x00 },/* reply #4.3, PROP_LENS_NAME: 19-35mm */
        { 0x06, 0x04, 0x03, 0x25, 0x00 },                       /* reply #4.4 */
        { 0x06, 0x05, 0x01, 0x04, 0x00, 0x00 },                 /* reply #4.5, PROP_AF_MODE */
        { 0x06, 0x05, 0x03, 0x0e, 0x00, 0x00 },                 /* reply #4.6, PROP_CARD3_RECORD */
        { 0x06, 0x05, 0x01, 0x22, 0x00, 0x00 },                 /* reply #4.7, PROP_CARD3_EXISTS */
        { 0 } } },
    { { 0x08, 0x06, 0x01, 0x25, 0x00, ARG0, 0x00 }, .description = "PROP_CARD3_STATUS", .out_spells = { /* spell #5 */
        { 0x08, 0x06, 0x01, 0x25, 0x00, ARG0, 0x00 },           /* reply #5.1, PROP_CARD3_STATUS(00) */
        { 0 } } },
    { { 0x06, 0x05, 0x01, 0x37, 0x00, 0x00 }, .description = "PROP_CARD_EXTENSION", .out_spells = { /* spell #6 */
        { 0x06, 0x05, 0x01, 0x37, 0x00, 0x00 },                 /* reply #6.1, PROP_CARD_EXTENSION */
        { 0x0a, 0x08, 0x01, 0x34, 0x08, 0x01, 0x07, 0x03, 0x01 },/* reply #6.2, PROP_CARD1_IMAGE_QUALITY */
        { 0 } } },
    { { 0x08, 0x06, 0x00, 0x00, 0x01, 0x34, 0x00 }, .description = "Complete WaitID = 0x8000002F PROP_CARD1_IMAGE_QUALITY", .out_spells = { /* spell #7 */
        { 0x0a, 0x08, 0x01, 0x35, 0x00, 0x00, 0x06, 0x04, 0x01 },/* reply #7.1, PROP_CARD2_IMAGE_QUALITY */
        { 0 } } },
    { { 0x08, 0x06, 0x00, 0x00, 0x01, 0x35, 0x00 }, .description = "Complete WaitID = 0x80000030 PROP_CARD2_IMAGE_QUALITY", .out_spells = { /* spell #8 */
        { 0x0a, 0x08, 0x01, 0x36, 0x08, 0x01, 0x07, 0x03, 0x01 },/* reply #8.1, PROP_CARD3_IMAGE_QUALITY */
        { 0 } } },
    { { 0x08, 0x06, 0x00, 0x00, 0x01, 0x36, 0x00 }, .description = "Complete WaitID = 0x80000031 PROP_CARD3_IMAGE_QUALITY", .out_spells = { /* spell #9 */
        { 0 } } },
    { { 0x0a, 0x08, 0x03, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 }, .description = "PROP_AVAIL_SHOT", .out_spells = { /* spell #10 */
        { 0 } } },
    { { 0x06, 0x04, 0x03, 0x10, 0x00 }, .description = "PROP 80030008", .out_spells = { /* spell #11 */
        { 0 } } },
    { { 0x06, 0x05, 0x03, 0x07, 0xff, 0x00 }, .description = "PROP_BURST_COUNT", .out_spells = { /* spell #12 */
        { 0 } } },
    { { 0x06, 0x05, 0x01, 0x2e, 0x01, 0x00 }, .description = "PROP_SAVE_MODE", .out_spells = { /* spell #13 */
        { 0x06, 0x05, 0x01, 0x2e, 0x01, 0x00 },                 /* reply #13.1, PROP_SAVE_MODE */
        { 0 } } },
    { { 0x0a, 0x08, 0x03, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00 }, .description = "PROP 80030007", .out_spells = { /* spell #14 */
        { 0 } } },
    { { 0x06, 0x05, 0x04, 0x0e, 0x00, 0x00 }, .description = "PROP 8002000D", .out_spells = { /* spell #15 */
        { 0 } } },
    { { 0x08, 0x06, 0x01, 0x25, 0x00, ARG0, 0x00 }, .description = "PROP_CARD3_STATUS", .out_spells = { /* spell #16 */
        { 0x08, 0x06, 0x01, 0x25, 0x00, ARG0, 0x00 },           /* reply #16.1, PROP_CARD3_STATUS(00) */
        { 0 } } },
    { { 0x08, 0x06, 0x01, 0x23, 0x00, ARG0, 0x00 }, .description = "PROP_CARD1_STATUS", .out_spells = { /* spell #17 */
        { 0x08, 0x06, 0x01, 0x23, 0x00, ARG0, 0x00 },           /* reply #17.1, PROP_CARD1_STATUS(01) */
        { 0 } } },
    { { 0x08, 0x06, 0x01, 0x26, 0x00, ARG0, 0x00 }, .description = "PROP_CARD1_FOLDER_NUMBER", .out_spells = { /* spell #18 */
        { 0 } } },
    { { 0x06, 0x05, 0x03, 0x07, 0x08, 0x00 }, .description = "PROP_BURST_COUNT", .out_spells = { /* spell #19 */
        { 0 } } },
    { { 0x0a, 0x08, 0x03, 0x06, 0x00, 0x00, 0x08, 0x18, 0x00 }, .description = "PROP_AVAIL_SHOT", .out_spells = { /* spell #20 */
        { 0 } } },
    { { 0x08, 0x07, 0x01, 0x29, ARG0, ARG1, 0x00, ARG2 }, .description = "PROP_CARD1_FILE_NUMBER", .out_spells = { /* spell #21 */
        { 0 } } },
    { { 0x06, 0x05, 0x03, 0x11, 0x01, 0x00 }, .description = "PROP_ICU_AUTO_POWEROFF", .out_spells = { /* spell #22 */
        { 0 } } },
    { { 0x06, 0x05, 0x02, 0x0a, 0x00, 0x00 }, .description = "PROP_PERMIT_ICU_EVENT", .out_spells = { /* spell #23 */
        { 0x06, 0x05, 0x01, 0x2c, 0x01, 0x00 },                 /* reply #23.1, PROP_CURRENT_MEDIA */
        { 0x0a, 0x08, 0x03, 0x00, 0xa1, 0x00, 0x00, 0x00, 0x00 },/* reply #23.2, PROP 80030000 */
        { 0x06, 0x05, 0x03, 0x04, 0x00, 0x00 },                 /* reply #23.3, PROP_POWER_KIND */
        { 0x14, 0x12, 0x03, 0x15, 0x01, 0x26, 0x4f, 0x00, 0xa0, 0x00, 0x13, 0x00, 0x23, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00 },/* reply #23.4, PROP_LENS */
        { 0x06, 0x05, 0x03, 0x17, 0x92, 0x00 },                 /* reply #23.5, PROP_EFIC_TEMP */
        { 0x08, 0x06, 0x01, 0x0a, 0x00, 0x01, 0x00 },           /* reply #23.6, PROP_AFPOINT */
        { 0x06, 0x05, 0x01, 0x38, 0x00, 0x00 },                 /* reply #23.7, PROP 80040005 */
        { 0x06, 0x05, 0x01, 0x39, 0x00, 0x00 },                 /* reply #23.8, PROP 80040006 */
        { 0x06, 0x05, 0x01, 0x0f, 0x00, 0x00 },                 /* reply #23.9, PROP 8000000F */
        { 0x06, 0x05, 0x03, 0x23, 0x08, 0x00 },                 /* reply #23.10 */
        { 0x0e, 0x0c, 0x03, 0x24, 0x31, 0x39, 0x2d, 0x33, 0x35, 0x6d, 0x6d, 0x00, 0x00 },/* reply #23.11, PROP_LENS_NAME: 19-35mm */
        { 0x06, 0x04, 0x03, 0x25, 0x00 },                       /* reply #23.12 */
        { 0x06, 0x05, 0x01, 0x3d, 0x00, 0x00 },                 /* reply #23.13, PROP_TEMP_STATUS */
        { 0x14, 0x12, 0x03, 0x15, 0x01, 0x26, 0x4f, 0x00, 0xa0, 0x00, 0x13, 0x00, 0x23, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00 },/* reply #23.14, PROP_LENS */
        { 0 } } },
    { { 0x06, 0x05, 0x03, 0x19, 0x01, 0x00 }, .description = "PROP_TFT_STATUS", .out_spells = { /* spell #24 */
        { 0 } } },
    { { 0x06, 0x05, 0x09, 0x11, 0x01, 0x00 }, .description = "PROP_LV_DISPSIZE", .out_spells = { /* spell #25 */
        { 0 } } },
    { { 0x12, 0x11, 0x09, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, .description = "PROP 80050020", .out_spells = { /* spell #26 */
        { 0 } } },
    { { 0x26, 0x24, 0x09, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, .description = "PROP_LV_FOCUS_DATA", .out_spells = { /* spell #27 */
        { 0 } } },
    { { 0x08, 0x06, 0x03, 0x18, 0x00, 0x00, 0x00 }, .description = "PROP 8003000F", .out_spells = { /* spell #28 */
        { 0 } } },
    { { 0x08, 0x06, 0x03, 0x1f, 0x00, 0x00, 0x00 }, .description = "PROP 80030019", .out_spells = { /* spell #29 */
        { 0 } } },
    { { 0x06, 0x05, 0x03, 0x13, 0x00, 0x00 }, .description = "PROP_LOGICAL_CONNECT", .out_spells = { /* spell #30 */
        { 0 } } },
    { { 0x06, 0x05, 0x03, 0x1e, 0x00, 0x00 }, .description = "PROP 8003001A", .out_spells = { /* spell #31 */
        { 0x06, 0x05, 0x03, 0x35, 0x01, 0x00 },                 /* reply #31.1, PROP_BATTERY_REPORT_COUNTER */
        { 0x1c, 0x1b, 0x03, 0x1d, 0x4f, 0x03, 0x00, 0x00, 0x00, 0x18, 0x00, 0x4c, 0x50, 0x2d, 0x45, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x7e, 0x5b, 0x1c, 0x57, 0x00 },/* reply #31.2, PROP_BATTERY_REPORT */
        { 0x06, 0x04, 0x03, 0x36, 0x00 },                       /* reply #31.3, PROP_BATTERY_REPORT_FINISHED */
        { 0 } } },
    { { 0x08, 0x07, 0x01, 0x3b, 0xff, 0xff, 0x00, 0x00 }, .description = "PROP_USBDEVICE_CONNECT", .out_spells = { /* spell #32 */
        { 0 } } },
    { { 0x08, 0x07, 0x01, 0x3b, 0xff, 0x00, 0x00, 0x00 }, .description = "PROP_USBDEVICE_CONNECT", .out_spells = { /* spell #33 */
        { 0 } } },
    { { 0x08, 0x06, 0x03, 0x1f, 0x00, 0x00, 0x00 }, .description = "PROP 80030019", .out_spells = { /* spell #34 */
        { 0 } } },
    { { 0x08, 0x07, 0x01, 0x3b, 0xff, 0x00, 0x00, 0x00 }, .description = "PROP_USBDEVICE_CONNECT", .out_spells = { /* spell #35 */
        { 0 } } },
    { { 0x06, 0x05, 0x03, 0x1e, 0x00, 0x00 }, .description = "PROP 8003001A", .out_spells = { /* spell #36 */
        { 0x06, 0x05, 0x03, 0x35, 0x01, 0x00 },                 /* reply #36.1, PROP_BATTERY_REPORT_COUNTER */
        { 0x1c, 0x1b, 0x03, 0x1d, 0x4f, 0x03, 0x00, 0x00, 0x00, 0x18, 0x00, 0x4c, 0x50, 0x2d, 0x45, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x7e, 0x5b, 0x1c, 0x57, 0x00 },/* reply #36.2, PROP_BATTERY_REPORT */
        { 0x06, 0x04, 0x03, 0x36, 0x00 },                       /* reply #36.3, PROP_BATTERY_REPORT_FINISHED */
        { 0 } } },
    { { 0x16, 0x14, 0x01, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x0f, 0x00 }, .description = "PROP_VIDEO_MODE", .out_spells = { /* spell #37 */
        { 0x16, 0x14, 0x01, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x0f, 0x00 },/* reply #37.1, PROP_VIDEO_MODE */
        { 0 } } },
    { { 0x16, 0x14, 0x01, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x0f, 0x00 }, .description = "PROP_VIDEO_MODE", .out_spells = { /* spell #38 */
        { 0x16, 0x14, 0x01, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x0f, 0x00 },/* reply #38.1, PROP_VIDEO_MODE */
        { 0 } } },
    { { 0x06, 0x05, 0x03, 0x19, 0x01, 0x00 }, .description = "PROP_TFT_STATUS", .out_spells = { /* spell #39 */
        { 0 } } },
    { { 0x06, 0x05, 0x03, 0x19, 0x01, 0x00 }, .description = "PROP_TFT_STATUS", .out_spells = { /* spell #40 */
        { 0 } } },
    { { 0x06, 0x05, 0x02, 0x0a, 0x01, 0x00 }, .description = "PROP_PERMIT_ICU_EVENT", .out_spells = { /* spell #41 */
        { 0x06, 0x05, 0x06, 0x11, 0x01, 0x00 },                 /* reply #41.1, GUI_Control:39, bindReceiveSwitch(17, 1) */
        { 0x06, 0x05, 0x06, 0x12, 0x00, 0x00 },                 /* reply #41.2, GUI_Control:41, bindReceiveSwitch(18, 0) */
     // { 0x06, 0x05, 0x06, 0x13, 0x00, 0x00 },                 /* reply #41.3, GMT_OLC_INFO_CHANGED, GUI_Control:59, bindReceiveSwitch(19, 0) */
        { 0x42, 0x41, 0x0a, 0x08, 0xff, 0x1f, 0x01, 0x00, 0x01, 0x01, 0xa0, 0x10, 0x00, 0x73, 0x01, 0x01, 0x50, 0x25, 0x3b, 0x01, 0x01, 0x00, 0x48, 0x04, 0x01, 0x08, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },/* reply #41.4, PD_NotifyOlcInfoChanged */
        { 0x06, 0x05, 0x04, 0x0e, 0x01, 0x00 },                 /* reply #41.5, PROP 8002000D */
        { 0 } } },
 // { { 0x06, 0x05, 0x03, 0x16, 0x06, 0x00 }, .description = "PROP_BATTERY_CHECK", .out_spells = { 
     // { 0x06, 0x05, 0x03, 0x16, 0x00, 0x00 },                 /* reply #42.1, PROP_BATTERY_CHECK */
     // { 0 } } },
    { { 0x06, 0x05, 0x04, 0x0d, 0x01, 0x00 }, .description = "PROP_ACTIVE_SWEEP_STATUS", .out_spells = { /* spell #42 */
        { 0x06, 0x05, 0x04, 0x0d, 0x00, 0x00 },                 /* reply #42.1, PROP_ACTIVE_SWEEP_STATUS, disabled */
        { 0x06, 0x05, 0x04, 0x15, 0x00, 0x00 },                 /* reply #42.2, PROP_DL_ACTION */
        { 0x06, 0x05, 0x03, 0x17, 0x92, 0x00 },                 /* reply #42.3, PROP_EFIC_TEMP */
        { 0 } } },
    { { 0x06, 0x05, 0x03, 0x19, 0x00, 0x00 }, .description = "PROP_TFT_STATUS", .out_spells = { /* spell #43 */
     // { 0x06, 0x05, 0x03, 0x17, 0x92, 0x00 },                 /* reply #43.1, PROP_EFIC_TEMP, delayed by 798 ms, likely external input */
        { 0 } } },

    #include "NotifyGUIEvent.h"
    #include "UILock.h"
    #include "CardFormat.h"
    #include "MpuProperties.h"
    #include "GPS.h"
    #include "LiveView.h"
    #include "Shutdown.h"
};
