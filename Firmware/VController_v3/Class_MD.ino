// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: MD_base_class declaration (base)
// Section 2: MD_GP10_class declaration (derived)
// Section 3: MD_GR55_class declaration (derived)
// Section 4: MD_VG99_class declaration (derived)
// Section 5: MD_ZG3_class declaration (derived)
// Section 6: MD_ZMS70_class declaration (derived)
// Section 7: MD_M13_class declaration (derived)
// Section 8: MD_HLX_class declaration (derived)
// Section 9: MD_FAS_class declaration (derived)
// Section 10: MD_KTN_class declaration (derived)
// Section 11: MD_KPA_class declaration (derived)
// Section 12: MD_SVL_class declaration (derived)
// Section 13: MD_SY1000_class declaration (derived)
// Section 14: MD_GM2_class declaration (derived)
// Section 15: MD_MG300_class declaration (derived)

// Here we declare the class objects for the midi devices the VController supports. The actual class code is found in the Md_<device_name>.ino files.

// We use a derived class structure here.
// The base class is MD_base_class. For each device there is a derived class, where certain functions are replaced to add device specific functionality.
// For a derived function to work, "virtual" must be declared in the header of the base class! I have forgotten this a number of times, so this is a reminder to self!

#define VC_PATCH_SIZE 192 // The size of a patch for Katana or SY1000

// ********************************* Section 1: MD_base_class declaration (base) ********************************************

#define NUMBER_OF_DEVICE_SETTINGS 12

class MD_base_class
{ // Base class for midi devices

  public:
    // Procedures:
    MD_base_class (uint8_t  _dev_no); // Constructor

    // Basic procedures
    virtual void init(); // Called in setup() of sketch
    virtual void update(); // Called in loop() of sketch
    uint8_t  get_setting(uint8_t  variable);
    void set_setting(uint8_t  variable, uint8_t  value);
    virtual uint8_t get_number_of_dev_types();
    virtual void get_dev_type_name(uint8_t number, String &name);
    virtual void do_after_dev_type_update();

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    virtual void forward_MIDI_message(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);
    virtual void forward_PC_message(uint8_t  program, uint8_t  channel);
    virtual void check_CC_in(uint8_t  control, uint8_t  value, uint8_t  channel, uint8_t  port);
    virtual void check_active_sense_in(uint8_t  port);
    void set_patch_number(uint16_t number);

    // Device connection procedures
    virtual void check_still_connected();
    void check_manual_connection();
    bool  can_request_sysex_data();
    virtual void send_alternative_identity_request(uint8_t  check_device_no);
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    void connect(uint8_t  device_id, uint8_t in_port, uint8_t out_port);
    virtual void do_after_connect();
    void disconnect();
    virtual void do_after_disconnect();

    // Midi out procedures
    void check_sysex_delay();
    uint8_t  calc_Roland_checksum(uint16_t  sum);
    virtual void set_bpm();
    virtual void bpm_tap();
    virtual void start_tuner();
    virtual void stop_tuner();

    // Patch selection procedures
    virtual void select_patch(uint16_t  new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t  calculate_patch_address(uint16_t  number);
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    virtual void request_current_patch_name();
    uint16_t  calculate_patch_number(uint8_t  bank_position, uint8_t  bank_size);
    bool  patch_select_pressed(uint16_t  new_patch, uint8_t sw);
    uint16_t calculate_prev_next_patch_number(signed int delta);
    void bank_updown(signed int delta, uint8_t  my_bank_size);
    bool  bank_selection_active();
    void update_bank_number(uint16_t  number);
    void update_bank_size(uint8_t  b_size);
    virtual bool  flash_LEDs_for_patch_bank_switch(uint8_t  sw);
    void display_patch_number_string();
    virtual void number_format(uint16_t  number, String &Output);
    uint16_t get_patch_min();
    uint16_t get_patch_max();

    // Setlist/song select
    virtual void setlist_song_select(uint16_t item);
    virtual uint16_t setlist_song_get_current_item_state();
    virtual uint16_t setlist_song_get_number_of_items();
    virtual void setlist_song_full_item_format(uint16_t item, String &Output);
    virtual void setlist_song_short_item_format(uint16_t item, String &Output);
    uint16_t patch_number_in_current_setlist(uint16_t number);

    // Direct select procedures
    virtual void direct_select_format(uint16_t  number, String &Output);
    virtual bool  valid_direct_select_switch(uint8_t  number);
    virtual void direct_select_start();
    virtual uint16_t  direct_select_patch_number_to_request(uint8_t  number);
    virtual void direct_select_press(uint8_t  number);

    // Device mute procedures
    virtual void unmute();
    virtual void mute();
    void select_switch();
    void is_always_on_toggle();
    bool  US20_mode_enabled();

    // Parameter control procedures
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_title_short(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);
    virtual uint16_t  number_of_parbank_parameters();
    virtual uint16_t  get_parbank_parameter_id(uint16_t  par_number);
    virtual void request_par_bank_category_name(uint8_t  sw);
    virtual void select_parameter_bank_category(uint8_t  category);
    virtual void par_bank_updown(signed int delta, uint8_t  my_bank_size);

    // Assign control procedures
    virtual void read_assign_name(uint8_t  number, String &Output);
    virtual void read_assign_short_name(uint8_t  number, String &Output);
    virtual void read_assign_trigger(uint8_t  number, String &Output);
    virtual uint8_t  get_number_of_assigns();
    virtual uint8_t  trigger_follow_assign(uint8_t  number);
    virtual void assign_press(uint8_t  Sw, uint8_t  value);
    virtual void assign_release(uint8_t  Sw);
    virtual void assign_load(uint8_t  sw, uint8_t  assign_number, uint8_t  cc_number);
    virtual void request_current_assign(uint8_t  sw);
    void asgn_bank_updown(signed int delta, uint8_t  my_bank_size);

    virtual void FX_press(uint8_t  Sw, Cmd_struct *cmd, uint8_t  number);
    virtual void FX_set_type_and_state(uint8_t  Sw);

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    virtual void toggle_expression_pedal(uint8_t  sw);
    virtual void set_expr_title(uint8_t  sw);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);

    // Page selection procedures
    uint8_t  read_current_device_page();
    uint8_t  select_next_device_page();

    // Snapshot/sceme selection procedures
    virtual void get_snapscene_title(uint8_t  number, String &Output);
    virtual void get_snapscene_title_short(uint8_t  number, String &Output);
    virtual void get_snapscene_label(uint8_t  number, String &Output);
    virtual bool  request_snapscene_name(uint8_t  sw, uint8_t sw1, uint8_t sw2, uint8_t sw3);
    virtual void set_snapscene(uint8_t  sw, uint8_t  number);
    virtual void release_snapscene(uint8_t  sw, uint8_t  number);
    virtual void show_snapscene(uint8_t  number);
    virtual void snapscene_number_format(String &Output);
    virtual bool  check_snapscene_active(uint8_t  scene);

    // Looper procedures
    virtual bool  looper_active();
    uint8_t  show_looper_LED(uint8_t  sw);
    uint8_t  request_looper_backlight_colour();
    virtual void request_looper_label(uint8_t  sw);
    void looper_press(uint8_t  looper_cmd, bool  send_cmd);
    virtual void looper_release();
    void looper_reset();
    void looper_timer_check();
    virtual void send_looper_cmd(uint8_t  cmd);

