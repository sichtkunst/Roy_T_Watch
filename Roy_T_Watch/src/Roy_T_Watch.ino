/**
 * Simple clock display interface, power consumption is about 20mA
 * original Written by lewishe
 * */
#include "config.h"

#define LV_COLOR_LCD_BG_BL_ON LV_COLOR_MAKE(0xA8, 0xC6, 0x4E)
#define LV_COLOR_LCD_BG_BL_OFF LV_COLOR_MAKE(0x3C, 0x41, 0x2C)
#define LV_COLOR_LCD_SEG_ON LV_COLOR_MAKE(0xF0, 0xFA, 0xF0)
#define LV_COLOR_LCD_SEG_OFF LV_COLOR_MAKE(0xFF, 0xFF, 0xFF) //Better use OPA_10
#define LV_COLOR_LCD_SEG_SHAD LV_COLOR_MAKE(0xFF, 0xFF, 0xFF) //Better use OPA_50

#define LV_COLOR_PHOSPHOR LV_COLOR_MAKE(0x88, 0xFF, 0x88) 

#ifdef DIGITAL_1
typedef struct {
    lv_obj_t *hour;
    lv_obj_t *minute;
    lv_obj_t *second;
    lv_obj_t *day;
    lv_obj_t *month;
    lv_obj_t *year;
} str_datetime_t;
static str_datetime_t g_data, g_data_shadow;
#endif //DIGITAL_1


TTGOClass *watch = nullptr;
PCF8563_Class *rtc;
int Angel_S, Angel_M, Angel_H;

#ifdef SLEEP_TIMER
int Sleeptimer_End, Sleeptimer_Now;

AXP20X_Class *power;
BMA *sensor;
#endif //SLEEP_TIMER

#ifdef BAT_LVL
int BatLvl;

#endif //BAT_LVL




void setup()
{
    Serial.begin(115288);
    watch = TTGOClass::getWatch();
    watch->begin();
    watch->lvgl_begin();
    rtc = watch->rtc;

    // Use compile time
    rtc->check();

    // Turn on the backlight
    watch->openBL();

    //Lower the brightness
    watch->bl->adjust(150);


	

#ifdef SLEEP_TIMER
    power = watch->power;
    sensor = watch->bma;

    // Accel parameter structure
    Acfg cfg;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;
    sensor->accelConfig(cfg);
    sensor->enableAccel();
        // Disable BMA423 isStepCounter feature
    sensor->enableFeature(BMA423_STEP_CNTR, false);
    // Enable BMA423 isTilt feature
    sensor->enableFeature(BMA423_TILT, true);
    // Enable BMA423 isDoubleClick feature
    sensor->enableFeature(BMA423_WAKEUP, true);
    // Reset steps
    sensor->resetStepCounter();
    
    // Turn off feature interrupt
    // sensor->enableStepCountInterrupt();
    
    sensor->enableTiltInterrupt();
    // It corresponds to isDoubleClick interrupt
    sensor->enableWakeupInterrupt();
#endif //SLEEP_TIMER


    lv_obj_t *img1 = lv_img_create(lv_scr_act(), NULL);

//Background
    LV_IMG_DECLARE(Logo_lvgl);
    lv_img_set_src(img1, &Logo_lvgl);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);
    static lv_style_t bg_style;
    lv_style_set_bg_color(&bg_style, LV_STATE_DEFAULT, LV_COLOR_LCD_BG_BL_ON);
    lv_style_set_bg_opa(&bg_style,LV_STATE_DEFAULT,LV_OPA_50);
    lv_obj_add_style(img1,LV_STATE_DEFAULT ,&bg_style);
    lv_obj_move_background(img1);

#ifdef BAT_LVL
	static lv_obj_t *BatBar = lv_bar_create(img1, NULL);
    //  static lv_color_t needle_colors[1];
    //  needle_colors[0] = LV_COLOR_BLACK;
    //	lv_gauge_set_needle_count(BatGauge, 1, needle_colors);
	static lv_style_t BatBar_Style;
	lv_style_set_bg_opa(&BatBar_Style, LV_STATE_DEFAULT,LV_OPA_50);
    lv_obj_add_style(BatBar,LV_STATE_DEFAULT,&BatBar_Style);
    lv_bar_set_range(BatBar, 0, 100);
    //lv_gauge_set_critical_value(BatGauge,20);
	lv_bar_set_start_value(BatBar,0,LV_ANIM_ON);
	lv_obj_set_size(BatBar, 100, 10);
    lv_obj_align(BatBar, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 5);
