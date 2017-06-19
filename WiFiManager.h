/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/esp8266/hardware/esp8266com/esp8266/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Licensed under MIT license
 **************************************************************/

#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <memory>

extern "C" {
  #include "user_interface.h"
}

#define PAGE_WIFI_CONFIG						(0)
#define PAGE_PERIOD_CONFIG						(1)
#define PAGE_SERVER_CONFIG						(2)

#define STT_NO_WIFI								(0)
#define STT_WIFI_BUT_NO_SERVER					(1)
#define STT_WIFI_AND_SERVER						(2)

const char HTTP_HEAD[] PROGMEM					= "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no, charset=UTF-8\"/><title>{v}</title>";
const char HTTP_HEAD_WITH_REFRESH[] PROGMEM		= "<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"refresh\"content=\"20;url=/status\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no, charset=UTF-8\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM					= "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_SCRIPT[] PROGMEM				= "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM				= "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM		= "<form action=\"/cwifi\" method=\"get\"><button>Thiết lập WiFi</button></form><br/><form action=\"/cperiod\" method=\"get\"><button>Thiết lập thông số đo lường, thời gian</button></form><br/><form action=\"/cserver\" method=\"get\"><button>Thiết lập thông số server</button></form><br/><form action=\"/i\" method=\"get\"><button>Thông tin thiết bị</button></form>";
const char HTTP_ITEM[] PROGMEM					= "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_C_WIFI_START[] PROGMEM     = "<strong>Thiết lập wifi</strong><br/><form method='get' action='csave'><fieldset><legend>Thông số wifi:</legend>Tên wifi (SSID): <input id='s' name='s' length=32 placeholder='thachbaonguyen'><br/>Mật khẩu: <input id='p' name='p' length=64 type='password' placeholder='1234'><br/></fieldset><br/>";
const char HTTP_FORM_C_PERIOD_START[] PROGMEM	= "<strong>Thiết lập thông số đo lường, thời gian</strong><br/><form method='get' action='csave'><fieldset><legend>Thông số đo lường, thời gian:</legend>Chu kỳ đọc cảm biến:<select id='pr' name='pr'><option value=\"5\">5</option><option value=\"15\">15</option><option value=\"30\">30</option><option value=\"60\">60</option></select> giây<br/>Chu kỳ gửi dữ liệu:<select id='pu' name='pu'><option value=\"5\">5</option><option value=\"10\">10</option><option value=\"20\">20</option><option value=\"30\">30</option></select> phút<br/></fieldset><br/>";
const char HTTP_FORM_C_SERVER_START[] PROGMEM	= "<strong>Thiết lập thông số server</strong><br/><form method='get' action='csave'><fieldset><legend>Thông số server:</legend>API Thingspeak: <input id='api' name='api' length=32 placeholder='HIXV5H5LOMVRE1I0'><br/>Cổng: <input id='port' name='port' length=8 placeholder='80'><br/></fieldset><br/>";
const char HTTP_FORM_PARAM[] PROGMEM			= "<br/><input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_FORM_END[] PROGMEM				= "<br/><button type='submit'>Lưu thiết lập</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM				= "<br/><div class=\"c\"><a href=\"/cwifi\">Quét wifi</a></div>";
const char HTTP_SAVED[] PROGMEM					= "<div>Đã lưu thiết lập.<br />Đang thử kết nối mạng bằng thông số vừa thiết lập....<br/></div>";
const char HTTP_RESET_BUTTON[] PROGMEM			= "<form action=\"/r\" method=\"post\"><button>Khởi động lại</button></form>";
const char HTTP_HOME_BUTTON[] PROGMEM			= "<form action=\"/\" method=\"post\"><button>Trở về</button></form>";
const char HTTP_END[] PROGMEM					= "</div></body></html>";
const char HTTP_STT_0[] PROGMEM					= "<div>Không thể kết nối được wifi!<br />Vui lòng ấn nút \"Trở lại\" để kiểm tra thông số.</div>";
const char HTTP_STT_1[] PROGMEM					= "<div>Kết nối wifi thành công!<br />Không thể kết nối tới server.<br />Vui lòng ấn nút \"Trở lại\" để kiểm tra thông số.</div>";
const char HTTP_STT_2[] PROGMEM					= "<div>Kết nối wifi và server thành công!<br /></div>";

#define WIFI_MANAGER_MAX_PARAMS 10