    // Variables:
    uint8_t  enabled; //Is this device enabled: state can be 0: OFF, 1: ON or 2: DETECT
    uint8_t  my_device_number;
    char device_name[8];
    char full_device_name[17];
    uint8_t  my_LED_colour;  // Variable no.1
    uint8_t my_snapscene_colour = 2;
    uint8_t  MIDI_channel;  // Variable no.2
    uint8_t  MIDI_in_port = 0;
    uint8_t MIDI_out_port = 0;
    uint8_t  MIDI_port_manual = 0; // Variable no.4
    uint8_t  MIDI_device_id; // Variable no.3
    uint16_t setlist_item_number = 0;
    uint16_t  patch_number = 0;
    uint16_t  prev_patch_number = 0;
    uint8_t  patch_number_offset = 1; // Is the first patch numbered one or zero?
    uint32_t  PC_ignore_timer;
    String current_patch_name;
    bool  popup_patch_name = false;
    uint16_t  patch_min;
    uint16_t  patch_max;
    uint16_t  bank_number = 0; // The bank number of the current_patch
    uint16_t  bank_select_number; // The bank number used for bank up/down
    uint16_t  bank_size = 10;
    uint8_t  CC00 = 0;
    uint8_t  direct_select_state = 0;
    //uint8_t  bank_select_number;
    bool  connected = false;
    uint8_t  no_response_counter = 0;
    uint8_t  max_times_no_response;
    bool  is_on = false;
    bool  is_always_on = true;
    bool  request_onoff = false;
    unsigned long sysex_delay_length;// time between sysex messages (in msec).
    unsigned long sysexDelay = 0;
    uint32_t  last_requested_sysex_address;
    uint8_t  last_requested_sysex_type;
    uint8_t  last_requested_sysex_switch;
    uint16_t  last_requested_sysex_patch_number;
    uint16_t  parameter_bank_number = 0;
    uint8_t  parameter_bank_category = 0;
    uint8_t  assign_bank_number;
    uint8_t  current_setlist = 0; // Needed for Helix and AxeFX
    uint8_t  current_snapscene = 0; // For Helix snapshots or AxeFX scenes
    uint8_t  current_exp_pedal = 0;
    uint32_t  midi_timer; // VController hangs if not declared in base class! Really don't know why. Only need it for the M13
    uint8_t  current_looper_state;
    uint8_t  looper_stop_pressed;
    bool  looper_show = false;
    bool  loop_recorded = false;
    bool  looper_reverse = false;
    bool  looper_undone = false;
    bool  looper_half_speed = false;
    bool  looper_pre = false;
    bool  looper_overdub_happened = false;
    uint32_t  looper_start_time = 0;
    uint32_t  looper_end_time = 0;
    uint32_t  current_loop_length;
    uint32_t  max_looper_length;
    uint8_t dev_type = 0; // Variable no.11

    // Default pages
    uint8_t  my_device_page1 = 0;  // Variable no.4
    uint8_t  my_device_page2 = 0;  // Variable no.5
    uint8_t  my_device_page3 = 0;  // Variable no.6
    uint8_t  my_device_page4 = 0;  // Variable no.7
    uint8_t  current_device_page = 0; // The page that is selected

    const uint16_t *device_pic = NULL;
};

// Looper commands:
#define LOOPER_CMD_HIDE 0
#define LOOPER_CMD_SHOW 1
#define LOOPER_CMD_STOP 2
#define LOOPER_CMD_PLAY 3
#define LOOPER_CMD_REC 4
#define LOOPER_CMD_OVERDUB 5
#define LOOPER_CMD_FORWARD 6
#define LOOPER_CMD_REVERSE 7
#define LOOPER_CMD_FULL_SPEED 8
#define LOOPER_CMD_HALF_SPEED 9
#define LOOPER_CMD_UNDO 10
#define LOOPER_CMD_REDO 11
#define LOOPER_CMD_PLAY_ONCE 12
#define LOOPER_CMD_PRE 13
#define LOOPER_CMD_POST 14

// Looper states - copied from AxeFX:
#define LOOPER_STATE_STOPPED 0
#define LOOPER_STATE_RECORD 1 // Bit 0: Record
#define LOOPER_STATE_PLAY 2 // Bit 1: Play
#define LOOPER_STATE_PLAY_ONCE 4 // Bit 2: Once
#define LOOPER_STATE_OVERDUB 8 // Bit 3: Overdub
#define LOOPER_STATE_REVERSE 16 // Bit 4: Reverse
#define LOOPER_STATE_HALF 32 // Bit 5: Half
#define LOOPER_STATE_UNDO 64 // Bit 6: Undo
#define LOOPER_STATE_ERASED 128 // Used for KPA

uint8_t  device_in_bank_selection = 0; // One global variable for all devices to keep track of bank selection
#define PAGE_BANK_SELECTION_IN_PROGRESS 255
#define MIDI_PC_SELECTION_IN_PROGRESS 254
#define SETLIST_BANK_SELECTION_IN_PROGRESS 253
#define SONG_BANK_SELECTION_IN_PROGRESS 252

// ********************************* Section 2: MD_GP10_class declaration (derived) ********************************************

class MD_GP10_class : public MD_base_class
{
  public:
    MD_GP10_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    virtual void do_after_connect();

    // Midi out procedures
    void write_sysex(uint32_t  address, uint8_t  value);
    void write_sysex(uint32_t  address, uint8_t  value1, uint8_t  value2);
    void request_sysex(uint32_t  address, uint8_t  no_of_bytes);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    // Patch selection procedures
    virtual void do_after_patch_selection();
    //virtual uint32_t  calculate_patch_address(uint16_t  number);
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    virtual void request_current_patch_name();
    //void page_check();
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t  number, String &Output);

    // Direct select procedures
    virtual void direct_select_format(uint16_t  number, String &Output);

    void request_guitar_switch_states();
    void check_inst_switch_states(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void unmute();
    virtual void mute();

    // Parameter control procedures
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    void read_parameter(uint8_t  sw, uint8_t  byte1, uint8_t  byte2);
    void check_update_label(uint8_t  Sw, uint8_t  value);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);

    // Assign control procedures
    virtual void read_assign_name(uint8_t  number, String &Output);
    virtual void read_assign_short_name(uint8_t  number, String &Output);
    virtual void read_assign_trigger(uint8_t  number, String &Output);
    virtual uint8_t  get_number_of_assigns();
    virtual uint8_t  trigger_follow_assign(uint8_t  number);
    virtual void assign_press(uint8_t  Sw, uint8_t  value);
    virtual void assign_release(uint8_t  Sw);
    virtual void assign_load(uint8_t  sw, uint8_t  assign_number, uint8_t  cc_number);
    virtual void request_current_assign(uint8_t  sw);
    void request_complete_assign_area(uint8_t  sw);
    void read_complete_assign_area(uint8_t  sw, uint32_t  address, const unsigned char* sxdata, short unsigned int sxlength);
    void assign_request(uint8_t  sw);
    bool  target_lookup(uint8_t  sw, uint16_t  target);

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    virtual void toggle_expression_pedal(uint8_t  sw);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);
    void update_exp_label(uint8_t  sw);

    // Variables:
    uint8_t  COSM_onoff;
    uint8_t  nrml_pu_onoff;
    uint8_t  assign_read = false; // Assigns are read in three steps on the GP10 1) Read first 64 bytes, 2) Read last 64 bytes, 3) Read targets
    uint8_t  assign_mem[0x80]; // Memory space for the data from the assigns
    uint8_t  exp_type;
    uint8_t  exp_on_type;
    uint8_t  exp_sw_type;
};

