
// Functions in file INIT.C
void Init(void);

// Functions in file CYCLE.C
void seal_cycle (void);
void seal_assembly_open(void);
void seal_assembly_close(void);
void start_drive_motor(void);
void start_blower_motor(void);
void calculate_air_flow(void);

// Functions in file INPUTS.C
void key_control(void);
void measure_inputs (void);
void error_detection (void);
void check_over_temp (void);

// Functions in file PID_control.C
void PID_controler(void);
void calc_PID_heater(void);
void switch_heater(void);


// Functions in file IO_TEST.C
void IO_test(void);
void open_because_of_error(void);

// Functions in file MENU.C
void menu_handling(void);
void flash_leds(void);
//void save_tagprog_if_changed(void);
//void clear_EE_tagprog(void);
void word2bytes(uint16_t temp1);
void bytes2word(uint8_t high_byte, uint8_t low_byte);
void CheckValidSettings();

// Functions in file display.C
void update_display(void);
void show_3digit_number(uint8_t temp1);
void show_2digit_number(uint8_t temp1);
void show_offset(uint8_t sign, uint8_t temp1);
void show_init_text(void);
void show_ext_3digit_number(uint16_t temp1);
void scroll_display(uint32_t temp3);

// Functions in file EEPROM.C
uint8_t read_EE_byte(uint8_t address);
uint16_t read_EE_int(uint8_t address);
void write_EE_byte(uint8_t address, uint8_t data);
void write_EE_int(uint8_t address, uint16_t data);
void read_metercounter(void);
void clear_metercounters(void);
void save_metercounter(void);

// Functions in file SPI.C
void SPI_delay(void);
uint8_t SPI_read(uint8_t SPI_data_out);
uint8_t SPI_ascii_to_dec(uint8_t idx);
void SPI_dec_to_ascii(uint8_t idx, uint8_t blockdata);
void read_block_and_status(uint8_t block_no);
void fetch_block(uint8_t block_no);
void write_block(uint8_t block_no);
void lock_block(uint8_t block_no);


// Functions in file RFID.C
void RFID_detection (void);


// Functions in file TAG_prog.C
//void program_TAG_info(void);


// Functions in file LMP90100.C
void init_LMP90100_registers(void);
void write_to_register(uint8_t reg_adr, uint8_t data_cnt, uint8_t reg_data);
void read_from_register(uint8_t reg_adr, uint8_t data_cnt, uint8_t *LMP_data_buf);
void measure_temperature (void);

