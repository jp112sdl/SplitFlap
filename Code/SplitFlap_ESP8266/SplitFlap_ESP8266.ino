#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "secrets.h"

#define BUSY_PIN      D5
#define NUM_MODULES   12

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
SoftwareSerial splitflapSerial(D7, D6);

String lasttext = "";
String serialInput = "";

const char * host = "SPLITFLAP";
bool blocked = false;

struct {
  String msg;
  int wait;
} SplitFlapCommand[64];

uint8_t msgcnt = 0;
uint8_t msgidx = 0;
unsigned long stopMillis = 0;
uint16_t waitMillisBetween  = 2000;
bool OTAStart = false;

const char HEAD[] PROGMEM = R"=====(
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no" />
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <meta name="apple-mobile-web-app-capable" content="yes">
    <title>SplitFlap</title>
    <style>
      body{
        text-align: center;
        font-family:verdana;
        background-color: #303030;
        color: white;
        font-size:1.4rem; 
        padding: 10px;
      } 
  
      input.lnkbtnred,input.lnkbtngn,input.lnkbtnback ,button{
        border:0;
        border-radius:0.3rem;
        color:#fff;
        line-height:2.4rem;
        font-size:1.2rem;
        width:240px;
        padding: 5px 10px;
        margin-top: 5px;
      } 

      button {
        background-color: #83b817;
      }

      input.lnkbtnback {
        background-color: #1fa3ec;
        padding: 5px 10px;
        margin-bottom: 5px;
        width:114px;
      }

      input.lnkbtnred {
        background-color: #ff6666;
      } 
      
      .listbtn {
        width:114px;
        padding: 5px 10px;
        margin-bottom: 5px;
      }

      .savebtn {
        width:114px;
        padding: 5px 10px;
        margin-bottom: 5px;
        background-color: #ff6666;
      }

      input.lnkbtnred,input.lnkbtngn,.savebtn {
        -webkit-appearance: button;
        -moz-appearance: button;
        appearance: button;
      } 
      
      label {
        color: white;
        font-size:1.4rem; 
        padding: 10px;
      }

      table {
         margin: 0 auto;
      }
      
      td.name {
       text-align: right;
       font-size:1.4rem;
      }

      #headline {
        display:flex;
        align-items:center;
        justify-content: center;
       }
  
      .flap-state {
        width: 30px;
        height: 30px;
        background-color: #83b817;
        display: inline-block;
        border: 1px solid #303030;
        margin: 5px 10px;
      }

      .flap-state__busy {
        background-color: red;
        animation: flipping 1.5s linear infinite;
      }

      .flap-state__unknown {
        background-color: yellow;
      }

      input[name=text] {
        width: 240px;
        max-width: 90vw;
        background-color: #606060;
        color: white;
        padding: 5px 5px;
        font-size:1.4rem;
      }
      
      .num {
        background-color: #606060;
        color: white;
        font-size:1.4rem;
        padding: 5px 5px;
      }

      .waittime {
        width: 55px;
      }
      
      .reset {
        width: 65px;
      }

      .offset {
        width: 80px;
        text-align:left;
      }

      input[type=submit] {
        margin-top: 5px;
        padding: 5px 10px;
      }

     form {
        margin-bottom: 0em;
      }

      @keyframes flipping {
        from {
          transform: rotateX(0deg);
        }

        to {
          transform: rotateX(360deg);
        }
      }
    </style>
  </head>
)=====";

const char CONFIG_page[] PROGMEM = R"=====(
<html>
  {head}
  <body>
    <div class="center">
      <form action="/config" method="post">
      <div><table>
        {offset_lines}
      </table></div>
        <div><button  class='savebtn' type="submit">Save</button>
      <input class='lnkbtnback' type='button' value='Back' onclick="window.location.href='/'" /></div> 
      </form>
    </div>
  </body>
</html>
)=====";

