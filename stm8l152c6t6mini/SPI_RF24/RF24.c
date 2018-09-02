

GPIO_TypeDef * gpio_csn = GPIOB;
u8 pin_csn = GPIO_Pin_2;

GPIO_TypeDef * gpio_ce = GPIOB;
u8 pin_ce = GPIO_Pin_1;

SPI_TypeDef * spi = SPI1;

u8 payload_size = 32;
bool dynamic_payloads_enabled = FALSE;
bool p_variant = FALSE;
u8 addr_width = 5;

u8 pipe0_reading_address[5] = {0}; /**< Last address set on pipe 0 for reading. */

u32 txDelay = 65;
// private

void csn(bool mode) {
  GPIO_WriteBit(gpio_csn, pin_csn, mode);
}

void ce(bool val) {
	GPIO_WriteBit(pgio_ce, pin_ce, val);
}


inline void beginTransaction() {
  csn(LOW);
}

inline void endTransaction() {
  csn(HIGH);
}

u8 spi_transfer(u8 dt) {
  u8 status;

  while (!SPI_GetFlagStatus(spi, SPI_FLAG_TXE));
  SPI_SendData(spi, dt);

  while (!SPI_GetFlagStatus(spi, SPI_FLAG_RXNE));
  status = SPI_ReceiveData(spi);

  return status;
}

void spi_transfer_n(u8 *out, u8 length, u8 *in) {
	while (length--) {
		u8 val = spi_transfer(*out++);
		if (in)
			*in++ = val;
	}
}

u8 read_register_n(u8 reg, u8 * data, u8 length) {
  u8 status;
	beginTransaction();
	status = spi_transfer(R_REGISTER | ( REGISTER_MASK & reg ));
	while (length--) {
		*data++ = spi_transfer(0xff);
	}
	endTransaction();
  return status;
}

u8 read_register(u8 reg) {
	u8 data;
	read_register_n(reg, &data, 1);
	return data;
}


u8 write_register_n(u8 reg, u8 * data, u8 length) {
  u8 status;
  beginTransaction();
	status = spi_transfer(W_REGISTER | ( REGISTER_MASK & reg ));
	spi_transfer_n(data, length, NULL);
  endTransaction();
  return status;
}

u8 write_register(u8 reg, u8 val) {
  return write_register_n(reg, &val, 1);
}

u8 write_payload(const void* buf, u8 data_len, const u8 writeType) {
  u8 status, blank_len;
  const u8* current = reinterpret_cast<const u8*>(buf);

  data_len = rf24_min(data_len, payload_size);
  blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;

  // IF_SERIAL_DEBUG( printf("[Writing %u bytes %u blanks]\n",data_len,blank_len); );

  beginTransaction();
  status = spi_transfer(writeType);

  spi_transfer_n(current, data_len);
  while (blank_len--) {
    spi_transfer(0);
  }
  endTransaction();

  return status;
}

u8 read_payload(void* buf, u8 data_len) {
  u8 status, blank_len;
  u8* current = reinterpret_cast<u8*>(buf);

  data_len = rf24_min(data_len, payload_size);

  blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;

  // IF_SERIAL_DEBUG( printf("[Reading %u bytes %u blanks]\n",data_len,blank_len); );

  beginTransaction();
  status = spi_transfer(R_RX_PAYLOAD);
  while (data_len--) {
    *current++ = spi_transfer(0xFF);
  }
  while (blank_len--) {
    spi_transfer(0xff);
  }
  endTransaction();
  return status;
}

u8 spiTrans(u8 cmd) {
  u8 status;
  beginTransaction();
  status = spi_transfer(cmd);
  endTransaction();
  return status;
}

u8 flush_rx(void) {
  return spiTrans(FLUSH_RX);
}

u8 flush_tx(void) {
  return spiTrans(FLUSH_TX);
}

u8 get_status(void) {
  return spiTrans(RF24_NOP);
}

void print_status(u8 status) {
  printf("STATUS\t\t = 0x%02x RX_DR=%x TX_DS=%x MAX_RT=%x RX_P_NO=%x TX_FULL=%x\r\n",
    status,
    (status & _BV(RX_DR))?1:0,
    (status & _BV(TX_DS))?1:0,
    (status & _BV(MAX_RT))?1:0,
    ((status >> RX_P_NO) & 0x07),
    (status & _BV(TX_FULL))?1:0
  );
}

