/*
 * Author: Abir Mojumder 
*/

#include "SPI.h"
#include <TimeLib.h>
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "inttypes.h"

//channel hopping interval in milliseconds
#define INTERVAL 250

//Initialize phase (setup PCAP file headers and ESP32 Wifi configuration)
//PCAP file required headers and values (information in https://wiki.wireshark.org/Development/LibpcapFileFormat)

uint32_t magic_num = 0xa1b2c3d4; //The default magic number. With this magic number, every packet needs timestamp in seconds and Microseconds.
uint16_t version_major = 2; /* major version number */
uint16_t version_minor = 4; /* minor version number */
uint32_t thiszone = 0; /* GMT to local correction */
uint32_t sigfigs = 0; /* accuracy of timestamps */
uint32_t snaplen = 65535; /* max length of captured packets, in octets */
uint32_t network = 105; /* data link type */

void setupPCAP(){
  serialout_32bit(magic_num);
  serialout_16bit(version_major);
  serialout_16bit(version_minor);
  serialout_32bit(thiszone);
  serialout_32bit(sigfigs);
  serialout_32bit(snaplen);
  serialout_32bit(network);
}

//set scanning channel (main loop changes channel in given interval)
int channel = 1;
int max_channel = 11;
unsigned long lastHopTime = 0;
boolean activate_hopping = true;

//The MAC header structure we will use. It is in the same format as in IEEE. Some data types are in bits others are in bytes 
//so that we can take any piece of the 6 bytes and display in HEX (mainly the addresses).
//Wireshark implementation does not need this seperately, this is only for testing out the serial monitor (shown in previous project reports).
//typedef struct {
//  unsigned frame_ctrl:16;
//  unsigned duration_id:16;
//  uint8_t addr1[6]; /* receiver mac address */
//  uint8_t addr2[6]; /* original sender mac address */
//  uint8_t addr3[6]; /* BSSID */
//  unsigned sequence_ctrl:16;
//} wifi_packet_mac_hdr_t;

//Data Frame/Packet struct to split captured packet into mac header and payload.
//typedef struct {
//  wifi_packet_mac_hdr_t hdr;
//  uint8_t payload[0];
//} wifi_captured_packet_t;

void setup() {
  Serial.begin(921600); //rate that wireshark program reads
  delay(2000);
  Serial.println();
  Serial.println("<<START>>"); // Wireshark program needs this line to indicate that ESP32 is capturing packets.
  setupPCAP(); //sends PCAP header information once to the wireshark program

//store serial buffer before flushing
  nvs_flash_init();
/**
 * Error checking using ESP_ERROR_CHECK macro.
 * Return type is esp_err_t
 */

  //stack configuration initialization, pass cfg to esp_wifi_init function.
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));  
  ESP_ERROR_CHECK(esp_wifi_start());
  
  //setting promiscuous mode here
  esp_wifi_set_promiscuous(true);
  //CSI function
  esp_wifi_set_promiscuous_rx_cb(sniff_out);

  //Set primary and secondary channel(not used)
  wifi_second_chan_t secondchannel = (wifi_second_chan_t)NULL;
  esp_wifi_set_channel(channel,secondchannel);
}


//Main Functions that run continuously:


//Callback method to capture promiscuous packets
void sniff_out(void* buf, wifi_promiscuous_pkt_type_t type){

  //received packet
  wifi_promiscuous_pkt_t* pak = (wifi_promiscuous_pkt_t*) buf;

  //received control header (extract from received packet):
  wifi_pkt_rx_ctrl_t header = (wifi_pkt_rx_ctrl_t)pak->rx_ctrl;

  //time of receiving packet
  uint32_t time_sec = now(); //current timestamp 
  uint32_t time_usec = (unsigned int)(micros() - millis() * 1000);

  /* Two structs below were for testing the parsing only*/
  //const wifi_captured_packet_t *pkt_frame = (wifi_captured_packet_t *)pak->payload;
  //const wifi_packet_mac_hdr_t *mac_hdr = &pkt_frame->hdr;

  serialPacket(time_sec, time_usec, header.sig_len, pak->payload);
  

}

//Parser method to send PCAP formated packets to serial port. (PCAP needs timestamps in sec, microsec, payload length and payload).
void serialPacket(uint32_t time_sec, uint32_t time_usec, uint32_t len, uint8_t* payload_buf){

  uint32_t orig_len = len; // number of octets of packet saved in file
  uint32_t incl_len = len; // actual length of packet

  //if received packet is greater than snaplen, limit it, otherwise malforms the wireshark output.
  if(incl_len > snaplen){
      incl_len = snaplen;
  }

  //send data to serial port in the order time in seconds > time in usec > incl_len > orig_len > payload
  serialout_32bit(time_sec);
  serialout_32bit(time_usec);
  serialout_32bit(incl_len);
  serialout_32bit(orig_len);
  Serial.write(payload_buf, incl_len);
}

//convert 32 bit input to 4 bytes output to serial port
//Need this method because Wireshark serial program only reads 1 byte each time and writes to a capture file.
void serialout_32bit(uint32_t input){
  uint8_t val[4];

  //shift bits to the right to get every chunk of bytes
  val[0] = input;
  val[1] = input >> 8;
  val[2] = input >> 16;
  val[3] = input >> 24;
  Serial.write(val, 4);
}

//convert 16 bit input to 2 bytes output to serial port
void serialout_16bit(uint16_t input){
  uint8_t val[2];

  //shift bits to the right to get every chunk of bytes
  val[0] = input;
  val[1] = input >> 8;
  Serial.write(val, 2);
}


//This function runs if channel hopping is enabled during initialization
void loop() {

  if(activate_hopping){
      unsigned long currentTime = millis();
      if(currentTime - lastHopTime >= INTERVAL){ //if last channel changed greater than interval, change the channel
      lastHopTime = currentTime;
      channel++; //go to next channel if possible
      if(channel > max_channel) {
        channel = 1;
      }
      wifi_second_chan_t secondchannel = (wifi_second_chan_t)NULL;
      esp_wifi_set_channel(channel,secondchannel);
    }
  }

}