// ********************************* Section 3: MD_GR55_class declaration (derived) ********************************************

class MD_GR55_class : public MD_base_class
{
  public:
    MD_GR55_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    virtual void do_after_connect();

    // Midi out procedures
    void write_sysex(uint32_t  address, uint8_t  value);
    void write_sysex(uint32_t  address, uint8_t  value1, uint8_t  value2);
    void request_sysex(uint32_t  address, uint8_t  no_of_bytes);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();
    //void sendGR55BankPatch(uint16_t  patchno);

    // Patch selection procedures
    virtual void select_patch(uint16_t  new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t  calculate_patch_address(uint16_t  number);
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    virtual void request_current_patch_name();
    virtual bool  flash_LEDs_for_patch_bank_switch(uint8_t  sw);
    //void page_check();
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t  number, String &Output);

    // Direct select procedures
    virtual void direct_select_format(uint16_t  number, String &Output);
    virtual void direct_select_start();
    virtual uint16_t  direct_select_patch_number_to_request(uint8_t  number);
    virtual void direct_select_press(uint8_t  number);

    // Device mute procedures
    void request_guitar_switch_states();
    void check_inst_switch_states(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void unmute();
    virtual void mute();
    void mute_now();
    void read_preset_name(uint8_t  number, uint16_t  patch);

    // Parameter control procedures
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    void read_parameter(uint8_t  sw, uint8_t  byte1, uint8_t  byte2);
    void check_update_label(uint8_t  Sw, uint8_t  value);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);

    // Assign control procedures
    virtual void read_assign_name(uint8_t  number, String &Output);
    virtual void read_assign_short_name(uint8_t  number, String &Output);
    virtual void read_assign_trigger(uint8_t  number, String &Output);
    virtual uint8_t  get_number_of_assigns();
    virtual uint8_t  trigger_follow_assign(uint8_t  number);
    virtual void assign_press(uint8_t  Sw, uint8_t  value);
    virtual void assign_release(uint8_t  Sw);
    virtual void assign_load(uint8_t  sw, uint8_t  assign_number, uint8_t  cc_number);
    virtual void request_current_assign(uint8_t  sw);
    void read_current_assign(uint8_t  sw, uint32_t  address, const unsigned char* sxdata, short unsigned int sxlength);
    void assign_request(uint8_t  sw);
    bool  target_lookup(uint8_t  sw, uint16_t  target);

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    virtual void toggle_expression_pedal(uint8_t  sw);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);
    void update_exp_label(uint8_t  sw);

    // Variables:
    uint8_t  preset_banks; // Default number of preset banks
    uint8_t  synth1_onoff;
    uint8_t  synth2_onoff;
    uint8_t  COSM_onoff;
    uint8_t  nrml_pu_onoff;
    bool  was_on = false;
    bool  bass_mode = false;
    uint8_t  exp_type;
    uint8_t  exp_on_type;
    uint8_t  exp_sw_type;
    uint8_t  flash_bank_of_three = 255;
};

// ********************************* Section 4: MD_VG99_class declaration (derived) ********************************************

class MD_VG99_class : public MD_base_class
{
  public:
    MD_VG99_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();
    virtual void update();

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    void check_SYSEX_in_fc300(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    virtual void do_after_connect();

    // Midi out procedures
    void write_sysex(uint32_t  address, uint8_t  value);
    void write_sysex(uint32_t  address, uint8_t  value1, uint8_t  value2);
    void write_sysexfc(uint16_t  address, uint8_t  value);
    void write_sysexfc(uint16_t  address, uint8_t  value1, uint8_t  value2, uint8_t  value3);
    void request_sysex(uint32_t  address, uint8_t  no_of_bytes);
    void control_edit_mode();
    void check_edit_mode_return_timer();
    void set_editor_mode(bool  state);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    // Patch selection procedures
    virtual void select_patch(uint16_t  new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t  calculate_patch_address(uint16_t  number);
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    virtual void request_current_patch_name();
    //void page_check();
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t  number, String &Output);

    // Direct select procedures
    virtual void direct_select_format(uint16_t  number, String &Output);

    // Device mute procedures
    void request_guitar_switch_states();
    void check_inst_switch_states(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void unmute();
    virtual void mute();
    void mute_now();

    // Parameter control procedures
    void count_parameter_categories();
    virtual void request_par_bank_category_name(uint8_t  sw);
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    //virtual uint8_t  read_parameter_numvals(uint16_t  number);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    void read_parameter(uint8_t  sw, uint8_t  byte1, uint8_t  byte2);
    void check_update_label(uint8_t  Sw, uint8_t  value);
    virtual uint16_t  number_of_parameters();
    virtual uint16_t  number_of_parbank_parameters();
    virtual uint16_t  get_parbank_parameter_id(uint16_t  par_number);
    virtual uint8_t  number_of_values(uint16_t  parameter);

    // Assign control procedures
    virtual void read_assign_name(uint8_t  number, String &Output);
    virtual void read_assign_short_name(uint8_t  number, String &Output);
    virtual void read_assign_trigger(uint8_t  number, String &Output);
    virtual uint8_t  get_number_of_assigns();
    virtual uint8_t  trigger_follow_assign(uint8_t  number);
    virtual void assign_press(uint8_t  Sw, uint8_t  value);
    virtual void assign_release(uint8_t  Sw);
    void fix_reverse_pedals();
    virtual void assign_load(uint8_t  sw, uint8_t  assign_number, uint8_t  cc_number);
    virtual void request_current_assign(uint8_t  sw);
    void read_current_assign(uint8_t  sw, uint32_t  address, const unsigned char* sxdata, short unsigned int sxlength);
    void assign_request(uint8_t  sw);
    bool  target_lookup(uint8_t  sw, uint16_t  target);
    //virtual uint8_t  number_of_assigns();

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    virtual void toggle_expression_pedal(uint8_t  sw);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);

    // Variables:
    uint8_t  COSM_A_onoff;
    uint8_t  COSM_B_onoff;
    uint8_t  FC300_device_id;
    bool  edit_mode = false;
    uint32_t  edit_mode_return_timer = 0;
#define VG99_EDIT_MODE_RETURN_TIME 100
    bool edit_return_timer_running = false;
};

// ********************************* Section 5: MD_ZG3_class declaration (derived) ********************************************

class MD_ZG3_class : public MD_base_class
{
  public:
    MD_ZG3_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();
    virtual void update(); // Called in loop() of sketch

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    //virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    virtual void do_after_connect();

    // Midi out procedures
    void write_sysex(uint8_t  message);
    void request_patch(uint8_t  number);
    void set_FX_state(uint8_t  number, uint8_t  state);
    void page_check();
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    // Patch selection procedures
    virtual void select_patch(uint16_t  new_patch);
    void do_after_patch_selection();
    virtual void number_format(uint16_t  number, String &Output);

    // Direct select procedures
    virtual void direct_select_format(uint16_t  number, String &Output);
    //virtual uint16_t  direct_select_patch_number_to_request(uint8_t  number);
    //virtual void direct_select_press(uint8_t  number);

