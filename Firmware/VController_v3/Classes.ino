// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: Device_class declaration (base)
// Section 2: GP10_class declaration (derived)
// Section 3: GR55_class declaration (derived)
// Section 4: VG99_class declaration (derived)
// Section 5: ZG3_class declaration (derived)
// Section 6: ZMS70_class declaration (derived)
// Section 7: M13_class declaration (derived)
// Section 8: HELIX_class declaration (derived)
// Section 9: AXEFX_class declaration (derived)
// Section 10: KTN_class declaration (derived)

// Here we declare the class objects for the devices the VController supports. The actual class code is found in the MIDI_<device_name>.ino files.

// We use a derived class structure here.
// The base class is Device_class. For each device there is a derived class, where certain functions are replaced to add device specific functionality.
// For a derived function to work, "virtual" must be declared in the header of the base class! I have forgotten this a number of times, so this is a reminder to self!

// ********************************* Section 1: Device_class declaration (base) ********************************************

#define NUMBER_OF_DEVICE_SETTINGS 11

class Device_class
{ // Base class for devices

  public:
    // Procedures:
    Device_class (uint8_t _dev_no); // Constructor

    virtual void init();
    virtual void update();
    uint8_t get_setting(uint8_t variable);
    void set_setting(uint8_t variable, uint8_t value);

    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void forward_MIDI_message(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void check_PC_in(uint8_t program, uint8_t channel, uint8_t port);
    virtual void forward_PC_message(uint8_t program, uint8_t channel);
    virtual void check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port);
    virtual void check_still_connected();
    void check_manual_connection();
    bool can_request_sysex_data();
    virtual void send_alternative_identity_request(uint8_t check_device_no);
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    void connect(uint8_t device_id, uint8_t port);
    virtual void do_after_connect();
    void disconnect();
    virtual void do_after_disconnect();
    void check_sysex_delay();
    uint8_t calc_Roland_checksum(uint16_t sum);
    virtual void set_bpm();
    virtual void bpm_tap();
    virtual void start_tuner();
    virtual void stop_tuner();

    virtual void select_patch(uint16_t new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t calculate_patch_address(uint16_t number);
    virtual bool request_patch_name(uint8_t sw, uint16_t number);
    virtual void request_current_patch_name();
    uint16_t calculate_patch_number(uint8_t bank_position, uint8_t bank_size);
    void patch_select_pressed(uint16_t new_patch);
    uint16_t prev_patch_number();
    uint16_t next_patch_number();
    void bank_updown(bool updown, uint8_t my_bank_size);
    bool bank_selection_active();
    void update_bank_number(uint16_t number);
    void update_bank_size(uint8_t b_size);
    virtual bool flash_LEDs_for_patch_bank_switch(uint8_t sw);
    void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);
    virtual bool valid_direct_select_switch(uint8_t number);
    virtual void direct_select_start();
    virtual uint16_t direct_select_patch_number_to_request(uint8_t number);
    virtual void direct_select_press(uint8_t number);

