#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include "secrets.h"

#define BUSY_PIN      D6

ESP8266WebServer server(80);
SoftwareSerial splitflapSerial(D8, D7);

String lasttext = "";

struct {
   String msg;
} SplitFlapCommand[255];

uint8_t msgcnt = 0;
uint8_t msgidx = 0;
unsigned long stopMillis = 0;
uint16_t waitMillisBetween  = 2000;

const char MAIN_page[] PROGMEM = R"=====(
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no" />
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <title>SplitFlap</title>
    <style>
      body{
        text-align: center;
        font-family:verdana;
        background-color: #303030;
      } 
  
      button{
        border:0;
        border-radius:0.3rem;
        background-color: #83b817;
        color:#fff;
        line-height:2.4rem;
        font-size:1.2rem;
        width:240px;
        padding: 5px 10px;
        margin-top: 5px;
      } 
      
      .listbtn {
        width:76px;
        padding: 5px 10px;
        margin-bottom: 5px;
      }
      
      label {
        color: white;
        font-size:1.4rem; 
        padding: 10px;
      }

      #headline {
        display:flex;
        align-items:center;
        justify-content: center;
       }
       
       #textliste {
         width:240px;
         height: 285px;
         font-size:1.4rem; 
         padding: 5px 10px;
         background-color: #606060;
         color: white;
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
        padding: 5px 10px;
        font-size:1.4rem;
      }
      
      input[type=number] {
        background-color: #606060;
        color: white;
        width: 50px;
        font-size:1.4rem;
      }

      input[type=submit] {
        margin-top: 5px;
        padding: 5px 10px;
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
  <body>
    <h1 style="color:#ddd;" id="headline">
      SplitFlap
      <span class="flap-state flap-state__unkown" id="state"></span>
    </h1>
    <div>
      <form action="/" method="post">
        <div><label for="reset_module">Modul</label><input id="reset_module" type="number" name="reset" value="0" min="0" max="12"><label>(0=All)</label></div>
        <div> <button>Reset</button></div>
      </form>
      
      <form action="/" method="post" id="frm">
        <div><input id="textfield" type="text" maxlength=12 onfocus="this.value=''" name="text" value=""></div>
        <div><button type="submit">Send</button></div>   
      </form>
      
      <form>
        <div>
          <button class="listbtn" type="button" onclick="addToSelect()">Add</button>
          <button class="listbtn" type="button" onclick="clearSelect()">Clear</button>
          <button class="listbtn" type="button" onclick="goSelect()">GO</button>
        </div>
        <div><select id="textliste" size="10" multiple ondblclick="onDblClickFunction();">
        </select></div>

      </form>

    </div>
    <div class="center">
</div>

    <script>   
       function clearSelect() {
          var oSelField = document.getElementById("textliste");
          oSelField.options.length=0;
       }
       
       function onDblClickFunction() {
          document.getElementById('textliste').ondblclick = function(){
          this.remove(this.selectedIndex);
         };
       }
       
       function goSelect() { 
         selectBox = document.getElementById("textliste");
         
         var sftext='';

         for (var i = 0; i < selectBox.options.length; i++) { 
           sftext+=selectBox.options[i].value+';';
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


       function addToSelect() {
       var textField = document.getElementById("textfield");
       var oSelField = document.getElementById("textliste");
       var oOption = document.createElement("OPTION");
       oSelField.options.add(oOption);
       oOption.text = textField.value;
       oOption.value = (textField.value==='') ? '          ' : textField.value;
       textField.value='';
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

void addTextToSplitFlapMessageBuffer(String text) {
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

  SplitFlapCommand[msgcnt].msg = text.substring(0,12);
  Serial.print("addToSplitFlapMessageBuffer #");Serial.print(msgcnt, DEC);Serial.print(" : '");Serial.print(text);;Serial.println("'");
  msgcnt++;
}

void handleRoot() {
  if (server.hasArg("reset")) {
    int modnum = atoi(server.arg("reset").c_str());
    if (modnum == 0) splitflapSerial.print("%za\n");
    else  {
      splitflapSerial.print("%z");
      splitflapSerial.print(String(modnum-1));
      splitflapSerial.print('\n');
    }
  }

  if (server.hasArg("textlist")) {
    String textlist = server.arg("textlist");
    // Convert from String Object to String.
    char buf[textlist.length()+1];
    textlist.toCharArray(buf, sizeof(buf));
    char *p = buf;
    char *str;
    while ((str = strtok_r(p, ";", &p)) != NULL) // delimiter is the semicolon
      addTextToSplitFlapMessageBuffer(str);
  }


  if (server.hasArg("text")) {
    String text = server.arg("text");
    addTextToSplitFlapMessageBuffer(text);
  }

  String s = FPSTR(MAIN_page); //Read HTML contents
  s.replace("{lasttext}", lasttext);
  server.send(200, "text/html", s); //Send web page
}

void setup() {
  Serial.begin(57600);
  pinMode(BUSY_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  splitflapSerial.begin(57600);
  setup_wifi();

  server.on("/", handleRoot);
  server.on("/getBusyState",[]() {server.send(200, "text/plain", String(getBusyState()));});
  server.onNotFound(handleRoot);

  server.begin();
}

void loop() {
  server.handleClient();
  digitalWrite(LED_BUILTIN, !getBusyState());

  if (getBusyState() == true) stopMillis = millis();

  if (millis() - stopMillis > waitMillisBetween) {
    if (msgcnt > 0) {
      Serial.print("Processing Message #");Serial.print(msgidx,DEC);Serial.print(" : '");Serial.print(SplitFlapCommand[msgidx].msg);Serial.println("'");
      splitflapSerial.print(SplitFlapCommand[msgidx].msg);
      splitflapSerial.print('\n');

      msgidx++;

      stopMillis = millis();

      if (msgidx >= msgcnt) {
        msgcnt = 0;
        msgidx = 0;
      }
    }
  }
}
