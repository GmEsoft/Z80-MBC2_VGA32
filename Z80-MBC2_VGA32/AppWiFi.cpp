#include "AppWiFi.h"

#include "AppStream.h"
#include "log.h"

#include "Preferences.h"
#include "WiFi.h"
#include "esp_wifi.h"

extern Preferences preferences;

void AppWiFi::setup()
{
}

bool AppWiFi::configure()
{
    String ssid = getSsid();
    String password = preferences.getString("wifi-passwd", "");

#if USE_BUTTON
    bool button = !digitalRead(USER_BUTTON);
#else
    bool button = false;
#endif

    bool reconfig = button;
    if ( reconfig ) {
        ssid.clear();
        AppStream.println("Reconfiguring...");
    }
    
    //connect to WiFi
    WiFi.mode(WIFI_STA);
    
    while ( true ) {
        bool ok = connect( ssid, password );
        
        if (WiFi.status() != WL_CONNECTED) {

            AppStream.println("Scanning networks...");
        
            // WiFi.scanNetworks will return the number of networks found
            WiFi.disconnect();
            int n = WiFi.scanNetworks();
            if (n == 0) {
                AppStream.println("no network found");
                ssid.clear();
                password.clear();
                reconfig = true;                
                break;
            } else {
                AppStream.print(n);
                AppStream.println(" networks found");
                for (int i = 0; i < n; ++i) {
                    // Print SSID and RSSI for each network found
                    AppStream.print(i + 1);
                    AppStream.print(": ");
                    AppStream.print(WiFi.SSID(i));
                    AppStream.print(" (");
                    AppStream.print(WiFi.RSSI(i));
                    AppStream.print(")");
                    AppStream.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
                    //delay(10);
                }
                AppStream.print("Select network (0=none): ");
                while ( AppStream.available() ) 
                  AppStream.read();
                String str = AppStream.input();
                bool askPassword = true;
                if ( !str.isEmpty() ) {
                    //AppStream.print(str.length());
                    int i = str.toInt();
                    if ( !i ) {
                        AppStream.print("No network selected");
                        ssid.clear();
                        password.clear();
                        reconfig = true;                
                        break;
                    }
                    if ( i > 0 && i <= n )
                        ssid = WiFi.SSID( i - 1 );
                    askPassword = WiFi.encryptionType( i - 1 ) != WIFI_AUTH_OPEN;
                }
                if ( askPassword && !ssid.isEmpty() ) {
                    AppStream.printf("Enter password for %s: ", ssid.c_str() );
                    while ( AppStream.available() ) 
                      AppStream.read();
                    password = AppStream.input();
                } else {
                    password.clear();
                }
                reconfig = true;                
            }
        } else {
            AppStream.println(" OK");
            break;
        }
    }

    if ( reconfig ) {
        setSsid( ssid );
        setPasswd( password );
    }
    
    return true;
    
    
}

String AppWiFi::getSsid()
{
    return preferences.getString("wifi-ssid", "" ); 
}

void AppWiFi::setSsid( String ssid )
{
    preferences.putString( "wifi-ssid", ssid );
}

String AppWiFi::getPasswd()
{
    return preferences.getString("wifi-passwd", "");
}

void AppWiFi::setPasswd( String passwd )
{
    preferences.putString( "wifi-passwd", passwd );    
}

bool AppWiFi::connect()
{
    String ssid = getSsid();
    String passwd = getPasswd();

    return connect( ssid, passwd );
}

bool AppWiFi::connect( const String &ssid, const String &password )
{
    bool ok = false;

    //connect to WiFi
    WiFi.mode(WIFI_STA);
    
    if ( !ssid.isEmpty() ) {
        AppStream.printf("Connecting to %s ", ssid.c_str());
        
        WiFi.begin(ssid.c_str(), password.c_str());
        
        for (int i=0; i<20 && WiFi.status() != WL_CONNECTED; ++i) {
            delay(1000);
            //AppStream.print(WiFi.status());
            AppStream.print(".");
        }

        ok =  WiFi.status() == WL_CONNECTED;
        AppStream.println( ok ? " OK" : " FAILED" );
    }

    return ok;
}

bool AppWiFi::disconnect()
{
    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    if ( false ) {
        esp_err_t ret;
        ret = esp_wifi_stop();
        LOGD( "esp_wifi_stop()   -> 0x%x %s", ret, esp_err_to_name( ret ) );
        ret = esp_wifi_deinit();
        LOGD( "esp_wifi_deinit() -> 0x%x %s", ret, esp_err_to_name( ret ) );
    }    
    return true;
}