    virtual void unmute();
    virtual void mute();
    void select_switch();
    void is_always_on_toggle();

    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_value_name(uint16_t number, uint16_t value, String &Output);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint8_t sw, uint16_t number);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);
    virtual uint16_t number_of_parbank_parameters();
    virtual uint16_t get_parbank_parameter_id(uint16_t par_number);
    virtual void request_par_bank_category_name(uint8_t sw);
    virtual void select_parameter_bank_category(uint8_t category);
    void par_bank_updown(bool updown, uint8_t my_bank_size);
    
    virtual void read_assign_name(uint8_t number, String &Output);
    virtual void read_assign_trigger(uint8_t number, String &Output);
    virtual uint8_t get_number_of_assigns();
    virtual uint8_t trigger_follow_assign(uint8_t number);
    virtual void assign_press(uint8_t Sw, uint8_t value);
    virtual void assign_release(uint8_t Sw);
    virtual void assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number);
    virtual void request_current_assign(uint8_t sw);
    //virtual uint8_t number_of_assigns();

    virtual void FX_press(uint8_t Sw, Cmd_struct *cmd, uint8_t number);
    virtual void FX_set_type_and_state(uint8_t Sw);

    virtual void move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal);
    virtual void toggle_expression_pedal(uint8_t sw);
    virtual void set_expr_title(uint8_t sw);
    virtual bool request_exp_pedal(uint8_t sw, uint8_t exp_pedal);

    uint8_t read_current_device_page();
    uint8_t select_next_device_page();

    virtual void set_snapscene_name(uint8_t number, String &Output);
    virtual void set_snapscene(uint8_t number);

    virtual bool looper_active();
    uint8_t show_looper_LED(uint8_t sw);
    virtual void request_looper_label(uint8_t sw);
    void looper_press(uint8_t looper_cmd);
    void looper_reset();
    void looper_timer_check();
    virtual void send_looper_cmd(uint8_t cmd);

    // Variables:
    uint8_t enabled; //Is this device enabled: state can be 0: OFF, 1: ON or 2: DETECT
    uint8_t my_device_number;
    char device_name[8];
    char full_device_name[17];
    uint8_t my_LED_colour;  // Variable no.1
    uint8_t MIDI_channel;  // Variable no.2
    uint8_t MIDI_port = 0;
    uint8_t MIDI_port_manual = 0; // Variable no.4
    uint8_t MIDI_device_id; // Variable no.3
    uint16_t patch_number = 0;
    uint8_t patch_number_offset = 1; // Is the first patch numbered one or zero?
    uint32_t PC_ignore_timer;
    String current_patch_name;
    uint16_t patch_min;
    uint16_t patch_max;
    uint16_t bank_number = 1; // The bank number of the current_patch
    uint16_t bank_select_number = 1; // The bank number used for bank up/down
    uint16_t bank_size = 10;
    uint8_t CC01 = 0;
    uint8_t direct_select_state = 0;
    //uint8_t bank_select_number;
    bool connected = false;
    uint8_t no_response_counter = 0;
    uint8_t max_times_no_response;
    bool is_on = false;
    bool is_always_on = true;
    bool request_onoff = false;
    unsigned long sysex_delay_length;// time between sysex messages (in msec).
    unsigned long sysexDelay = 0;
    uint32_t last_requested_sysex_address;
    uint8_t last_requested_sysex_type;
    uint8_t last_requested_sysex_switch;
    uint16_t last_requested_sysex_patch_number;
    uint16_t parameter_bank_number = 0;
    uint8_t parameter_bank_category = 0;
    uint8_t current_setlist = 0; // Needed for Helix and AxeFX
    uint8_t current_snapscene = 0; // For Helix snapshots or AxeFX scenes
    uint8_t current_exp_pedal = 0;
    uint32_t midi_timer; // VController hangs if not declared here! Really don't know why. Only need it for the M13
    uint8_t current_looper_state;
    bool looper_show = false;
    bool loop_recorded = false;
    bool looper_reverse = false;
    bool looper_undone = false;
    bool looper_half_speed = false;
    bool looper_pre = false;
    bool looper_overdub_happened = false;
    uint32_t looper_start_time = 0;
    uint32_t looper_end_time = 0;
    uint32_t current_loop_time;

    // Default pages
    uint8_t my_device_page1 = 0;  // Variable no.4
    uint8_t my_device_page2 = 0;  // Variable no.5
    uint8_t my_device_page3 = 0;  // Variable no.6
    uint8_t my_device_page4 = 0;  // Variable no.7
    uint8_t current_device_page = 0; // The page that is selected
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

uint8_t device_in_bank_selection = 0; // One global variable for all devices to keep track of bank selection

// ********************************* Section 2: GP10_class declaration (derived) ********************************************

class GP10_class : public Device_class
{
  public:
    GP10_class (uint8_t _dev_no) : Device_class (_dev_no) {} // Constructor

    virtual void init();

    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void check_PC_in(uint8_t program, uint8_t channel, uint8_t port);
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void do_after_connect();