    // Parameter control procedures
    void ZG3_Recall_FXs(uint8_t  Sw);
    //virtual void FX_press(uint8_t  Sw, Cmd_struct *cmd, uint8_t  number);
    //virtual void FX_set_type_and_state(uint8_t  Sw);
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_title_short(uint16_t  number, String &Output);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);

    // Variables:
    const uint8_t  NUMBER_OF_FX_SLOTS = 6; // The Zoom MS70-CDR has 6 FX slots
    uint16_t  FX[6]; //Memory location for the 6 FX
    bool  send_patch_change = false;

#define ZG3_CURRENT_PATCH_DATA_SIZE 110
};

// ********************************* Section 6: MD_ZMS70_class declaration (derived) ********************************************

class MD_ZMS70_class : public MD_base_class
{
  public:
    MD_ZMS70_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();
    virtual void update(); // Called in loop() of sketch

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    uint16_t  FXtypeMangler(uint8_t  byte1, uint8_t  byte2, uint8_t  byte4);
    //virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    virtual void do_after_connect();

    // Midi out procedures
    void write_sysex(uint8_t  message);
    void request_patch(uint8_t  number);
    void set_FX_state(uint8_t  number, uint8_t  state);
    void send_current_patch();
    virtual void set_bpm();
    void write_tempo_to_cpmem();
    virtual void start_tuner();
    virtual void stop_tuner();

    // Patch selection procedures
    virtual void select_patch(uint16_t  new_patch);
    virtual void do_after_patch_selection();
    //void page_check();
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t  number, String &Output);

    // Parameter control procedures
    //void FX_press(uint8_t  Sw, Cmd_struct *cmd, uint8_t  number);
    uint8_t  FXsearch(uint8_t  type, uint8_t  number);
    //void FX_set_type_and_state(uint8_t  Sw);
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_title_short(uint16_t  number, String &Output);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    void write_active_fx_to_cpmem(uint8_t  fx_no);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);

    // Variables
    const uint8_t  NUMBER_OF_FX_SLOTS = 6; // The Zoom MS70-CDR has 6 FX slots
    uint16_t  FX[6]; //Memory location for the 6 FX
#define ZMS70_CURRENT_PATCH_DATA_SIZE 146
    uint8_t  CP_MEM[ZMS70_CURRENT_PATCH_DATA_SIZE]; // Memory to store the current patch data
    bool  CP_MEM_current;
    bool  send_patch_change = false;
};

// ********************************* Section 7: MD_M13_class declaration (derived) ********************************************

class MD_M13_class : public MD_base_class
{
  public:
    MD_M13_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();
    virtual void update();

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);
    virtual void check_CC_in(uint8_t  control, uint8_t  value, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    virtual void do_after_connect();
    virtual void do_after_disconnect();
    virtual void check_still_connected();

    // Midi out procedures
    //void write_sysex(uint32_t  address, uint8_t  value);
    //void write_sysex(uint32_t  address, uint8_t  value1, uint8_t  value2);
    void request_scene(uint8_t  number);
    void send_midi_request_for_scene(uint8_t  number);
    //virtual void set_bpm();
    virtual void bpm_tap();
    virtual void start_tuner();
    virtual void stop_tuner();

    // Patch selection procedures
    virtual void do_after_patch_selection();
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);

    // Parameter control procedures
    uint8_t  FXsearch(uint8_t  type, uint8_t  number);
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_title_short(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    void read_parameter(uint8_t  sw, uint8_t  byte1, uint8_t  byte2);
    //void check_update_label(uint8_t  Sw, uint8_t  value);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    virtual void toggle_expression_pedal(uint8_t  sw);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);

    // Looper procedures
    virtual bool  looper_active();
    virtual void send_looper_cmd(uint8_t  cmd);

    // Array index of par_on
#define M13_PAR_ON_SIZE 4
#define M13_PAR_BYPASS 0
#define M13_PAR_BYPASS_LOOP 1
#define M13_PAR_EXP1 2
#define M13_PAR_EXP2 3


    // Variables:
    bool  par_on[M13_PAR_ON_SIZE]; // Keeps track if the state of the looper and other parameters
    uint8_t  read_scene; // The current scene that is being read
    uint8_t  read_scene_byte; // The byte of the current scene that is being read
    bool  tuner_active;
};

// ********************************* Section 8: MD_HLX_class declaration (derived) ********************************************

class MD_HLX_class : public MD_base_class
{
  public:
    MD_HLX_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();
    virtual void update();
    virtual uint8_t get_number_of_dev_types();
    virtual void get_dev_type_name(uint8_t number, String &name);
    virtual void do_after_dev_type_update();

    // Midi in procedures
    //virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);
    virtual void check_CC_in(uint8_t  control, uint8_t  value, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    virtual void do_after_connect();

    // Midi out procedures
    virtual void set_bpm();
    virtual void bpm_tap();
    virtual void start_tuner();
    virtual void stop_tuner();

    // Patch selection procedures
    virtual void do_after_patch_selection();
    virtual bool  flash_LEDs_for_patch_bank_switch(uint8_t  sw);
    virtual void number_format(uint16_t  number, String &Output);

    // Setlist/song select
    virtual void setlist_song_select(uint16_t item);
    virtual uint16_t setlist_song_get_current_item_state();
    virtual uint16_t setlist_song_get_number_of_items();
    virtual void setlist_song_full_item_format(uint16_t item, String &Output);
    virtual void setlist_song_short_item_format(uint16_t item, String &Output);

    // Direct select procedures
    virtual void direct_select_format(uint16_t  number, String &Output);
    virtual bool  valid_direct_select_switch(uint8_t  number);
    virtual void direct_select_start();
    virtual void direct_select_press(uint8_t  number);

    virtual void unmute();
    virtual void mute();

    // Parameter control procedures
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    void read_parameter(uint8_t  sw, uint8_t  byte1, uint8_t  byte2);
    void check_update_label(uint8_t  Sw, uint8_t  value);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    virtual void toggle_expression_pedal(uint8_t  sw);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);

    // Snapshot/sceme selection procedures
    virtual void get_snapscene_title(uint8_t  number, String &Output);
    //virtual bool  request_snapscene_name(uint8_t  sw, uint8_t sw1, uint8_t sw2, uint8_t sw3);
    virtual void set_snapscene(uint8_t  sw, uint8_t  number);
    virtual void show_snapscene(uint8_t  number);
    virtual void snapscene_number_format(String &Output);

    // Looper procedures
    virtual bool  looper_active();
    virtual void send_looper_cmd(uint8_t  cmd);

    // MIDI sequencer
    void set_sequence(uint8_t  pattern);
    void stop_sequence();
    void set_sequence_beats(uint8_t  beats);
    void send_sequence_step_CC();
    void setup_random_number_generator();
    uint8_t  generate_random_number();

    // MIDI forwarding
    void PC_forwarding(uint8_t  Program, uint8_t  Channel, uint8_t  Port);
    void CC_forwarding(uint8_t  Controller, uint8_t  Value, uint8_t  Channel, uint8_t  Port);