void print_observe_tx(u8 value) {
  printf("OBSERVE_TX=%02x: POLS_CNT=%x ARC_CNT=%x\r\n",
    value,
    (value >> PLOS_CNT) & 0x0F,
    (value >> ARC_CNT) & 0x0F
  );
}

void print_byte_register(const char* name, u8 reg, u8 qty) {
  printf("%s\t =", name);

  while (qty--)
    printf(" 0x%02x", read_register(reg++));
  printf("\r\n");
}

void print_address_register(const char* name, u8 reg, u8 qty) {
  u8 buffer[addr_width];
  printf("%s\t =", name);

  while (qty--) {
    read_register_n(reg++, buffer, sizeof buffer);

    printf(" 0x");
    u8* bufptr = buffer + sizeof buffer;
    while (--bufptr >= buffer) {
      printf("%02x", *bufptr);
    }
  }

  printf("\r\n");
}

void setChannel(u8 channel) {
  const u8 max_channel = 125;
  write_register(RF_CH, rf24_min(channel,max_channel));
}

u8 getChannel() {
  return read_register(RF_CH);
}

void setPayloadSize(u8 size) {
  payload_size = rf24_min(size, 32);
}

u8 getPayloadSize(void) {
  return payload_size;
}


static const char rf24_datarate_e_str_0[] = "1MBPS";
static const char rf24_datarate_e_str_1[] = "2MBPS";
static const char rf24_datarate_e_str_2[] = "250KBPS";
static const char * const rf24_datarate_e_str_P[] = {
  rf24_datarate_e_str_0,
  rf24_datarate_e_str_1,
  rf24_datarate_e_str_2,
};
static const char rf24_model_e_str_0[] = "nRF24L01";
static const char rf24_model_e_str_1[] = "nRF24L01+";
static const char * const rf24_model_e_str_P[] = {
  rf24_model_e_str_0,
  rf24_model_e_str_1,
};
static const char rf24_crclength_e_str_0[] = "Disabled";
static const char rf24_crclength_e_str_1[] = "8 bits";
static const char rf24_crclength_e_str_2[] = "16 bits" ;
static const char * const rf24_crclength_e_str_P[] = {
  rf24_crclength_e_str_0,
  rf24_crclength_e_str_1,
  rf24_crclength_e_str_2,
};
static const char rf24_pa_dbm_e_str_0[] = "PA_MIN";
static const char rf24_pa_dbm_e_str_1[] = "PA_LOW";
static const char rf24_pa_dbm_e_str_2[] = "PA_HIGH";
static const char rf24_pa_dbm_e_str_3[] = "PA_MAX";
static const char * const rf24_pa_dbm_e_str_P[] = {
  rf24_pa_dbm_e_str_0,
  rf24_pa_dbm_e_str_1,
  rf24_pa_dbm_e_str_2,
  rf24_pa_dbm_e_str_3,
};

void printDetails(void) {
  print_status(get_status());
  print_address_register(PSTR("RX_ADDR_P0-1"), RX_ADDR_P0, 2);
  print_byte_register(PSTR("RX_ADDR_P2-5"), RX_ADDR_P2, 4);
  print_address_register(PSTR("TX_ADDR\t"), TX_ADDR);

  print_byte_register(PSTR("RX_PW_P0-6"), RX_PW_P0, 6);
  print_byte_register(PSTR("EN_AA\t"), EN_AA);
  print_byte_register(PSTR("EN_RXADDR"), EN_RXADDR);
  print_byte_register(PSTR("RF_CH\t"), RF_CH);
  print_byte_register(PSTR("RF_SETUP"), RF_SETUP);
  print_byte_register(PSTR("CONFIG\t"), NRF_CONFIG);
  print_byte_register(PSTR("DYNPD/FEATURE"), DYNPD, 2);

  printf("Data Rate\t = %s\r\n", rf24_datarate_e_str_P[getDataRate()]);
  printf("Model\t\t = %s\r\n", rf24_model_e_str_P[isPVariant()]);
  printf("CRC Length\t = %s\r\n", rf24_crclength_e_str_P[getCRCLength()]);
  printf("PA Power\t = %s\r\n", rf24_pa_dbm_e_str_P[getPALevel()]);
}