    void write_sysex(uint32_t address, uint8_t value);
    void write_sysex(uint32_t address, uint8_t value1, uint8_t value2);
    void request_sysex(uint32_t address, uint8_t no_of_bytes);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    virtual void do_after_patch_selection();
    //virtual uint32_t calculate_patch_address(uint16_t number);
    virtual bool request_patch_name(uint8_t sw, uint16_t number);
    virtual void request_current_patch_name();
    //void page_check();
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);

    void request_guitar_switch_states();
    void check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void unmute();
    virtual void mute();

    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_value_name(uint16_t number, uint16_t value, String &Output);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint8_t sw, uint16_t number);
    void read_parameter(uint8_t sw, uint8_t byte1, uint8_t byte2);
    void check_update_label(uint8_t Sw, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);

    virtual void read_assign_name(uint8_t number, String &Output);
    virtual void read_assign_trigger(uint8_t number, String &Output);
    virtual uint8_t get_number_of_assigns();
    virtual uint8_t trigger_follow_assign(uint8_t number);
    virtual void assign_press(uint8_t Sw, uint8_t value);
    virtual void assign_release(uint8_t Sw);
    virtual void assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number);
    virtual void request_current_assign(uint8_t sw);
    void request_complete_assign_area(uint8_t sw);
    void read_complete_assign_area(uint8_t sw, uint32_t address, const unsigned char* sxdata, short unsigned int sxlength);
    void assign_request(uint8_t sw);
    bool target_lookup(uint8_t sw, uint16_t target);

    virtual void move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal);
    virtual void toggle_expression_pedal(uint8_t sw);
    virtual bool request_exp_pedal(uint8_t sw, uint8_t exp_pedal);
    void update_exp_label(uint8_t sw);

    // Variables:
    uint8_t COSM_onoff;
    uint8_t nrml_pu_onoff;
    uint8_t assign_read = false; // Assigns are read in three steps on the GP10 1) Read first 64 bytes, 2) Read last 64 bytes, 3) Read targets
    uint8_t assign_mem[0x80]; // Memory space for the data from the assigns
    uint8_t exp_type;
    uint8_t exp_on_type;
    uint8_t exp_sw_type;
};

// ********************************* Section 3: GR55_class declaration (derived) ********************************************

class GR55_class : public Device_class
{
  public:
    GR55_class (uint8_t _dev_no) : Device_class (_dev_no) {} // Constructor

    virtual void init();

    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void check_PC_in(uint8_t program, uint8_t channel, uint8_t port);
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void do_after_connect();

    void write_sysex(uint32_t address, uint8_t value);
    void write_sysex(uint32_t address, uint8_t value1, uint8_t value2);
    void request_sysex(uint32_t address, uint8_t no_of_bytes);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();
    void sendGR55BankPatch(uint16_t patchno);

    virtual void select_patch(uint16_t new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t calculate_patch_address(uint16_t number);
    virtual bool request_patch_name(uint8_t sw, uint16_t number);
    virtual void request_current_patch_name();
    virtual bool flash_LEDs_for_patch_bank_switch(uint8_t sw);
    //void page_check();
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);
    virtual void direct_select_start();
    virtual uint16_t direct_select_patch_number_to_request(uint8_t number);
    virtual void direct_select_press(uint8_t number);

