#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include "secrets.h"

#define BUSY_PIN      D6


ESP8266WebServer server(80);
SoftwareSerial splitflapSerial(D8, D7);

String lasttext = "";

const char MAIN_page[] PROGMEM = R"=====(
<html>

  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no" />
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <title>SplitFlap</title>
    <style>
      p {
       font-family: "Arial", Helvetica, sans-serif;
      }
      .flap-state {
        width: 30px;
        height: 30px;
        background-color: green;
        display: inline-block;
        border: 1px solid black;
      }

      .flap-state__busy {
        background-color: red;
        animation: flipping 1.5s linear infinite;
      }

      .flap-state__unknown {
        background-color: yellow;
      }

      input[name=text] {
        width: 200px;
        max-width: 90vw;
        padding: 5px 10px;
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

    <h1>
      <p>SplitFlap
      <span class="flap-state flap-state__unkown" id="state"></span></p>
    </h1>
    <div>
      <form action="/" method="post" id="frm">
        <label>
          <input type="text" maxlength=10 name="text" value="{lasttext}"></p>
        </label>
        <div>
          <p><input type="submit" value="Absenden"></p>
        </div>
      </form>
    </div>
</p>

    <script>
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
          if (val === "1") { // Wenn da eh n String zurück kommt nehmen wir den halt direkt ;)
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

void handleRoot() {
  if (server.hasArg("text")) {
    String text = server.arg("text");
    text.toUpperCase();
    lasttext = text;
    Serial.print("Text:");
    Serial.println(text);

    text.replace("Ä", "[");
    text.replace("Ö", "]");
    text.replace("Ü", "(");
    text.replace("ß", ")");
    text += "          ";
    String sfText = text.substring(0,10);

    splitflapSerial.print(sfText);
    splitflapSerial.print('\n');

    Serial.println(sfText);
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
  server.on("/getBusyState",[]() {server.send(200, "text/plain", String(digitalRead(BUSY_PIN)));});
  server.onNotFound(handleRoot);

  server.begin();
}

void loop() {
  server.handleClient();
  digitalWrite(LED_BUILTIN, !digitalRead(BUSY_PIN));
}