const char MAIN_page[] PROGMEM = R"=====(
<html>
  {head}
  <body>
    <h1 style="color:#ddd;" id="headline">
      SplitFlap
      <span class="flap-state flap-state__unkown" id="state"></span>
    </h1>

    <div class="center">
      <form action="/" method="post">
        <div><label for="reset_module">Modul</label><input class="num reset" id="reset_module" type="number" name="reset" value="0" min="0" max="{num_modules}"><label>(0=All)</label></div>
        <div> <button>Reset</button></div>
      </form>
      
      <form action="/" method="post" id="frm">
        <div><input id="textfield" type="text" maxlength={num_modules} onfocus="this.value=''" name="text" value="" placeholder="Single Word"></div>
        <div><button type="submit">Send</button></div>   
      </form>
      
      <form>
        <div>
          <button class="listbtn" type="button" onclick="ClearBtn()">Clear</button>
          <button class="listbtn" type="button" onclick="GoBtn();">GO</button>
        </div>
        <div><input id="textline1" type="text" maxlength={num_modules} onfocus="this.value=''" name="text" value="{t1}" placeholder="{t1}">&nbsp;<input class="num waittime" id="waitline1" type="number" name="waitline" value="{w1}" min="1" max="60">s</div>
        <div><input id="textline2" type="text" maxlength={num_modules} onfocus="this.value=''" name="text" value="{t2}" placeholder="{t2}">&nbsp;<input class="num waittime" id="waitline2" type="number" name="waitline" value="{w2}" min="1" max="60">s</div>
        <div><input id="textline3" type="text" maxlength={num_modules} onfocus="this.value=''" name="text" value="{t3}" placeholder="{t3}">&nbsp;<input class="num waittime" id="waitline3" type="number" name="waitline" value="{w3}" min="1" max="60">s</div>
        <div><input id="textline4" type="text" maxlength={num_modules} onfocus="this.value=''" name="text" value="{t4}" placeholder="{t4}">&nbsp;<input class="num waittime" id="waitline4" type="number" name="waitline" value="{w4}" min="1" max="60">s</div>
        <div><input id="textline5" type="text" maxlength={num_modules} onfocus="this.value=''" name="text" value="{t5}" placeholder="{t5}">&nbsp;<input class="num waittime" id="waitline5" type="number" name="waitline" value="{w5}" min="1" max="60">s</div>
        <div><input id="textline6" type="text" maxlength={num_modules} onfocus="this.value=''" name="text" value="{t6}" placeholder="{t6}">&nbsp;<input class="num waittime" id="waitline6" type="number" name="waitline" value="{w6}" min="1" max="60">s</div>
      </form>
      <div>
        <input class='lnkbtnred' type='button' value='Konfiguration' onclick="window.location.href='/config'" />
      </div>
    </div>

    <script>   
       function ClearBtn() {
         for (var i = 1; i < 7; i++) { 
           var t  = document.getElementById("textline"+i);
           var w =  document.getElementById("waitline"+i);
           t.value="";
           w.value=2;
          }
       }
       
       function GoBtn() { 
         var sftext='';

         for (var i = 1; i < 7; i++) { 
           var t  = document.getElementById("textline"+i).value;
           if (t.length > 0) {
             var w = document.getElementById("waitline"+i).value;
             sftext+=t+'\''+w+';';
           }
         } 
         
         console.log(sftext);
         post('/', {textlist: sftext});
       }
       
       function post(path, params, method='post') {
         const form = document.createElement('form');
         form.method = method;
         form.action = path;

         for (const key in params) {
           if (params.hasOwnProperty(key)) {
             const hiddenField = document.createElement('input');
             hiddenField.type = 'hidden';
             hiddenField.name = key;
             hiddenField.value = params[key];

             form.appendChild(hiddenField);
           }
          }

          document.body.appendChild(form);
          form.submit();
       }


      (function() {
        var state = document.getElementById("state");
        var frm = document.getElementById("frm");

        function httpGet(u) {
          return new Promise(function(resolve, reject) {
            var xhr = new XMLHttpRequest();
            xhr.addEventListener("load", function() {
              console.log('XHR response', this.responseText);
              resolve(this.responseText);
            });
            xhr.open('GET', u);
            xhr.send();
          });
        }

        function setState(val) {
          console.log('setState', val);
          state.classList.remove('flap-state__unkown');
          if (val === "1") {
            state.classList.add('flap-state__busy');
          } else {
            state.classList.remove('flap-state__busy');
          }
        }

        frm.addEventListener('submit', function(ev) {
          ev.preventDefault();
          setState("1");
          var url = frm.action;
          var xhr = new XMLHttpRequest();
          xhr.open("POST", url);
          xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
          xhr.send('text=' + encodeURIComponent(frm.elements.text.value));
        });

        function stateRefresher() {
          httpGet('/getBusyState').then(setState);
        }
        // Refresh state every 1s
        setInterval(stateRefresher, 1000);
      })()
    </script>
  </body>