    void request_guitar_switch_states();
    void check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void unmute();
    virtual void mute();
    void mute_now();
    void read_preset_name(uint8_t number, uint16_t patch);

    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_value_name(uint16_t number, uint16_t value, String &Output);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint8_t sw, uint16_t number);
    void read_parameter(uint8_t sw, uint8_t byte1, uint8_t byte2);
    void check_update_label(uint8_t Sw, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);

    virtual void read_assign_name(uint8_t number, String &Output);
    virtual void read_assign_trigger(uint8_t number, String &Output);
    virtual uint8_t get_number_of_assigns();
    virtual uint8_t trigger_follow_assign(uint8_t number);
    virtual void assign_press(uint8_t Sw, uint8_t value);
    virtual void assign_release(uint8_t Sw);
    virtual void assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number);
    virtual void request_current_assign(uint8_t sw);
    void read_current_assign(uint8_t sw, uint32_t address, const unsigned char* sxdata, short unsigned int sxlength);
    void assign_request(uint8_t sw);
    bool target_lookup(uint8_t sw, uint16_t target);

    virtual void move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal);
    virtual void toggle_expression_pedal(uint8_t sw);
    virtual bool request_exp_pedal(uint8_t sw, uint8_t exp_pedal);
    void update_exp_label(uint8_t sw);

    // Variables:
    uint8_t preset_banks; // Default number of preset banks
    uint8_t synth1_onoff;
    uint8_t synth2_onoff;
    uint8_t COSM_onoff;
    uint8_t nrml_pu_onoff;
    bool was_on = false;
    bool bass_mode = false;
    uint8_t exp_type;
    uint8_t exp_on_type;
    uint8_t exp_sw_type;
    uint8_t flash_bank_of_three = 255;
};

// ********************************* Section 4: VG99_class declaration (derived) ********************************************

class VG99_class : public Device_class
{
  public:
    VG99_class (uint8_t _dev_no) : Device_class (_dev_no) {} // Constructor

    virtual void init();
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    void check_SYSEX_in_fc300(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void check_PC_in(uint8_t program, uint8_t channel, uint8_t port);
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void do_after_connect();

    void write_sysex(uint32_t address, uint8_t value);
    void write_sysex(uint32_t address, uint8_t value1, uint8_t value2);
    void write_sysexfc(uint16_t address, uint8_t value);
    void write_sysexfc(uint16_t address, uint8_t value1, uint8_t value2, uint8_t value3);
    void request_sysex(uint32_t address, uint8_t no_of_bytes);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    virtual void select_patch(uint16_t new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t calculate_patch_address(uint16_t number);
    virtual bool request_patch_name(uint8_t sw, uint16_t number);
    virtual void request_current_patch_name();
    //void page_check();
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);

    void request_guitar_switch_states();
    void check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void unmute();
    virtual void mute();
    void mute_now();

    void count_parameter_categories();
    virtual void request_par_bank_category_name(uint8_t sw);
    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_value_name(uint16_t number, uint16_t value, String &Output);
    //virtual uint8_t read_parameter_numvals(uint16_t number);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint8_t sw, uint16_t number);
    void read_parameter(uint8_t sw, uint8_t byte1, uint8_t byte2);
    void check_update_label(uint8_t Sw, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint16_t number_of_parbank_parameters();
    virtual uint16_t get_parbank_parameter_id(uint16_t par_number);
    virtual uint8_t number_of_values(uint16_t parameter);

    virtual void read_assign_name(uint8_t number, String &Output);
    virtual void read_assign_trigger(uint8_t number, String &Output);
    virtual uint8_t get_number_of_assigns();
    virtual uint8_t trigger_follow_assign(uint8_t number);
    virtual void assign_press(uint8_t Sw, uint8_t value);
    virtual void assign_release(uint8_t Sw);
    void fix_reverse_pedals();
    virtual void assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number);
    virtual void request_current_assign(uint8_t sw);
    void read_current_assign(uint8_t sw, uint32_t address, const unsigned char* sxdata, short unsigned int sxlength);
    void assign_request(uint8_t sw);
    bool target_lookup(uint8_t sw, uint16_t target);
    //virtual uint8_t number_of_assigns();

    virtual void move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal);
    virtual void toggle_expression_pedal(uint8_t sw);
    virtual bool request_exp_pedal(uint8_t sw, uint8_t exp_pedal);

    // Variables:
    uint8_t COSM_A_onoff;
    uint8_t COSM_B_onoff;
    uint8_t FC300_device_id;
    bool editor_mode = false; // Indicates that the VG-99 is in editor mode
};

// ********************************* Section 5: ZG3_class declaration (derived) ********************************************

class ZG3_class : public Device_class
{
  public:
    ZG3_class (uint8_t _dev_no) : Device_class (_dev_no) {} // Constructor