void begin() {
  u8 setup = 0;

  delay(5) ;

  // Reset NRF_CONFIG and enable 16-bit CRC.
  write_register(NRF_CONFIG, 0x0C) ;

  // Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
  // WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
  // sizes must never be used. See documentation for a more complete explanation.
  setRetries(5, 15);

  // Reset value is MAX
  //setPALevel( RF24_PA_MAX ) ;

  // check for connected module and if this is a p nRF24l01 variant
  //
  if(setDataRate(RF24_250KBPS)) {
    p_variant = true ;
  }

  setup = read_register(RF_SETUP);
  /*if( setup == 0b00001110 )     // register default for nRF24L01P
  {
    p_variant = true ;
  }*/

  // Then set the data rate to the slowest (and most reliable) speed supported by all
  // hardware.
  setDataRate(RF24_1MBPS);

  // Initialize CRC and request 2-byte (16bit) CRC
  //setCRCLength( RF24_CRC_16 ) ;

  // Disable dynamic payloads, to match dynamic_payloads_enabled setting - Reset value is 0
  toggle_features();
  write_register(FEATURE, 0);
  write_register(DYNPD, 0);
  dynamic_payloads_enabled = false;

  // Reset current status
  // Notice reset and flush is the last thing we do
  write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );

  // Set up default configuration.  Callers can always change it later.
  // This channel should be universally safe and not bleed over into adjacent
  // spectrum.
  setChannel(76);

  // Flush buffers
  flush_rx();
  flush_tx();

  powerUp(); //Power up by default when begin() is called

  // Enable PTX, do not write CE high so radio will remain in standby I mode ( 130us max to transition to RX or TX instead of 1500us from powerUp )
  // PTX should use only 22uA of power
  write_register(NRF_CONFIG, ( read_register(NRF_CONFIG) ) & ~_BV(PRIM_RX) );

  // if setup is 0 or ff then there was no response from module
  return setup != 0 && setup != 0xff;
}


bool isChipConnected() {
  u8 setup = read_register(SETUP_AW);
  return setup >= 1 && setup <= 3;
}

void startListening(void) {
  powerUp();

  write_register(NRF_CONFIG, read_register(NRF_CONFIG) | _BV(PRIM_RX));
  write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

  ce(HIGH);

  // Restore the pipe0 adddress, if exists
  if (pipe0_reading_address[0] > 0){
    write_register(RX_ADDR_P0, pipe0_reading_address, addr_width);
  } else {
	   closeReadingPipe(0);
  }

  // Flush buffers
  //flush_rx();
  if (read_register(FEATURE) & _BV(EN_ACK_PAY)){
    flush_tx();
  }
  // Go!
  //delayMicroseconds(100);
}

static const u8 child_pipe_enable[] = {
  ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5
};

void stopListening(void) {
  ce(LOW);

  delayMicroseconds(txDelay);

  if (read_register(FEATURE) & _BV(EN_ACK_PAY)){
    delayMicroseconds(txDelay); //200
	  flush_tx();
  }
  //flush_rx();

  write_register(NRF_CONFIG, ( read_register(NRF_CONFIG) ) & ~_BV(PRIM_RX) );

  write_register(EN_RXADDR, read_register(EN_RXADDR) | _BV(child_pipe_enable[0])); // Enable RX on pipe0

  //delayMicroseconds(100);
}

void powerDown(void) {
  ce(LOW); // Guarantee CE is low on powerDown
  write_register(NRF_CONFIG, read_register(NRF_CONFIG) & ~_BV(PWR_UP));
}

void powerUp(void) {
   u8 cfg = read_register(NRF_CONFIG);

   if (!(cfg & _BV(PWR_UP))) {
      write_register(NRF_CONFIG, cfg | _BV(PWR_UP));
      delay(2);
   }
}