</html>

)=====";

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  printWifiStatus();
}

bool getBusyState() {
  return digitalRead(BUSY_PIN);
}

void addTextToSplitFlapMessageBuffer(String text, int wait) {
  text.toUpperCase();
  lasttext = text;
  //Serial.print("Text:");
  //Serial.println(text);

  text.replace("Ä", "[");
  text.replace("ä", "[");
  text.replace("Ö", "]");
  text.replace("ö", "]");
  text.replace("Ü", "(");
  text.replace("ü", "(");
  text.replace("ß", ")");
  text += "            ";

  SplitFlapCommand[msgcnt].msg = text.substring(0,NUM_MODULES);
  SplitFlapCommand[msgcnt].wait = wait;
  Serial.print("addToSplitFlapMessageBuffer #");Serial.print(msgcnt, DEC);Serial.print(" : '");Serial.print(text);;Serial.print("' Wait: ");Serial.println(wait,DEC);
  msgcnt++;
}

void handleRoot() {
  String s = FPSTR(MAIN_page); //Read HTML contents
  s.replace("{head}", FPSTR(HEAD));

  if (server.hasArg("reset")) {
    int modnum = atoi(server.arg("reset").c_str());
    if (modnum == 0) splitflapSerial.print("%za\n");
    else  {
      splitflapSerial.print("%z");
      splitflapSerial.print(String(modnum-1));
      splitflapSerial.print('\n');
    }
  }

  uint8_t tlcnt = 0;
  if (server.hasArg("textlist")) {
    String textlist = server.arg("textlist");
    // Convert from String Object to String.
    char buf[textlist.length()+1];
    textlist.toCharArray(buf, sizeof(buf));
    char *p = buf;
    char *str;
    while ((str = strtok_r(p, ";", &p)) != NULL) { // delimiter is the semicolon
      String sStr = String(str);
      
      int idxDelim = sStr.indexOf("'");
      String txt = sStr.substring(0,idxDelim);

      int wait = atoi(sStr.substring(idxDelim+1,sStr.length()).c_str());

      addTextToSplitFlapMessageBuffer(txt, wait);
      s.replace("{t"+String(++tlcnt)+"}", txt);
      s.replace("{w"+String(tlcnt)+"}", String(wait));
    }
  }

  for (uint8_t i = tlcnt; i < 7; i++) {
    s.replace("{t"+String(i)+"}", "");
    s.replace("{w"+String(i)+"}", "2");
  }

  if (server.hasArg("text")) {
    String text = server.arg("text");
    addTextToSplitFlapMessageBuffer(text, 0);
  }

  s.replace("{num_modules}", String(NUM_MODULES));
  server.send(200, "text/html", s); //Send web page
}