    virtual void init();
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    //virtual void check_PC_in(uint8_t program, uint8_t channel, uint8_t port);
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void do_after_connect();
    void write_sysex(uint8_t message);
    void request_patch(uint8_t number);
    void set_FX_state(uint8_t number, uint8_t state);
    void page_check();
    virtual bool request_patch_name(uint8_t sw, uint16_t number);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    void do_after_patch_selection();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);
    //virtual uint16_t direct_select_patch_number_to_request(uint8_t number);
    //virtual void direct_select_press(uint8_t number);
    void ZG3_Recall_FXs(uint8_t Sw);

    //virtual void FX_press(uint8_t Sw, Cmd_struct *cmd, uint8_t number);
    //virtual void FX_set_type_and_state(uint8_t Sw);
    virtual bool request_parameter(uint8_t sw, uint16_t number);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);
    virtual void read_parameter_value_name(uint16_t number, uint16_t value, String &Output);

    // Variables:
    const uint8_t NUMBER_OF_FX_SLOTS = 6; // The Zoom MS70-CDR has 6 FX slots
    uint16_t FX[6]; //Memory location for the 6 FX
    
#define ZG3_CURRENT_PATCH_DATA_SIZE 110
};

// ********************************* Section 6: ZMS70_class declaration (derived) ********************************************

class ZMS70_class : public Device_class
{
  public:
    ZMS70_class (uint8_t _dev_no) : Device_class (_dev_no) {} // Constructor

    virtual void init();
    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    uint16_t FXtypeMangler(uint8_t byte1, uint8_t byte2, uint8_t byte4);
    //virtual void check_PC_in(uint8_t program, uint8_t channel, uint8_t port);
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void do_after_connect();
    void write_sysex(uint8_t message);
    void request_patch(uint8_t number);
    void set_FX_state(uint8_t number, uint8_t state);
    void send_current_patch();
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    virtual void do_after_patch_selection();
    //void page_check();
    virtual bool request_patch_name(uint8_t sw, uint16_t number);
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);

    //void FX_press(uint8_t Sw, Cmd_struct *cmd, uint8_t number);
    uint8_t FXsearch(uint8_t type, uint8_t number);
    //void FX_set_type_and_state(uint8_t Sw);

    virtual bool request_parameter(uint8_t sw, uint16_t number);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);
    virtual void read_parameter_value_name(uint16_t number, uint16_t value, String &Output);

    // Variables
    const uint8_t NUMBER_OF_FX_SLOTS = 6; // The Zoom MS70-CDR has 6 FX slots
    uint16_t FX[6]; //Memory location for the 6 FX
#define ZMS70_CURRENT_PATCH_DATA_SIZE 146
    uint8_t CP_MEM[ZMS70_CURRENT_PATCH_DATA_SIZE]; // Memory to store the current patch data
    bool CP_MEM_current;
};

// ********************************* Section 7: M13_class declaration (derived) ********************************************

class M13_class : public Device_class
{
  public:
    M13_class (uint8_t _dev_no) : Device_class (_dev_no) {} // Constructor

    virtual void init();
    virtual void update();

    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void check_PC_in(uint8_t program, uint8_t channel, uint8_t port);
    virtual void check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port);
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void do_after_connect();
    virtual void do_after_disconnect();
    virtual void check_still_connected();

    //void write_sysex(uint32_t address, uint8_t value);
    //void write_sysex(uint32_t address, uint8_t value1, uint8_t value2);
    void request_scene(uint8_t number);
    //virtual void set_bpm();
    virtual void bpm_tap();
    virtual void start_tuner();
    virtual void stop_tuner();

    virtual void do_after_patch_selection();
    virtual bool request_patch_name(uint8_t sw, uint16_t number);

    uint8_t FXsearch(uint8_t type, uint8_t number);
    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_value_name(uint16_t number, uint16_t value, String &Output);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint8_t sw, uint16_t number);
    void read_parameter(uint8_t sw, uint8_t byte1, uint8_t byte2);
    //void check_update_label(uint8_t Sw, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);

    virtual void move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal);
    virtual void toggle_expression_pedal(uint8_t sw);
    virtual bool request_exp_pedal(uint8_t sw, uint8_t exp_pedal);

    virtual bool looper_active();
    virtual void send_looper_cmd(uint8_t cmd);

    // Array index of par_on
