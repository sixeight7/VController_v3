// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: Device_class declaration (base)
// Section 2: GP10_class declaration (derived)
// Section 3: GR55_class declaration (derived)
// Section 4: VG99_class declaration (derived)
// Section 5: ZG3_class declaration (derived)
// Section 6: ZMS70_class declaration (derived)

// Here we declare the class objects for the devices the VController supports. The actual class code is found in the MIDI_????.ino files.

// ********************************* Section 1: Device_class declaration (base) ********************************************

#define NUMBER_OF_DEVICE_SETTINGS 9

class Device_class
{ // Base class for devices

  public:
    // Procedures:
    Device_class (uint8_t _dev_no); // Constructor

    virtual void init();
    uint8_t get_setting(uint8_t variable);
    void set_setting(uint8_t variable, uint8_t value);

    virtual void check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    virtual void check_PC_in(uint8_t program, uint8_t channel, uint8_t port);
    void check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port);
    void check_still_connected();
    virtual void identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port);
    void connect(uint8_t device_id, uint8_t port);
    virtual void do_after_connect();
    void disconnect();
    void check_sysex_delay();
    uint8_t calc_Roland_checksum(uint16_t sum);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    virtual void SendProgramChange(uint16_t new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t calculate_patch_address(uint16_t number);
    virtual void request_patch_name(uint16_t number);
    virtual void request_current_patch_name();
    uint16_t calculate_patch_number(uint8_t bank_position, uint8_t bank_size);
    void patch_select(uint16_t new_patch);
    uint16_t prev_patch_number();
    uint16_t next_patch_number();
    void bank_updown(bool updown, uint8_t my_bank_size);
    bool bank_selection_active();
    virtual void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);
    virtual void direct_select_press(uint8_t number);

    virtual void unmute();
    virtual void mute();
    void select_switch();
    void is_always_on_toggle();

    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_state(uint16_t number, uint8_t value, String &Output);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint16_t number);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);
    void par_bank_updown(bool updown, uint8_t my_bank_size);

    virtual void read_assign_name(uint8_t number, String &Output);
    virtual void read_assign_trigger(uint8_t number, String &Output);
    virtual uint8_t get_number_of_assigns();
    virtual uint8_t trigger_follow_assign(uint8_t number);
    virtual void assign_press(uint8_t Sw, uint8_t value);
    virtual void assign_release(uint8_t Sw);
    virtual void assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number);
    virtual void request_current_assign();
    //virtual uint8_t number_of_assigns();

    virtual void FX_press(uint8_t Sw, Cmd_struct *cmd, uint8_t number);
    virtual void FX_set_type_and_state(uint8_t Sw);

    // Variables:
    uint8_t my_device_number;
    char device_name[8];
    char full_device_name[17];
    uint8_t my_LED_colour;  // Variable no.1
    uint8_t MIDI_channel;  // Variable no.2
    uint8_t MIDI_port = 0; // Variable no.4
    uint8_t MIDI_device_id; // Variable no.3
    uint16_t patch_number = 0;
    uint32_t PC_ignore_timer;
    String current_patch_name;
    uint16_t patch_min;
    uint16_t patch_max;
    uint16_t bank_number = 1; // The bank number of the current_patch
    uint16_t bank_select_number = 1; // The bank number used for bank up/down
    uint8_t bank_size = 10;
    uint8_t CC01 = 0;
    uint8_t direct_select_state = 0;
    uint8_t direct_select_first_digit;
    bool connected = false;
    uint8_t no_response_counter = 0;
    uint8_t max_times_no_response;
    bool is_on = false;
    bool is_always_on = true;
    bool request_onoff = false;
    unsigned long SYSEX_DELAY_LENGTH;// time between sysex messages (in msec).
    unsigned long sysexDelay = 0;
    uint32_t last_requested_sysex_address;
    uint8_t last_requested_sysex_type;
    uint16_t parameter_bank_number = 0;

    // Default pages
    uint8_t my_patch_page = 0;  // Variable no.4
    uint8_t my_parameter_page = 0;  // Variable no.5
    uint8_t my_assign_page = 0;  // Variable no.6
};

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
    virtual void request_patch_name(uint16_t number);
    virtual void request_current_patch_name();
    //void page_check();
    virtual void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);

    void request_guitar_switch_states();
    void check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void unmute();
    virtual void mute();

    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_state(uint16_t number, uint8_t value, String &Output);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint16_t number);
    void read_parameter(uint8_t byte1, uint8_t byte2);
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
    virtual void request_current_assign();
    void request_complete_assign_area();
    void read_complete_assign_area(uint32_t address, const unsigned char* sxdata, short unsigned int sxlength);
    void assign_request();
    bool target_lookup(uint16_t target);

    // Variables:
    uint8_t COSM_onoff;
    uint8_t nrml_pu_onoff;
    uint8_t assign_read = false; // Assigns are read in three steps on the GP10 1) Read first 64 bytes, 2) Read last 64 bytes, 3) Read targets
    uint8_t assign_mem[0x80]; // Memory space for the data from the assigns
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

    virtual void SendProgramChange(uint16_t new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t calculate_patch_address(uint16_t number);
    virtual void request_patch_name(uint16_t number);
    virtual void request_current_patch_name();
    //void page_check();
    virtual void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);
    virtual void direct_select_press(uint8_t number);

    void request_guitar_switch_states();
    void check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void unmute();
    virtual void mute();
    void mute_now();
    void read_preset_name(uint8_t number, uint16_t patch);

    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_state(uint16_t number, uint8_t value, String &Output);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint16_t number);
    void read_parameter(uint8_t byte1, uint8_t byte2);
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
    virtual void request_current_assign();
    void read_current_assign(uint32_t address, const unsigned char* sxdata, short unsigned int sxlength);
    void assign_request();
    bool target_lookup(uint16_t target);

    // Variables:
    uint8_t preset_banks; // Default number of preset banks
    uint8_t synth1_onoff;
    uint8_t synth2_onoff;
    uint8_t COSM_onoff;
    uint8_t nrml_pu_onoff;
    bool was_on = false;
    bool bass_mode = false;
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

    virtual void SendProgramChange(uint16_t new_patch);
    virtual void do_after_patch_selection();
    //virtual uint32_t calculate_patch_address(uint16_t number);
    virtual void request_patch_name(uint16_t number);
    virtual void request_current_patch_name();
    //void page_check();
    virtual void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);

    void request_guitar_switch_states();
    void check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength);
    virtual void unmute();
    virtual void mute();
    void mute_now();

    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual void read_parameter_state(uint16_t number, uint8_t value, String &Output);
    //virtual uint8_t read_parameter_numvals(uint16_t number);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual bool request_parameter(uint16_t number);
    void read_parameter(uint8_t byte1, uint8_t byte2);
    void check_update_label(uint8_t Sw, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);

    virtual void read_assign_name(uint8_t number, String &Output);
    virtual void read_assign_trigger(uint8_t number, String &Output);
    virtual uint8_t get_number_of_assigns();
    virtual uint8_t trigger_follow_assign(uint8_t number);
    virtual void assign_press(uint8_t Sw, uint8_t value);
    virtual void assign_release(uint8_t Sw);
    void fix_reverse_pedals();
    virtual void assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number);
    virtual void request_current_assign();
    void read_current_assign(uint32_t address, const unsigned char* sxdata, short unsigned int sxlength);
    void assign_request();
    bool target_lookup(uint16_t target);
    //virtual uint8_t number_of_assigns();

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
    virtual void request_patch_name(uint16_t number);
    virtual void set_bpm();
    virtual void start_tuner();
    virtual void stop_tuner();

    void do_after_patch_selection();
    virtual void number_format(uint16_t number, String &Output);
    virtual void direct_select_format(uint16_t number, String &Output);
    void display_patch_number_string();
    virtual void direct_select_press(uint8_t number);
    void ZG3_Recall_FXs(uint8_t Sw);

    //virtual void FX_press(uint8_t Sw, Cmd_struct *cmd, uint8_t number);
    //virtual void FX_set_type_and_state(uint8_t Sw);
    virtual bool request_parameter(uint16_t number);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);
    virtual void read_parameter_state(uint16_t number, uint8_t value, String &Output);

    // Variables:
    const uint8_t NUMBER_OF_FX_SLOTS = 6; // The Zoom MS70-CDR has 6 FX slots
    uint16_t FX[6]; //Memory location for the 6 FX
#define ZG3_CURRENT_PATCH_DATA_SIZE 110
    uint8_t CP_MEM[ZG3_CURRENT_PATCH_DATA_SIZE]; // Memory to store the current patch data
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
    virtual void request_patch_name(uint16_t number);
    virtual void display_patch_number_string();
    virtual void number_format(uint16_t number, String &Output);

    //void FX_press(uint8_t Sw, Cmd_struct *cmd, uint8_t number);
    uint8_t FXsearch(uint8_t type, uint8_t number);
    //void FX_set_type_and_state(uint8_t Sw);

    virtual bool request_parameter(uint16_t number);
    virtual void parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number);
    virtual void read_parameter_name(uint16_t number, String &Output);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);
    virtual void read_parameter_state(uint16_t number, uint8_t value, String &Output);

    // Variables
    const uint8_t NUMBER_OF_FX_SLOTS = 6; // The Zoom MS70-CDR has 6 FX slots
    uint16_t FX[6]; //Memory location for the 6 FX
#define ZMS70_CURRENT_PATCH_DATA_SIZE 146
    uint8_t CP_MEM[ZMS70_CURRENT_PATCH_DATA_SIZE]; // Memory to store the current patch data
};