    // Variables:
    //bool  par_on[HLX_PAR_ON_SIZE]; // Keeps track if the state of the looper and other parameters
    bool  tuner_active;
    uint8_t  flash_bank_of_four = 255;
    uint32_t  MIDI_Helix_IC_timer = 0;
    uint8_t  MIDI_Helix_received_msg_no = 0;
    uint8_t  MIDI_Helix_sent_msg_no = 0;
    uint8_t  MIDI_Helix_current_program = 0;
    uint8_t  current_sequence = 0;
    uint8_t  beat_divider = 1;
    uint8_t  number_of_sequence_steps;
    uint8_t  current_sequence_step = 0;
    uint8_t  previous_sequence_value = 0;
    bool  update_sequencer = false;
    uint8_t number_of_snapshots = 8;

#define TYPE_HELIX_01A 0
#define TYPE_HELIX_000 1
#define TYPE_HX_STOMP_01A 2
#define TYPE_HX_STOMP_000 3
#define TYPE_HX_STOMP_XL_01A 4
#define TYPE_HX_STOMP_XL_000 5
#define TYPE_HX_EFFECTS_01A 6
#define TYPE_HX_EFFECTS_000 7
};

// ********************************* Section 9: MD_FAS_class declaration (derived) ********************************************

class MD_FAS_class : public MD_base_class
{
  public:
    MD_FAS_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();
    virtual void update();
    
    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void send_alternative_identity_request(uint8_t  check_device_no);
    void set_type(uint8_t  device_type);
    virtual void do_after_connect();
    virtual void check_still_connected();

    // Midi out procedures
    void write_sysex(uint8_t  byte1);
    void write_sysex(uint8_t  byte1, uint8_t  byte2);
    void write_sysex(uint8_t  byte1, uint8_t  byte2, uint8_t  byte3);
    void write_sysex(uint8_t  byte1, uint8_t  byte2, uint8_t  byte3, uint8_t  byte4, uint8_t  byte5, uint8_t  byte6, uint8_t  byte7, uint8_t  byte8, uint8_t  byte9);
    uint8_t  calc_FS_checksum(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    // Patch selection procedures
    virtual void select_patch(uint16_t  new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t  calculate_patch_address(uint16_t  number);
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    virtual void request_current_patch_name();
    //void page_check();
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t  number, String &Output);

    // Direct select procedures
    virtual void direct_select_format(uint16_t  number, String &Output);

    // Device mute procedures
    virtual void unmute();
    virtual void mute();

    // Parameter control procedures
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    void clear_FX_states();
    void set_FX_state(uint8_t  cc, bool  state);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    //void read_parameter(uint8_t  sw, uint8_t  byte1, uint8_t  byte2);
    void check_update_label(uint8_t  Sw, uint8_t  value);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);
    virtual uint16_t  number_of_parbank_parameters();
    virtual uint16_t  get_parbank_parameter_id(uint16_t  par_number);

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);

    // Snapshot/sceme selection procedures
    virtual void get_snapscene_title(uint8_t  number, String &Output);
    //virtual bool  request_snapscene_name(uint8_t  sw, uint8_t sw1, uint8_t sw2, uint8_t sw3);
    virtual void set_snapscene(uint8_t  sw, uint8_t  number);
    virtual void show_snapscene(uint8_t  number);

    // Looper procedures
    virtual bool  looper_active();
    virtual void send_looper_cmd(uint8_t  cmd);

    // Variables:
#define FAS_NUMBER_OF_FX 93 // FX go fron CC#37 to CC#121 = 85, and we add 8 External values 85 + 8 = 93
#define FAS_FX_FIRST_CC 37
    uint8_t  model_number;
    uint8_t  effect_state[FAS_NUMBER_OF_FX];
    uint8_t  current_scene;
    uint8_t  number_of_active_blocks;
    bool  looper_block_detected;
};

// ********************************* Section 10: MD_KTN_class declaration (derived) ********************************************

class MD_KTN_class : public MD_base_class
{
  public:
    MD_KTN_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();
    virtual void update();
    virtual uint8_t get_number_of_dev_types();
    virtual void get_dev_type_name(uint8_t number, String &name);

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    virtual void forward_MIDI_message(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);
    virtual void forward_PC_message(uint8_t  program, uint8_t  channel);

    // Device connection procedures
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    void respond_to_identity_request_of_editor();
    virtual void do_after_connect();
    void check_default_page_settings();

    // Midi out procedures
    void write_sysex(uint32_t  address, uint8_t  value);
    void write_sysex(uint32_t  address, uint8_t  value1, uint8_t  value2);
    void request_sysex(uint32_t  address, uint8_t  no_of_bytes);
    void write_patch_data(uint32_t  address, uint8_t  index, uint8_t  len);
    virtual void set_bpm();

    // Patch selection procedures
    virtual void select_patch(uint16_t  new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t  calculate_patch_address(uint16_t  number);
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    virtual void request_current_patch_name();
    //void page_check();
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t  number, String &Output);

    // Direct select procedures
    virtual void direct_select_format(uint16_t  number, String &Output);
    virtual bool  valid_direct_select_switch(uint8_t  number);
    virtual void direct_select_start();
    virtual uint16_t  direct_select_patch_number_to_request(uint8_t  number);
    virtual void direct_select_press(uint8_t  number);

    // Patch loading and saving
    void load_patch(uint8_t  number);
    void save_patch();
    void update_patch(uint8_t  version, uint16_t  number);
    void store_patch();
    bool  exchange_patch();
    void request_patch_message(uint8_t  number);
    void read_patch_message(uint8_t  number, const unsigned char* sxdata, short unsigned int sxlength, bool  checksum_ok);
    void skip_patch_message(uint8_t  number, uint8_t  start_index, uint8_t  data_length);
    bool  FX_chain_changed();
    void read_patch_name_from_buffer(String &txt);
    void store_patch_name_to_buffer(String txt);
    void load_patch_buffer_with_default_patch();

    // Parameter control procedures
    void count_parameter_categories();
    virtual void request_par_bank_category_name(uint8_t  sw);
    virtual void par_bank_updown(signed int delta, uint8_t  my_bank_size);
    void show_popup_category(uint16_t  number);
    void show_popup_parameter(uint16_t  number);
    bool  check_parameter_empty(uint16_t  number);
    uint16_t  get_fx_table_index(uint16_t  number);
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    uint32_t  parameter_address(uint8_t  number);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    void read_parameter(uint8_t  sw, uint8_t  byte1, uint8_t  byte2);
    void check_update_label(uint8_t  Sw, uint16_t  value);
    virtual uint16_t  number_of_parameters();
    virtual uint16_t  number_of_parbank_parameters();
    virtual uint16_t  get_parbank_parameter_id(uint16_t  par_number);
    virtual uint8_t  number_of_values(uint16_t  parameter);

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    virtual void toggle_expression_pedal(uint8_t  sw);
    void auto_toggle_exp_pedal(uint8_t  parameter, uint8_t  value);
    void reset_exp_pedal_selection();
    virtual void set_expr_title(uint8_t  sw);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);

    // Variables:
    bool  is_mk2;
    uint8_t  version;
    uint8_t  KTN_patch_buffer[VC_PATCH_SIZE];