class WiFiManagerParameter {
  public:
    WiFiManagerParameter(const char *custom);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);
	void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    const char *getID();
    const char *getValue();
    const char *getPlaceholder();
    int         getValueLength();
    const char *getCustomHTML();
  private:
    const char *_id;
    const char *_placeholder;
    char       *_value;
    int         _length;
    const char *_customHTML;

    

    friend class WiFiManager;
};


class WiFiManager
{
  public:
    WiFiManager();

    boolean       autoConnect();
    boolean       autoConnect(char const *apName, char const *apPassword = NULL);

    //if you want to always start the config portal, without trying to connect first
    boolean       startConfigPortal();
    boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

    // get the AP name of the config portal, so it can be used in the callback
    String        getConfigPortalSSID();

    void          resetSettings();

    //sets timeout before webserver loop ends and exits even if there has been no setup.
    //usefully for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void          setConfigPortalTimeout(unsigned long seconds);
    void          setTimeout(unsigned long seconds);

    //sets timeout for which to attempt connecting, usefull if you get a lot of failed connects
    void          setConnectTimeout(unsigned long seconds);


    void          setDebugOutput(boolean debug);
    //defaults to not showing anything under 8% signal quality if called
    void          setMinimumSignalQuality(int quality = 8);
    //sets a custom ip /gateway /subnet configuration
    void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //sets config for a static IP
    void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //called when AP mode and config portal is started
    void          setAPCallback( void (*func)(WiFiManager*) );
    //called when settings have been changed and connection was successful
    void          setSaveConfigCallback( void (*func)(void) );
    //adds a custom parameter
    void          addParameter(WiFiManagerParameter *p);
    //if this is set, it will exit after config, even if connection is unsucessful.
    void          setBreakAfterConfig(boolean shouldBreak);
    //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
    //TODO
    //if this is set, customise style
    void          setCustomHeadElement(const char* element);
    //if this is true, remove duplicated Access Points - defaut true
    void          setRemoveDuplicateAPs(boolean removeDuplicates);
	int           connectWifi(String ssid, String pass);

	
  private:
    std::unique_ptr<DNSServer>        dnsServer;
    std::unique_ptr<ESP8266WebServer> server;

    //const int     WM_DONE                 = 0;
    //const int     WM_WAIT                 = 10;

    //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

    void          setupConfigPortal();
    void          startWPS();

    const char*   _apName                 = "no-net";
    const char*   _apPassword             = NULL;
    String        _ssid                   = "";
    String        _pass                   = "";
	char		  api[20]				  ;
	int           rPeriod				  = 10;			// in seconds
	int           uPeriod				  = 5 * 60;		// in seconds
	int			  port					  = 80;
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 0;
    unsigned long _configPortalStart      = 0;

	int			  lastPage				  = -1;

    IPAddress     _ap_static_ip;
    IPAddress     _ap_static_gw;
    IPAddress     _ap_static_sn;
    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    int           _paramsCount            = 0;
    int           _minimumQuality         = -1;
    boolean       _removeDuplicateAPs     = true;
    boolean       _shouldBreakAfterConfig = false;
    boolean       _tryWPS                 = false;

    const char*   _customHeadElement      = "";

    //String        getEEPROMString(int start, int len);
    //void          setEEPROMString(int start, int len, String string);

    int           status = WL_IDLE_STATUS;
    
    uint8_t       waitForConnectResult();

    void          handleRoot();
    void          handlecWifi(boolean scan);
	void          handlecPeriod();
	void          handlecServer();
	void          handleConfigSave();
	void          handleInfo();
    void          handleReset();
    void          handleNotFound();
	void		  handleStatus(int stt);
    void          handle204();
    boolean       captivePortal();
    boolean       configPortalHasTimeout();

    // DNS server
    const byte    DNS_PORT = 53;

    //helpers
    int           getRSSIasQuality(int RSSI);
    boolean       isIp(String str);
    String        toStringIp(IPAddress ip);

    boolean       connect;
    boolean       _debug = true;

    void (*_apcallback)(WiFiManager*) = NULL;
    void (*_savecallback)(void) = NULL;

    WiFiManagerParameter* _params[WIFI_MANAGER_MAX_PARAMS];

    template <typename Generic>
    void          DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
      return  obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
      DEBUG_WM("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
      return false;
    }
};

#endif