#define M13_PAR_ON_SIZE 4
#define M13_PAR_BYPASS 0
#define M13_PAR_BYPASS_LOOP 1
#define M13_PAR_EXP1 2
#define M13_PAR_EXP2 3


    // Variables:
    bool par_on[M13_PAR_ON_SIZE]; // Keeps track if the state of the looper and other parameters
    uint8_t read_scene; // The current scene that is being read
    uint8_t read_scene_byte; // The byte of the current scene that is being read
    bool tuner_active;
};

// ********************************* Section 8: HLX_class declaration (derived) ********************************************

class HLX_class : public Device_class
{
  public:
    HLX_class (uint8_t _dev_no) : Device_class (_dev_no) {} // Constructor

    virtual void init();
    virtual void update();

    //virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void check_PC_in(uint8_t program, uint8_t channel, uint8_t port);
    //virtual void check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port);
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void do_after_connect();

    virtual void bpm_tap();
    virtual void start_tuner();
    virtual void stop_tuner();

    virtual void do_after_patch_selection();
    virtual bool flash_LEDs_for_patch_bank_switch(uint8_t sw);
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);
    virtual bool valid_direct_select_switch(uint8_t number);
    virtual void direct_select_start();
    virtual void direct_select_press(uint8_t number);

    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_value_name(uint16_t number, uint16_t value, String &Output);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint8_t sw, uint16_t number);
    void read_parameter(uint8_t sw, uint8_t byte1, uint8_t byte2);
    void check_update_label(uint8_t Sw, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);

    virtual void move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal);
    virtual void toggle_expression_pedal(uint8_t sw);
    virtual bool request_exp_pedal(uint8_t sw, uint8_t exp_pedal);

    virtual void set_snapscene_name(uint8_t number, String &Output);
    virtual void set_snapscene(uint8_t number);

    virtual bool looper_active();
    virtual void send_looper_cmd(uint8_t cmd);

    // Variables:
    //bool par_on[HLX_PAR_ON_SIZE]; // Keeps track if the state of the looper and other parameters
    bool tuner_active;
    uint8_t flash_bank_of_four = 255;
};

// ********************************* Section 9: AXEFX_class declaration (derived) ********************************************

class AXEFX_class : public Device_class
{
  public:
    AXEFX_class (uint8_t _dev_no) : Device_class (_dev_no) {} // Constructor

    virtual void init();
    virtual void update();

    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void check_PC_in(uint8_t program, uint8_t channel, uint8_t port);
    virtual void send_alternative_identity_request(uint8_t check_device_no);
    void set_type(uint8_t device_type);
    virtual void do_after_connect();
    virtual void check_still_connected();

    void write_sysex(uint8_t byte1);
    void write_sysex(uint8_t byte1, uint8_t byte2);
    void write_sysex(uint8_t byte1, uint8_t byte2, uint8_t byte3);
    void write_sysex(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8, uint8_t byte9);
    uint8_t calc_FS_checksum(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    virtual void select_patch(uint16_t new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t calculate_patch_address(uint16_t number);
    virtual bool request_patch_name(uint8_t sw, uint16_t number);
    virtual void request_current_patch_name();
    //void page_check();
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);
    virtual void unmute();
    virtual void mute();

    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_value_name(uint16_t number, uint16_t value, String &Output);
    void clear_FX_states();
    void set_FX_state(uint8_t cc, bool state);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint8_t sw, uint16_t number);
    //void read_parameter(uint8_t sw, uint8_t byte1, uint8_t byte2);
    void check_update_label(uint8_t Sw, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);
    virtual uint16_t number_of_parbank_parameters();
    virtual uint16_t get_parbank_parameter_id(uint16_t par_number);

