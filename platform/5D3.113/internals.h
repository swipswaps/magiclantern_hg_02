/**
 * Camera internals for 5D3 1.1.3
 */

/** This camera has both a CF and an SD slot **/
#define CONFIG_DUAL_SLOT

/** Properties are persistent (saved in NVRAM) => a mistake can cause permanent damage. Undefine this for new ports. */
/** The 5D3 port is young, but... let's give it a try! **/
#define CONFIG_PROP_REQUEST_CHANGE

/** 
 * State object hooks are pieces of code that run in Canon tasks (state objects). See state-object.c . 
 * They might slow down Canon code, so here you can disable all of them (useful for debugging or early ports) 
 */
#define CONFIG_STATE_OBJECT_HOOKS

/** This camera runs DryOS **/
//~ #define CONFIG_VXWORKS

/** This camera has a DIGIC V chip */
#define CONFIG_DIGIC_V

/** This camera has a full-frame sensor */
#define CONFIG_FULLFRAME

/** This camera has LiveView and can record video **/
#define CONFIG_LIVEVIEW
#define CONFIG_MOVIE

/** This camera has a 3:2 screen, 720x480 **/
#define CONFIG_3_2_SCREEN

/** We only have a single red LED **/
//~ #define CONFIG_BLUE_LED

/** There is no LCD sensor that turns the display off **/
//~ #define CONFIG_LCD_SENSOR

/** This camera has a mirror lockup feature **/
#define CONFIG_MLU

/** This camera reports focus info in LiveView **/
#define CONFIG_LV_FOCUS_INFO

/** Reports roll and pitch angle **/
#define CONFIG_ELECTRONIC_LEVEL

/** Define this if the camera has an ambient light sensor used for auto brightness **/
#define CONFIG_AUTO_BRIGHTNESS

/** There is a Q menu in Play mode, with image protect, rate etc **/
/** But it's a bit different from the other cameras, so let's say it doesn't have **/
//~ #define CONFIG_Q_MENU_PLAYBACK

/** No flip-out display **/
//~ #define CONFIG_VARIANGLE_DISPLAY

/** It has a 5D2-like battery which reports exact percentage **/
#define CONFIG_BATTERY_INFO

/** We can do bulb exposures **/
#define CONFIG_BULB

/** There is a separate bulb mode on the mode dial (other cameras have BULB = M + shutter speed beyond 30s) **/
#define CONFIG_SEPARATE_BULB_MODE

/** We can't control audio settings from ML **/
//~ #define CONFIG_AUDIO_CONTROLS

/** Zoom button can be used while recording (for Magic Zoom) **/
//~ #define CONFIG_ZOOM_BTN_NOT_WORKING_WHILE_RECORDING

/** We can redirect the display buffer to some arbitrary address, just by changing YUV422_LV_BUFFER_DISPLAY_ADDR **/
#define CONFIG_CAN_REDIRECT_DISPLAY_BUFFER_EASILY
#define CONFIG_CAN_REDIRECT_DISPLAY_BUFFER

/** Therefore, we can implement display filters (features that alter the LiveView image in real-time) **/
#define CONFIG_DISPLAY_FILTERS

/** We can override ISO on a per-frame basis, by changing FRAME_ISO (e.g. for HDR video or gradual exposure) **/
#define CONFIG_FRAME_ISO_OVERRIDE

/** But we can't override the digital ISO component via FRAME_ISO **/
#define CONFIG_FRAME_ISO_OVERRIDE_ANALOG_ONLY

/** We can also override shutter on a per-frame basis */
#define CONFIG_FRAME_SHUTTER_OVERRIDE

/** We can change ExpSim from ML **/
#define CONFIG_EXPSIM

/** We can playback sounds via ASIF DMA **/
#define CONFIG_BEEP

/** This camera has no trouble saving Kelvin and/or WBShift in movie mode **/
//~ #define CONFIG_WB_WORKAROUND

/** We ca restore ML files after formatting the card in the camera **/
#define CONFIG_RESTORE_AFTER_FORMAT

/** We know how to use DMA_MEMCPY, though I don't see any reason for doing so **/
/** it's not really faster than plain memcpy, and the side effects are not yet fully understood **/
/** (read: I'm too dumb to understand why it's better than memcpy and why it's safe to use) **/
//~ #define CONFIG_DMA_MEMCPY

/** We know how to use edmac_memcpy. This one is really fast (600MB/s!) */
#define CONFIG_EDMAC_MEMCPY

/** We shouldn't warn the user if movie exposure is Auto **/
//~ #define CONFIG_MOVIE_AE_WARNING

/** We can display some extra info in photo mode (not LiveView) **/
#define CONFIG_PHOTO_MODE_INFO_DISPLAY

/** Perfect sync using EVF_STATE **/
#define CONFIG_EVF_STATE_SYNC

/** We can record movies in regular photo modes - M, P, Tv, Av... */
#define CONFIG_NO_DEDICATED_MOVIE_MODE

/** We can adjust AFMA (AF microadjustment) */
#define CONFIG_AFMA

/** The camera accepts extended AFMA values (default range: -20...20; extended: -100...100) */
/** This can be dangerous, as the values are outside Canon limits */
#define CONFIG_AFMA_EXTENDED

/** You can configure separate AFMA values for both wide and tele ends */
#define CONFIG_AFMA_WIDE_TELE

/** FPS override: change timers from EVF state (both methods are OK on 5D3) */
//~ #define CONFIG_FPS_UPDATE_FROM_EVF_STATE

/** Use the new Rec.709 for YUV-RGB conversion (undefine for Rec.601) */
#define CONFIG_REC709

/** We have access to raw data in both photo mode and in LiveView */
#define CONFIG_RAW_PHOTO
#define CONFIG_RAW_LIVEVIEW

/** Zoom on half-shutter may cause black pictures.
 *  Workaround: block the shutter button while switching zoom, to avoid the race condition
 *  todo: find a proper fix that does not prevent picture taking
 */
#define CONFIG_ZOOM_HALFSHUTTER_UILOCK

/** Use joystick for one-finger menu navigation */
#define CONFIG_JOY_CENTER_ACTIONS


/** this method bypasses Canon's lv_save_raw and slurps the raw data directly from connection #0 */
#define CONFIG_EDMAC_RAW_SLURP

/** Patch LiveViewApp dialog hander to hide Canon bottom bar */
#define CONFIG_LVAPP_HACK_PATCH