void errNotify(){
	printf("RF24 HARDWARE FAIL: Radio not responding, verify pin connections, wiring, etc.\r\n");
	// #if defined (FAILURE_HANDLING)
	// failureDetected = 1;
	// #else
	// delay(5000);
	// #endif
}

bool write_n(const void* buf, u8 len, const bool multicast) {
	//Start Writing
	startFastWrite(buf, len, multicast);

	//Wait until complete or failed
	u32 timer = millis();

	while(!( get_status() & ( _BV(TX_DS) | _BV(MAX_RT) ))) {
		if (millis() - timer > 95){
			errNotify();
			return 0;
		}
	}

	ce(LOW);

	u8 status = write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );

  //Max retries exceeded
  if (status & _BV(MAX_RT)) {
  	flush_tx(); //Only going to be 1 packet int the FIFO at a time using this method, so just flush
  	return 0;
  }
	//TX OK 1 or 0
  return 1;
}

bool write(const void* buf, u8 len) {
	return write(buf, len, 0);
}

//For general use, the interrupt flags are not important to clear
bool writeBlocking(const void* buf, u8 len, u32 timeout) {
	//Block until the FIFO is NOT full.
	//Keep track of the MAX retries and set auto-retry if seeing failures
	//This way the FIFO will fill up and allow blocking until packets go through
	//The radio will auto-clear everything in the FIFO as long as CE remains high
	u32 timer = millis();							  //Get the time that the payload transmission started

	while (get_status() & _BV(TX_FULL)) {		  //Blocking only if FIFO is full. This will loop and block until TX is successful or timeout
		if (get_status() & _BV(MAX_RT)) {					  //If MAX Retries have been reached
			reUseTX();										  //Set re-transmit and clear the MAX_RT interrupt flag
			if (millis() - timer > timeout) {
        return 0;
      }		  //If this payload has exceeded the user-defined timeout, exit and return 0
		}
	}
	//Start Writing
	startFastWrite(buf, len, 0);								  //Write the payload if a buffer is clear
	return 1;												  //Return 1 to indicate successful transmission
}

void reUseTX(){
	write_register(NRF_STATUS, _BV(MAX_RT));			  //Clear max retry flag
	spiTrans(REUSE_TX_PL);
	ce(LOW);										  //Re-Transfer packet
	ce(HIGH);
}

bool writeFast(const void* buf, u8 len, const bool multicast) {
	//Block until the FIFO is NOT full.
	//Keep track of the MAX retries and set auto-retry if seeing failures
	//Return 0 so the user can control the retrys and set a timer or failure counter if required
	//The radio will auto-clear everything in the FIFO as long as CE remains high
	u32 timer = millis();

	while (get_status() & _BV(TX_FULL)) {			  //Blocking only if FIFO is full. This will loop and block until TX is successful or fail
		if (get_status() & _BV(MAX_RT)) {
			//reUseTX();										  //Set re-transmit
			write_register(NRF_STATUS, _BV(MAX_RT));			  //Clear max retry flag
			return 0;										  //Return 0. The previous payload has been retransmitted
															  //From the user perspective, if you get a 0, just keep trying to send the same payload
		}

		if(millis() - timer > 95 ){
			// errNotify();
			return 0;
		}
  }
		     //Start Writing
	startFastWrite(buf, len, multicast);
	return 1;
}

bool writeFast(const void* buf, u8 len ){
	return writeFast(buf, len, 0);
}

//Per the documentation, we want to set PTX Mode when not listening. Then all we do is write data and set CE high
//In this mode, if we can keep the FIFO buffers loaded, packets will transmit immediately (no 130us delay)
//Otherwise we enter Standby-II mode, which is still faster than standby mode
//Also, we remove the need to keep writing the config register over and over and delaying for 150 us each time if sending a stream of data
void startFastWrite(const void* buf, u8 len, const bool multicast, bool startTx) { //TMRh20
	//write_payload( buf,len);
	write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);
	if (startTx) {
		ce(HIGH);
	}
}

//Added the original startWrite back in so users can still use interrupts, ack payloads, etc
//Allows the library to pass all tests
void startWrite(const void* buf, u8 len, const bool multicast) {
  // Send the payload
  //write_payload( buf, len );
  write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD ) ;
  ce(HIGH);
  ce(LOW);
}