#define KTN_FX_CHAIN_SIZE 20
    uint8_t  KTN_FX_chain[KTN_FX_CHAIN_SIZE];
    bool  mod_enabled = false;
    uint8_t  current_mod_type = 0;
    bool  fx_enabled = false;
    uint8_t  current_fx_type = 0;
    uint8_t  current_eq_type = 0;
    uint8_t  current_global_eq_type = 0;
    uint8_t  current_pedal_type = 0;
    uint8_t  current_midi_message; // Used for reading the patch from the Katana
    uint32_t  current_midi_message_address;
    uint16_t  save_patch_number = 0;
    uint32_t  midi_timer;
    uint8_t  prev_channel_number = 255;
    bool  editor_connected = false;
#define KTN_IDENTITY_MESSAGE_SIZE 15
    uint8_t  identity_message[KTN_IDENTITY_MESSAGE_SIZE];

#define TYPE_KTN_100 0
#define TYPE_KTN_50 1
};

// ********************************* Section 11: MD_KPA_class declaration (derived) ********************************************

class MD_KPA_class : public MD_base_class
{
  public:
    MD_KPA_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();
    virtual void update();

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);
    virtual void check_CC_in(uint8_t  control, uint8_t  value, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void check_active_sense_in(uint8_t  port);
    virtual void send_alternative_identity_request(uint8_t  check_device_no);
    virtual void do_after_connect();

    // Midi out procedures
    void write_sysex(uint16_t  address, uint16_t  value);
    void write_sysex_string(uint16_t  address, char *str);
    void send_beacon(uint8_t  setNum, uint8_t  flags, uint8_t  timeLease);
    void request_single_parameter(uint16_t  address);
    void request_current_rig_name();
    void request_performance_name(uint8_t  number);
    void request_owner_name();
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    // Patch selection procedures
    void switch_mode(uint8_t  mode);
    virtual void select_patch(uint16_t  new_patch);
    virtual void do_after_patch_selection();
    //void preselect_performance(uint16_t  number);
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    virtual void request_current_patch_name();
    virtual void number_format(uint16_t  number, String &Output);
    void page_update_timer_check();
    void check_write_performance_name(uint16_t number, String pname);
    void clear_performance_name(uint16_t number);
    bool read_performance_name(uint16_t number, String &pname);
    void check_after_editor_patch_dump();

    // Setlist/song select
    /*virtual void setlist_song_select(uint16_t item);
      virtual uint16_t setlist_song_get_current_item_state();
      virtual uint16_t setlist_song_get_number_of_items();
      virtual void setlist_song_full_item_format(uint16_t item, String &Output);
      virtual void setlist_song_short_item_format(uint16_t item, String &Output);*/

    // Direct select procedures
    virtual void direct_select_format(uint16_t  number, String &Output);
    virtual bool valid_direct_select_switch(uint8_t  number);
    virtual void direct_select_start();
    virtual void direct_select_press(uint8_t  number);

    // Snapshot/sceme selection procedures
    virtual void get_snapscene_title(uint8_t  number, String &Output);
    virtual void get_snapscene_title_short(uint8_t  number, String &Output);
    virtual bool request_snapscene_name(uint8_t  sw, uint8_t sw1, uint8_t sw2, uint8_t sw3);
    virtual void get_snapscene_label(uint8_t  number, String &Output);
    virtual void set_snapscene(uint8_t  sw, uint8_t  number);
    virtual void release_snapscene(uint8_t  sw, uint8_t  number);
    virtual void show_snapscene(uint8_t  number);
    virtual void snapscene_number_format(String &Output);

    // Parameter control procedures
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    void clear_FX_states();
    void set_FX_state(uint8_t  index, bool  state);
    void set_mode(uint8_t mode);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    void read_FX_type(uint8_t  sw, uint8_t  type);
    void read_parameter(uint8_t  sw, uint8_t  byte1, uint8_t  byte2);
    void check_update_label(uint8_t  Sw, uint8_t  value);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    virtual void toggle_expression_pedal(uint8_t  sw);
    virtual void set_expr_title(uint8_t  sw);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);

    // Looper procedures
    virtual bool  looper_active();
    virtual void send_looper_cmd(uint8_t  cmd);
    virtual void looper_release();

    // Variables:
    bool  start_KPA_detection = false;
    bool  tuner_active;
    uint8_t  current_mode;
    uint8_t  current_performance;
    uint16_t  browse_rig_number = 0;
    uint16_t  performance_rig_number = 0;
#define KPA_NUMBER_OF_FX 18
    uint8_t  effect_state[KPA_NUMBER_OF_FX];
    uint8_t  last_looper_cmd;
    String current_snapscene_label;
    bool send_morph_message;
    uint32_t page_update_timer;
#define KPA_PAGE_UPDATE_TIME 500
    uint8_t KPA_name_buffer[VC_PATCH_SIZE];
    uint16_t current_buffer_number = 0;
#define KPA_SLOT_COLOUR LED_BLUE
    uint32_t pc_change_timeout = 0;
#define PC_CHANGE_TIME 1500
    uint8_t performance_on_kpa = 255;
    bool ready_to_read_rig_name = false;
    String last_read_rig_name = "";
    uint8_t last_checked_rig_number = 255;
#define KPA_RIG_BASE_NUMBER 200
    bool rig_browsing = false;
};

// ********************************* Section 12: MD_SVL_class declaration (derived) ********************************************

class MD_SVL_class : public MD_base_class
{
  public:
    MD_SVL_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();

    // Midi in procedures
    //virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);
    virtual void check_CC_in(uint8_t  control, uint8_t  value, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    virtual void do_after_connect();

    // Midi out procedures
    virtual void bpm_tap();

    // Patch selection procedures
    virtual void select_patch(uint16_t  new_patch);
    virtual void do_after_patch_selection();
    virtual void number_format(uint16_t  number, String &Output);

    // Direct select procedures
    virtual void direct_select_format(uint16_t  number, String &Output);

    // Parameter control procedures
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    void read_parameter(uint8_t  sw, uint8_t  byte1, uint8_t  byte2);
    void check_update_label(uint8_t  Sw, uint8_t  value);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);
    void update_parameter_state_through_cc(uint8_t  control, uint8_t  value);

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);

    // Variables:
#define SVL_NUMBER_OF_PARAMETERS 15
    bool  par_on[SVL_NUMBER_OF_PARAMETERS]; // Keeps track of the state the parameters
    uint8_t  flash_bank_of_four = 255;
};

// ********************************* Section 13: MD_SY1000_class declaration (derived) ********************************************

class MD_SY1000_class : public MD_base_class
{
  public:
    MD_SY1000_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();
    virtual void update();

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    virtual void forward_MIDI_message(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);
    virtual void check_CC_in(uint8_t  control, uint8_t  value, uint8_t  channel, uint8_t  port);
    void check_note_in(uint8_t  note, uint8_t  velocity, uint8_t  channel, uint8_t  port);
    uint16_t  sx_index(uint8_t  data3, uint16_t  index);

    // Device connection procedures
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    void respond_to_identity_request_of_editor();
    virtual void do_after_connect();
    virtual void do_after_disconnect();
    void request_first_data_item();
    void request_next_data_item();
    void request_current_data_item();
    void check_midi_timer();