void handleConfig() {
  blocked = true;

  bool save = false;
  for (uint8_t i = 0; i < NUM_MODULES; i++) {
    if (server.hasArg("offset"+String(i))) {
      save = true;
      int val = atoi(server.arg("offset"+String(i)).c_str());
      splitflapSerial.print("%o");
      if (i < 10) splitflapSerial.print("0");
      splitflapSerial.print(i, DEC);
      splitflapSerial.print(val);
      splitflapSerial.print("\n");
      delay(10);
      Serial.print("offset");Serial.print(i,DEC);Serial.print(":");Serial.println(val, DEC);
    }
  }

  if (save) delay(700);
  
  splitflapSerial.print("%g\n");

  unsigned long start = millis();
  String json = "";
  while (millis() - start < 1000) {
    bool newChar = false;
    while (splitflapSerial.available()) {
      char in = splitflapSerial.read();
      if (in == '\n') {
         newChar = true;
       } else {
         json += in;
       }
    }

    if (newChar) {
      newChar = false;
      Serial.println(json);
      break;
    }
  }

  if (json == "") {
    Serial.println("Serial TIMEOUT");
  }

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, json);
   if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
  }

  
  
  String s = FPSTR(CONFIG_page); //Read HTML contents
  s.replace("{head}", FPSTR(HEAD));

  String offset_lines = "";
  for (uint8_t i = 0; i < NUM_MODULES; i++) {
    uint16_t val = doc["ZERO_OFFSET"][i].as<uint16_t>();
    offset_lines+="<tr><td class=\"name\">Offset "+String(i+1)+":</td><td><input class=\"num offset\" id=\"offset"+String(i)+"\" type=\"number\" name=\"offset"+String(i)+"\" value=\""+String(val)+"\" min=\"0\" max=\"4096\"></td></tr>";
  }
  s.replace("{offset_lines}", offset_lines);

  server.send(200, "text/html", s); //Send web page
  blocked = false;
}

void startOTAhandling() {
  Serial.print(F("Starte OTA-Handler... "));
  ArduinoOTA.onStart([]() {
    Serial.println(F("Start updating"));
    OTAStart = true;
  });
  ArduinoOTA.onEnd([]() {
    Serial.print("\nEnd");
    OTAStart = false;
  });
  ArduinoOTA.onProgress([](__attribute__ ((unused))unsigned int progress,__attribute__ ((unused)) unsigned int total) {
    Serial.print(".");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    OTAStart = false;
    Serial.print("Error "+String(error));
    if (error == OTA_AUTH_ERROR) Serial.print("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.print("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.print("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.print("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.print("End Failed");
  });

  ArduinoOTA.setHostname(host);
  ArduinoOTA.begin();
  Serial.println("done");
}

void setup() {
  Serial.begin(57600);
  pinMode(BUSY_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  splitflapSerial.begin(19200);
  setup_wifi();

  MDNS.begin(host);

  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/getBusyState",[]() {server.send(200, "text/plain", String(getBusyState()));});
  server.onNotFound(handleRoot);

  startOTAhandling();

  httpUpdater.setup(&server);
  
  server.begin();

}

void loop() {
  //auf OTA Anforderung reagieren
  ArduinoOTA.handle();

  if (!OTAStart) {
    server.handleClient();
    digitalWrite(LED_BUILTIN, !getBusyState());

    bool newChar = false;
    if (splitflapSerial.available() && !blocked) {
      char in = splitflapSerial.read();
      if (in == '\n') {
         newChar = true;
       } else {
         serialInput += in;
       }
    }

    if (newChar) {
      newChar = false;
      Serial.print("IN splitflapSerial: ");Serial.println(serialInput);
      serialInput = "";
    }

    if (getBusyState() == true) stopMillis = millis();

    if (millis() - stopMillis > waitMillisBetween) {
      if (msgcnt > 0) {
        Serial.print("Wait time was: ");Serial.println(waitMillisBetween, DEC);
        Serial.print("Processing Message #");Serial.print(msgidx,DEC);Serial.print(" : '");Serial.print(SplitFlapCommand[msgidx].msg);Serial.println("'");
        splitflapSerial.print(SplitFlapCommand[msgidx].msg);
        splitflapSerial.print('\n');

        waitMillisBetween = SplitFlapCommand[msgidx].wait * 1000;
        stopMillis = millis();

        msgidx++;

        if (msgidx >= msgcnt) {
          msgcnt = 0;
          msgidx = 0;
        }
      }
    }
  }
}