    virtual void move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal);
    virtual bool request_exp_pedal(uint8_t sw, uint8_t exp_pedal);

    virtual void set_snapscene_name(uint8_t number, String &Output);
    virtual void set_snapscene(uint8_t number);

    virtual bool looper_active();
    virtual void send_looper_cmd(uint8_t cmd);

    // Variables:
#define AXEFX_NUMBER_OF_FX 93 // FX go fron CC#37 to CC#121 = 85, and we add 8 External values 85 + 8 = 93
#define AXE_FX_FIRST_CC 37
    uint8_t model_number;
    uint8_t effect_state[AXEFX_NUMBER_OF_FX];
    uint8_t current_scene;
    uint8_t number_of_active_blocks;
    bool looper_block_detected;
};

// ********************************* Section 10: KTN_class declaration (derived) ********************************************

class KTN_class : public Device_class
{
  public:
    KTN_class (uint8_t _dev_no) : Device_class (_dev_no) {} // Constructor

    virtual void init();
    virtual void update();

    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void forward_MIDI_message(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void check_PC_in(uint8_t program, uint8_t channel, uint8_t port);
    virtual void forward_PC_message(uint8_t program, uint8_t channel);
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void do_after_connect();

    void write_sysex(uint32_t address, uint8_t value);
    void write_sysex(uint32_t address, uint8_t value1, uint8_t value2);
    void request_sysex(uint32_t address, uint8_t no_of_bytes);
    void write_patch_data(uint32_t address, uint8_t index, uint8_t len);
    virtual void set_bpm();
    
    virtual void select_patch(uint16_t new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t calculate_patch_address(uint16_t number);
    virtual bool request_patch_name(uint8_t sw, uint16_t number);
    virtual void request_current_patch_name();
    //void page_check();
    //virtual void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);
    virtual bool valid_direct_select_switch(uint8_t number);
    virtual void direct_select_start();
    virtual uint16_t direct_select_patch_number_to_request(uint8_t number);
    virtual void direct_select_press(uint8_t number);
    
    void load_patch(uint8_t number);
    void save_patch();
    void store_patch();
    bool exchange_patch();
    void request_patch_message(uint8_t number);
    void read_patch_message(uint8_t number, const unsigned char* sxdata, short unsigned int sxlength, bool checksum_ok);
    bool FX_chain_changed();
    void read_patch_name_from_buffer(String &txt);
    void store_patch_name_to_buffer(String txt);
    void load_patch_buffer_with_default_patch();

    void count_parameter_categories();
    virtual void request_par_bank_category_name(uint8_t sw);
    uint16_t get_fx_table_index(uint16_t number);
    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_value_name(uint16_t number, uint16_t value, String &Output);
    uint32_t parameter_address(uint8_t number);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint8_t sw, uint16_t number);
    void read_parameter(uint8_t sw, uint8_t byte1, uint8_t byte2);
    void check_update_label(uint8_t Sw, uint16_t value);
    virtual uint16_t number_of_parameters();
    virtual uint16_t number_of_parbank_parameters();
    virtual uint16_t get_parbank_parameter_id(uint16_t par_number);
    virtual uint8_t number_of_values(uint16_t parameter);

    virtual void move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal);
    virtual void toggle_expression_pedal(uint8_t sw);
    void auto_toggle_exp_pedal(uint8_t parameter, uint8_t value);
    virtual void set_expr_title(uint8_t sw);
    virtual bool request_exp_pedal(uint8_t sw, uint8_t exp_pedal);

    // Variables:
#define KTN_PATCH_SIZE 176
    uint8_t KTN_patch_buffer[KTN_PATCH_SIZE];
#define KTN_FX_CHAIN_SIZE 20
    uint8_t KTN_FX_chain[KTN_FX_CHAIN_SIZE];
    bool mod_enabled = false;
    uint8_t current_mod_type = 0; 
    bool fx_enabled = false;
    uint8_t current_fx_type = 0;
    uint8_t current_midi_message; // Used for reading the patch from the Katana
    uint32_t current_midi_message_address;
    uint8_t save_patch_number = 0;
    uint32_t midi_timer;
    uint8_t prev_patch_number = 255;
};