    // Midi out procedures
    void write_sysex(uint32_t  address, uint8_t  value);
    void write_sysex(uint32_t  address, uint8_t  value1, uint8_t  value2);
    void write_sysex(uint32_t  address, uint8_t  value1, uint8_t  value2, uint8_t  value3, uint8_t  value4);
    void write_sysex(uint32_t  address, uint8_t  value1, uint8_t  value2, uint8_t  value3, uint8_t  value4, uint8_t  value5, uint8_t  value6, uint8_t  value7, uint8_t  value8);
    void request_sysex(uint32_t  address, uint8_t  no_of_bytes);
    void control_edit_mode();
    void check_edit_mode_return_timer();
    void set_editor_mode(bool  state);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();
    void check_send_tempo_timer();

    // Patch selection procedures
    virtual void select_patch(uint16_t  new_patch);
    void set_patch_gap_timer();
    void check_patch_gap_timer();
    virtual void do_after_patch_selection();
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    virtual void request_current_patch_name();
    void request_current_patch_number();
    //void page_check();
    //virtual void display_patch_number_string();
    bool  flash_LEDs_for_patch_bank_switch(uint8_t  sw);
    virtual void number_format(uint16_t  number, String &Output);

    // Direct select procedures
    virtual bool  valid_direct_select_switch(uint8_t  number);
    virtual void direct_select_format(uint16_t  number, String &Output);
    virtual void direct_select_start();
    virtual uint16_t  direct_select_patch_number_to_request(uint8_t  number);
    virtual void direct_select_press(uint8_t  number);

    void request_guitar_switch_states();
    void check_inst_switch_states(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void unmute();
    virtual void mute();
    void mute_now();

    // Parameter control procedures
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    uint32_t  read_parameter_address(uint16_t  number);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    void write_parameter_value(uint16_t  number, uint8_t  value);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    void read_parameter(uint8_t  sw, uint8_t  byte1, uint8_t  byte2);
    void check_update_label(uint8_t  Sw, uint8_t  value);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);
    void set_switch_mode(uint8_t  mode);
    void request_switch_mode();
    void check_switch_mode(uint32_t  address, const unsigned char* sxdata, short unsigned int sxlength);
    void cc_operate_switch_mode(uint8_t  sw, uint8_t  value);
    void auto_return_switch_mode();
    void update_leds_on_SY1000();
    void set_LED_colour(uint8_t  sw, uint8_t  colour);
    void set_all_LED_colours(uint8_t  * colour);
    void update_switches_on_page(uint32_t  address, uint8_t  data);

    // Assign control procedures
    uint32_t  calculate_assign_address(uint8_t  number);
    virtual void read_assign_name(uint8_t  number, String &Output);
    virtual void read_assign_short_name(uint8_t  number, String &Output);
    virtual void read_assign_trigger(uint8_t  number, String &Output);
    virtual uint8_t  get_number_of_assigns();
    virtual uint8_t  trigger_follow_assign(uint8_t  number);
    virtual void assign_press(uint8_t  Sw, uint8_t  value);
    virtual void assign_release(uint8_t  Sw);
    void check_delayed_release_assignments();
    virtual void assign_load(uint8_t  sw, uint8_t  assign_number, uint8_t  cc_number);
    virtual void request_current_assign(uint8_t  sw);
    void read_current_assign(uint8_t  sw, uint32_t  address, const unsigned char* sxdata, short unsigned int sxlength);
    void assign_request(uint8_t  sw);
    bool  ctl_target_lookup(uint8_t  sw, uint16_t  target);
    bool  target_lookup(uint8_t  sw, uint16_t  target);
    void request_full_assign(uint8_t  number);
    void read_full_assign(uint8_t  number, uint32_t  address, const unsigned char* sxdata, short unsigned int sxlength);
    void change_active_assign_sources(uint8_t  from_value, uint8_t  to_value);
    void set_assign_settings(uint8_t  assign, uint16_t  target, uint8_t  mode, uint8_t  trigger);
    void restore_assign_settings(uint8_t  assign);
    uint32_t  calculate_full_assign_address(uint8_t  number);
    uint8_t  check_for_scene_assign_source(uint8_t  cc);
    void initialize_scene_assigns();
    void toggle_scene_assign(uint8_t  number);
    void set_scene_assign_states(uint8_t  my_byte);
    uint8_t  read_scene_assign_state();
    void check_scene_assigns_with_new_state(uint8_t  new_byte);
    void show_scene_assign_LEDs();

    // Snapshot/sceme selection procedures
    uint32_t  get_scene_inst_parameter_address(uint16_t  number);
    uint32_t  get_scene_parameter_address(uint16_t  number);
    virtual void get_snapscene_title(uint8_t  number, String &Output);
    virtual void get_snapscene_label(uint8_t  number, String &Output);
    virtual bool  request_snapscene_name(uint8_t  sw, uint8_t sw1, uint8_t sw2, uint8_t sw3);
    virtual void set_snapscene(uint8_t  sw, uint8_t  number);
    virtual void show_snapscene(uint8_t  number);
    void set_snapscene_number_and_LED(uint8_t  number);
    bool  load_scene(uint8_t  prev_scene, uint8_t  new_scene);
    uint8_t  get_scene_index(uint8_t  scene);
    uint8_t  read_scene_data(uint8_t  scene, uint8_t  parameter);
    uint8_t  read_temp_scene_data(uint8_t  parameter);
    void save_scene();
    void request_scene_message(uint8_t  number);
    void check_read_scene_midi_timer();
    void read_scene_message(uint8_t  number, uint8_t  data);
    void store_scene();
    void update_change_on_all_scenes();
    void check_delta_and_update_scenes();
    void exchange_scene(uint8_t  new_scene, uint8_t  prev_scene);
    void initialize_scene(uint8_t  scene);
    virtual bool  check_snapscene_active(uint8_t  scene);
    void set_scene_active(uint8_t  scene);
    void clear_scene_active(uint8_t  scene);
    bool  check_mute_during_scene_change(uint8_t  scene);
    void set_mute_during_scene_change(uint8_t  scene, bool value);
    void read_scene_name_from_buffer(uint8_t  scene);
    void store_scene_name_to_buffer(uint8_t  scene);
    bool  store_patch(uint16_t  number);
    bool  exchange_patches(uint16_t  number);
    bool  insert_patch(uint16_t  number);
    bool  initialize_patch(uint16_t  number);
    void load_patch(uint16_t  number);
    void initialize_patch_space();

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    uint16_t  send_expression_value(uint8_t  exp_pedal, uint8_t  exp_type, uint8_t  value);
    virtual void toggle_expression_pedal(uint8_t  sw);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);
    void update_exp_label(uint8_t  sw);

    // Special functions
    void check_patch_midi(uint8_t  cc, uint8_t  value);
    void reset_special_functions();
    void set_harmony_interval(uint8_t  note, uint8_t  velocity, uint8_t  channel, uint8_t  port);
    uint8_t  calculate_interval(uint8_t  my_type, uint8_t  my_interval, uint8_t  my_note);
    void add_bass_string_assigns();
    //void control_slow_gear(uint8_t  note, uint8_t  velocity, uint8_t  channel, uint8_t  port);
    //void update_slow_gear();
    //void update_string_level(uint8_t  string, uint8_t  level);

    // Variables:
    uint32_t  patch_gap_timer = 0;
    bool  patch_gap_timer_running = false;