#endif //BAT_LVL

#ifdef DIGITAL_1
    LV_FONT_DECLARE(Matrix_Dot_64p);
    static lv_style_t style, style_shadow, style_small;
    lv_style_init(&style);
    lv_style_init(&style_shadow);

    lv_style_set_text_color(&style, LV_STATE_DEFAULT, LV_COLOR_LIME);  
    lv_style_set_text_color(&style_shadow, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    lv_style_set_text_font(&style, LV_STATE_DEFAULT, &Matrix_Dot_64p);
    lv_style_set_text_font(&style_shadow, LV_STATE_DEFAULT, &Matrix_Dot_64p);

    lv_style_set_text_opa(&style,LV_STATE_DEFAULT,LV_OPA_COVER);
    lv_style_set_text_opa(&style_shadow,LV_STATE_DEFAULT,LV_OPA_80);

    g_data_shadow.hour = lv_label_create(img1, nullptr);
    g_data.hour = lv_label_create(img1, nullptr);
    
    lv_obj_add_style(g_data_shadow.hour, LV_OBJ_PART_MAIN, &style_shadow);
    lv_obj_add_style(g_data.hour, LV_OBJ_PART_MAIN, &style);

    lv_label_set_text(g_data_shadow.hour, "88");
    lv_label_set_text(g_data.hour, lv_label_get_text(g_data_shadow.hour));

    lv_obj_align(g_data_shadow.hour, img1, LV_ALIGN_IN_TOP_LEFT, 14, 24);
    lv_obj_align(g_data.hour, img1, LV_ALIGN_IN_TOP_LEFT, 10, 20);

    g_data_shadow.minute = lv_label_create(img1, nullptr);
    g_data.minute = lv_label_create(img1, nullptr);

    lv_obj_add_style(g_data_shadow.minute, LV_OBJ_PART_MAIN, &style_shadow);
    lv_obj_add_style(g_data.minute, LV_OBJ_PART_MAIN, &style);

    lv_label_set_text(g_data_shadow.minute, ":88");
    lv_label_set_text(g_data.minute, lv_label_get_text(g_data_shadow.minute));
    
    lv_obj_align(g_data_shadow.minute, img1, LV_ALIGN_IN_TOP_LEFT, 79, 24);
    lv_obj_align(g_data.minute, img1, LV_ALIGN_IN_TOP_LEFT, 75, 20);

    g_data_shadow.second = lv_label_create(img1, nullptr);
    g_data.second = lv_label_create(img1, nullptr);
    
    lv_obj_add_style(g_data_shadow.second, LV_OBJ_PART_MAIN, &style_shadow);
    lv_obj_add_style(g_data.second, LV_OBJ_PART_MAIN, &style);

    lv_label_set_text(g_data_shadow.second, ":88");
    lv_label_set_text(g_data.second, lv_label_get_text(g_data_shadow.second));

    lv_obj_align(g_data_shadow.second, img1, LV_ALIGN_IN_TOP_LEFT, 144, 24);
    lv_obj_align(g_data.second, img1, LV_ALIGN_IN_TOP_LEFT, 140, 20);
  
    g_data.day = lv_label_create(img1, nullptr);
    lv_obj_add_style(g_data.day, LV_OBJ_PART_MAIN, &style);
   lv_label_set_text(g_data.day, "88");
    lv_obj_align(g_data.day, img1, LV_ALIGN_IN_LEFT_MID, 1, 20);

    g_data.month = lv_label_create(img1, nullptr);
    lv_obj_add_style(g_data.month, LV_OBJ_PART_MAIN, &style);
    lv_label_set_text(g_data.month, ".88");
    lv_obj_align(g_data.month, img1, LV_ALIGN_IN_LEFT_MID, 62, 20);

    g_data.year = lv_label_create(img1, nullptr);
    lv_obj_add_style(g_data.year, LV_OBJ_PART_MAIN, &style);
    lv_label_set_text(g_data.year, ".8888");
    lv_obj_align(g_data.year, img1, LV_ALIGN_IN_LEFT_MID, 125, 20); 

#endif // DIGITAL_1

#ifdef TICKER
    LV_FONT_DECLARE(Ticker_Small_12p);

    static lv_obj_t *Ticker = lv_label_create (img1,NULL);
    lv_style_set_text_font(&style_small, LV_STATE_DEFAULT, &Ticker_Small_12p);
    lv_style_set_text_color(&style_small,LV_STATE_DEFAULT,LV_COLOR_LIME);
    lv_style_set_text_opa(&style_small,LV_STATE_DEFAULT,LV_OPA_COVER);
    lv_label_set_long_mode(Ticker,LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(Ticker, 220);
    lv_obj_add_style(Ticker, LV_OBJ_PART_MAIN, &style_small);
    lv_label_set_text(Ticker,"12367890qwertzuiopasdfghjklyxcvbbnzrdzrd55dtumu6u");
    lv_obj_align(Ticker, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 180);


#endif // TICKER
	
#ifdef ANALOG_1
// Watchface

    static lv_obj_t *Second_Hand_s = lv_img_create(img1, NULL);
    static lv_obj_t *Minute_Hand_s = lv_img_create(img1, NULL);
    static lv_obj_t *Hour_Hand_s = lv_img_create(img1, NULL);
    
    static lv_obj_t *Hour_Hand = lv_img_create(img1, NULL);
    static lv_obj_t *Minute_Hand = lv_img_create(img1, NULL);
    static lv_obj_t *Second_Hand = lv_img_create(img1, NULL);

// Dial   to do..maybe


///HandShadow
// HourHand 
    LV_IMG_DECLARE(HourHand_S);
    lv_img_set_src(Hour_Hand_s, &HourHand_S);
    lv_img_set_antialias(Hour_Hand_s,true);
    lv_img_set_pivot(Hour_Hand_s,20,6);
    lv_obj_align(Hour_Hand_s, img1, LV_ALIGN_IN_TOP_LEFT, 105, 125);
    lv_img_set_angle(Hour_Hand_s,3300);
// MinuteHand
    LV_IMG_DECLARE(MinuteHand_S);
    lv_img_set_src(Minute_Hand_s, &MinuteHand_S);
    lv_img_set_antialias(Minute_Hand_s,true);
    lv_img_set_pivot(Minute_Hand_s,20,6);
    lv_img_set_antialias(Minute_Hand_s,true);
    lv_obj_align(Minute_Hand_s, img1,  LV_ALIGN_IN_TOP_LEFT, 105, 125);
    lv_img_set_angle(Minute_Hand_s,2100);
// SecondHand
    LV_IMG_DECLARE(SecondHand_S);
    lv_img_set_src(Second_Hand_s, &SecondHand_S);
    lv_img_set_antialias(Second_Hand_s,true);
    lv_img_set_pivot(Second_Hand_s,20,6);
    lv_obj_align(Second_Hand_s, img1,  LV_ALIGN_IN_TOP_LEFT, 105, 125);
    lv_img_set_angle(Second_Hand_s,900);

// Normal Hand

// HourHand
    LV_IMG_DECLARE(HourHand);
    lv_img_set_src(Hour_Hand, &HourHand);
    lv_img_set_antialias(Hour_Hand,true);
    lv_img_set_pivot(Hour_Hand,20,5);
    lv_obj_align(Hour_Hand, img1, LV_ALIGN_IN_TOP_LEFT, 100, 120);
    lv_img_set_angle(Hour_Hand,3300);
// MinuteHand
    LV_IMG_DECLARE(MinuteHand);
    lv_img_set_src(Minute_Hand, &MinuteHand);
    lv_img_set_antialias(Minute_Hand,true);
    lv_img_set_pivot(Minute_Hand,20,5);
    lv_img_set_antialias(Minute_Hand,true);
    lv_obj_align(Minute_Hand, img1,  LV_ALIGN_IN_TOP_LEFT, 100, 120);
    lv_img_set_angle(Minute_Hand,2100);
// SecondHand
    LV_IMG_DECLARE(SecondHand);
    lv_img_set_src(Second_Hand, &SecondHand);
    lv_img_set_antialias(Second_Hand,true);
    lv_img_set_pivot(Second_Hand,20,5);
    lv_obj_align(Second_Hand, img1,  LV_ALIGN_IN_TOP_LEFT, 100, 120);
    lv_img_set_angle(Second_Hand,900);
	
// Shadow Style
   static lv_style_t hand_s_style;
    lv_style_set_image_opa(&hand_s_style, LV_STATE_DEFAULT, LV_OPA_20);
    lv_obj_add_style(Second_Hand_s,LV_OBJ_PART_MAIN ,&hand_s_style);
    lv_obj_add_style(Hour_Hand_s,LV_OBJ_PART_MAIN ,&hand_s_style);
    lv_obj_add_style(Minute_Hand_s,LV_OBJ_PART_MAIN ,&hand_s_style);

// Normal Style
    static lv_style_t hand_style;

    lv_obj_add_style(Second_Hand,LV_OBJ_PART_MAIN ,&hand_style);
    lv_obj_add_style(Hour_Hand,LV_OBJ_PART_MAIN ,&hand_style);
    lv_obj_add_style(Minute_Hand,LV_OBJ_PART_MAIN ,&hand_style);
	
#endif //ANALOG_1

///////////////////////////////////////////////////////
    lv_task_create([](lv_task_t *t) 
    {
    RTC_Date curr_datetime = rtc->getDateTime();
    
#ifdef ANALOG_1	
    Angel_S = ((int) curr_datetime.second 	*60) 	+ 2700;
    Angel_M = ((int) curr_datetime.minute	*60) 	+ 2700;
    Angel_H = ((int) curr_datetime.hour		*300)	+ ((int) curr_datetime.minute	*5)	+ 2700;

    while (Angel_S >= 3600) Angel_S = Angel_S - 3600;
    while (Angel_M >= 3600) Angel_M = Angel_M - 3600;
    while (Angel_H >= 3600) Angel_H = Angel_H - 3600;

	//Normal
    lv_img_set_angle(Second_Hand,Angel_S);
    lv_img_set_angle(Minute_Hand,Angel_M);
    lv_img_set_angle(Hour_Hand,Angel_H);
    
    //Shadow
    lv_img_set_angle(Second_Hand_s,Angel_S);
    lv_img_set_angle(Minute_Hand_s,Angel_M);
    lv_img_set_angle(Hour_Hand_s,Angel_H);
	
#endif //ANALOG_1
#ifdef DIGITAL_1

    lv_label_set_text_fmt(g_data.second, "%02u", curr_datetime.second);
    lv_label_set_text_fmt(g_data.minute, "%02u", curr_datetime.minute);
    lv_label_set_text_fmt(g_data.hour, "%02u", curr_datetime.hour);
    lv_label_set_text_fmt(g_data.day, "%02u", curr_datetime.day);
    lv_label_set_text_fmt(g_data.month, "%02u", curr_datetime.month);
    lv_label_set_text_fmt(g_data.year, "%02u", curr_datetime.year);
#endif //DIGITAL_1  
#ifdef BAT_LVL
    BatLvl = watch->power->getBattPercentage();
	lv_bar_set_value(BatBar,0,(int)(BatLvl));
#endif //BAT_LVL
//// Serial Output
  Serial.print("Vbus: "); Serial.print((int)watch->power->getVbusVoltage()); Serial.println(" mV");
  Serial.print("Vbus: "); Serial.print(watch->power->getVbusCurrent()); Serial.println(" mA");
  Serial.print("BATT: "); Serial.print(watch->power->getBattVoltage()); Serial.println(" mV");
  Serial.print("Per: "); Serial.print(watch->power->getBattPercentage()); Serial.println(" %");
  Serial.println();
    Serial.print("Time:"); Serial.print(curr_datetime.hour); Serial.print(":");Serial.print(curr_datetime.minute); Serial.print(":");Serial.print(curr_datetime.second); Serial.println();
    Serial.println();

////


	}, 1000, LV_TASK_PRIO_MID, nullptr);
/////////////////////////////////////////////////////////////
    // Set 20MHz operating speed to reduce power consumption
    setCpuFrequencyMhz(20);
#ifdef SLEEP_TIMER

    RTC_Date SleepTimer = rtc->getDateTime();
    Sleeptimer_End = (int)SLEEP_TIMER+(SleepTimer.second)+(SleepTimer.minute*60)+(SleepTimer.hour*360);
#endif //SLEEP_TIMER
   
    }


void loop()
{
    lv_task_handler();
	
#ifdef SLEEP_TIMER
	
    RTC_Date Timer = rtc->getDateTime();
        Sleeptimer_Now = (int)Timer.second+(Timer.minute*60)+(Timer.hour*360);

    if (Sleeptimer_Now>=Sleeptimer_End) 
    {
        Sleeptimer_End = Sleeptimer_End +60;
        watch->displaySleep();
        watch->powerOff();
    // LDO2 is used to power the display, and LDO2 can be turned off if needed
    // power->setPowerOutPut(AXP202_LDO2, false);
        esp_sleep_enable_ext1_wakeup(GPIO_SEL_39, ESP_EXT1_WAKEUP_ANY_HIGH);
        esp_deep_sleep_start();
    }
#endif //SLEEP_TIMER
}
