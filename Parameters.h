// Menu states
enum MenuState {
  MENU_LFORATE,
  MENU_LFOWAVE,
  MENU_LFOMULT,
  MENU_LFOALT,
  MENU_BEND_WHEEL,
  MENU_FM_MOD_WHEEL,
  MENU_FM_AT_WHEEL,
  MENU_DETUNE,
  MENU_GLIDE_SW,
  MENU_GLIDE_TIME,
  MENU_POLYMODE,
  MENU_NOTEPRIORITY,
  MENU_MIDICHANNEL,
  MENU_TOTAL_ITEMS
};

bool showSaveMessage = false;
unsigned long saveMessageStartTime = 0;
const unsigned long saveMessageDuration = 500;  // 2 seconds

MenuState currentMenu = MENU_LFORATE;
bool editing = false;
unsigned long lastMenuInteraction = 0;
bool settingsSaved = false;

bool GLIDE_SW = false;   // Off/On
int GLIDE_TIME = 0;      // 0-127
bool POLYMODE = false;   // Off/On

// Glide
bool portamentoOn = false;         // Portamento state (on/off)
uint16_t portamentoTime = 0;       // Portamento time in milliseconds (0–10000 ms)
uint16_t portamentoTimestr = 0;    // Display 0-127
bool portamentoActive = false;     // Tracks if portamento is currently running
unsigned long glideStartTime = 0;  // Start time for portamento
float currentMV_a = 0.0;           // Current mV value for channel_a
float targetMV_a = 0.0;            // Target mV value for channel_a
float currentMV_b = 0.0;           // Current mV value for channel_b
float targetMV_b = 0.0;            // Target mV value for channel_b
float currentMV_c = 0.0;           // Current mV value for channel_c
float targetMV_c = 0.0;            // Target mV value for channel_c
float initialMV_a = 0.0;           // Initial mV value for channel_a
float initialMV_b = 0.0;           // Initial mV value for channel_b
float initialMV_c = 0.0;           // Initial mV value for channel_c
float step_a = 0.0;       // Step size for channel_a
float step_b = 0.0;       // Step size for channel_b
float step_c = 0.0;       // Step size for channel_c

byte note = 0;

int OCTAVE_A = 0;
int OCTAVE_B = 0;
int OCTAVE_C = 0;
int VOLTOFFSET = 3270;
int oscillator;

//
// Modulation
//

float FM_VALUE = 0.0f;
float FM_AT_VALUE = 0.0f;
float MOD_VALUE = 0.0f;

int FM_RANGE_UPPER = 0;
int FM_RANGE_LOWER = 0;
int FM_AT_RANGE_UPPER = 0;
int FM_AT_RANGE_LOWER = 0;

float FM_MOD_WHEEL = 0.00f;
float FM_MOD_WHEEL_STR = 0.00f;
float FM_AT_WHEEL = 0.00f;
float FM_AT_WHEEL_STR = 0.00f;

int BEND_WHEEL = 0;
int TM_RANGE = 0;
int DETUNE = 0;
int INTERVAL_POT = 0;
int INTERVAL = 0;

// LFO parameters
int LFORATE = 0;
int LFOWAVE = 0;
int LFOMULT = 0;
bool LFOALT = false;  // Off/On
int lfoalt_old = 99;

int masterChan = 1;
int masterTran;
int previousMode;
int transpose = 0;
int8_t i;
int noteMsg;
int keyboardMode = 2;
int octave = 0;
int realoctave = -36;
int bend_data;
int note1;
int oscanote1;
int oscbnote1;
int osccnote1;
bool reset = false;
bool displayvalues = false;
bool osc1Through = false;
byte sendnote, sendvelocity;

unsigned int velmV;
unsigned int mV;
unsigned int additionalmV;
unsigned int finalmV;

float noteTrig[1];

uint32_t int_ref_on_flexible_mode = 0b00001001000010100000000000000000;  // { 0000 , 1001 , 0000 , 1010000000000000 , 0000 }

uint32_t sample_data = 0b00000000000000000000000000000000;

uint32_t sample_data1 = 0b00000000000000000000000000000000;

uint32_t vel_data1 = 0b00000000000000000000000000000000;

uint32_t channel_a = 0b00000010000000000000000000000000;
uint32_t channel_b = 0b00000010000100000000000000000000;
uint32_t channel_c = 0b00000010001000000000000000000000;
uint32_t channel_d = 0b00000010001100000000000000000000;
uint32_t channel_e = 0b00000010010000000000000000000000;
uint32_t channel_f = 0b00000010010100000000000000000000;
uint32_t channel_g = 0b00000010011000000000000000000000;
uint32_t channel_h = 0b00000010011100000000000000000000;