#define PATCH_GAP_TIME 500 // The time that the SY1000 blocks sysex messages after sending a PC message
    bool edit_return_timer_running = false;
    uint8_t  data_item = 0;
    uint8_t  INST_onoff[3];
    uint8_t  INST_type[3];
    uint8_t  nrml_pu_onoff;
    uint8_t  exp1_type;
    uint8_t  exp2_type;
    uint8_t  flash_bank_of_four = 255;
    bool  bass_mode = false;
    bool  tuner_active = false;
#define NUMBER_OF_SCENE_ASSIGNS 8
    bool  scene_assign_state[NUMBER_OF_SCENE_ASSIGNS];
    uint8_t  assign_area[16][43];
    uint8_t  read_full_assign_number = 16;
    uint32_t  assign_return_target_timer = 0;
    uint8_t  assign_return_target = 0;
#define SY1000_ASSIGN_RETURN_TIME 1000
    uint32_t  edit_mode_return_timer = 0;
#define SY1000_EDIT_MODE_RETURN_TIME 100 //2500
    uint8_t  SY1000_patch_buffer[VC_PATCH_SIZE];
    uint8_t  save_scene_number;
    uint8_t  last_loaded_scene = 0;
#define SY1000_SCENE_DATA_BUFFER_SIZE 15
    uint8_t  scene_data_buffer[SY1000_SCENE_DATA_BUFFER_SIZE];
    char scene_label_buffer[8];
    bool mute_during_scene_change;
    uint16_t  read_scene_parameter_number;
    uint32_t  read_scene_parameter_address;
    uint32_t  read_scene_midi_timer;
    uint32_t  check_ample_time_between_pc_messages_timer = 0;
#define CHECK_AMPLE_TIME_BETWEEN_PC_MESSAGES_TIME 1000
    uint8_t  switch_mode = 0;
    uint8_t  prev_switch_mode = 0;
    bool  request_LED_state = false;
    bool  manual_mode = false;
    bool  edit_mode = false;
    bool  edit_mode_always_on = true;
    bool editor_connected = false;
    uint8_t  prev_switch_mode_cc = 0;
    uint32_t  send_tempo_timer = 0;
#define CHECK_SEND_TEMPO_TIME 500
    uint8_t  inst_harmony_type[3] = { 0 };
    uint8_t  inst_harmony_interval[3] = { 0 };
    uint8_t  master_key = 0;
    uint8_t  current_string_interval[3][6] = {{ 0 }};
#define SY1000_MIDI_TIME 500
    //uint32_t  slow_gear_timer = 0;
    //uint8_t  slow_gear_string_level[6] = { 0 };
    //bool  slow_gear_string_updown[6] = { true };
#define SY1000_IDENTITY_MESSAGE_SIZE 15
    uint8_t  identity_message[SY1000_IDENTITY_MESSAGE_SIZE];
};

// ********************************* Section 14: MD_GM2_class declaration (derived) ********************************************

class MD_GM2_class : public MD_base_class
{
  public:
    MD_GM2_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();
    virtual void update(); // Called in loop() of sketch

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port);
    virtual void do_after_connect();

    // Midi out procedures
    void write_sysex(uint8_t  address, uint8_t  data1, uint8_t  data2);
    void request_sysex(uint8_t  address);
    void request_patch(uint16_t  number);
    void request_current_patch();
    //void start_editor_mode();
    void page_check();
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    // Patch selection procedures
    virtual void select_patch(uint16_t  new_patch);
    void do_after_patch_selection();
    virtual void number_format(uint16_t  number, String &Output);

    // Direct select procedures
    virtual void direct_select_format(uint16_t  number, String &Output);
    //virtual uint16_t  direct_select_patch_number_to_request(uint8_t  number);
    //virtual void direct_select_press(uint8_t  number);

    // Parameter control procedures
    void GM2_Recall_FXs(uint8_t  Sw);
    //virtual void FX_press(uint8_t  Sw, Cmd_struct *cmd, uint8_t  number);
    //virtual void FX_set_type_and_state(uint8_t  Sw);
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    void read_FX_parameter(uint8_t  sw, uint8_t  byte);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);

    // Master expression pedal procedures
    virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);

    // Variables:
#define GM2_NUMBER_OF_FX 8
    uint8_t  FX_state[GM2_NUMBER_OF_FX];
    uint8_t  FX_type[GM2_NUMBER_OF_FX];
    bool  send_patch_change = false;
};

// ********************************* Section 15: MD_MG300_class declaration (derived) ********************************************

class MD_MG300_class : public MD_base_class
{
  public:
    MD_MG300_class (uint8_t  _dev_no) : MD_base_class (_dev_no) {} // Constructor

    // Basic procedures
    virtual void init();
    virtual void update();

    // Midi in procedures
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t  port);
    void read_patch_name(const unsigned char* sxdata, short unsigned int sxlength);
    char convert_char(uint8_t  data);
    //virtual void check_PC_in(uint8_t  program, uint8_t  channel, uint8_t  port);
    virtual void check_CC_in(uint8_t  control, uint8_t  value, uint8_t  channel, uint8_t  port);

    // Device connection procedures
    virtual void send_alternative_identity_request(uint8_t  check_device_no);
    virtual void do_after_connect();

    // Midi out procedures
    void request_patch(uint8_t  number);
    void request_current_patch();
    void request_device_state();
    virtual void set_bpm();

    // Patch selection procedures
    virtual void select_patch(uint16_t  new_patch);
    virtual void do_after_patch_selection();
    void delayed_after_connect_and_patch_selection();
    virtual bool  request_patch_name(uint8_t  sw, uint16_t  number);
    virtual void number_format(uint16_t  number, String &Output);

    // Direct select procedures
    virtual bool  flash_LEDs_for_patch_bank_switch(uint8_t  sw);
    virtual void direct_select_format(uint16_t  number, String &Output);
    virtual bool  valid_direct_select_switch(uint8_t  number);
    virtual void direct_select_start();
    virtual void direct_select_press(uint8_t  number);

    // Parameter control procedures
    virtual void read_parameter_title(uint16_t  number, String &Output);
    virtual void read_parameter_name(uint16_t  number, String &Output);
    virtual void read_parameter_value_name(uint16_t  number, uint16_t  value, String &Output);
    virtual void parameter_press(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual void parameter_release(uint8_t  Sw, Cmd_struct *cmd, uint16_t  number);
    virtual bool  request_parameter(uint8_t  sw, uint16_t  number);
    void read_parameter(uint8_t  sw, uint8_t  byte1, uint8_t  byte2);
    void read_FX_state_from_memory();
    //void check_update_label(uint8_t  Sw, uint8_t  value);
    virtual uint16_t  number_of_parameters();
    virtual uint8_t  number_of_values(uint16_t  parameter);
    void update_parameter_state_through_cc(uint8_t  control, uint8_t  value);

    // Master expression pedal procedures
    //virtual void move_expression_pedal(uint8_t  sw, uint8_t  value, uint8_t  exp_pedal);
    //virtual bool  request_exp_pedal(uint8_t  sw, uint8_t  exp_pedal);

    // Variables:
#define MG300_NUMBER_OF_FX 9
    uint8_t  fx_type[MG300_NUMBER_OF_FX];
    bool  fx_state[MG300_NUMBER_OF_FX];
    uint8_t  flash_bank_of_four = 255;
    bool  current_patch_read = false;
    //uint8_t  expr_pedal_cc = 0;
    uint32_t  current_patch_request_timer = 0;
    uint16_t  current_checksum;
};
