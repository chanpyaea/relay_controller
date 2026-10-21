#include <ESPAsyncWebServer.h>
#include <Update.h>
#include "relay_manager.h"
#include "ac_monitor.h"

AsyncWebServer server(80);

// String htmlPage()
// {
//     String html = R"rawliteral(
//     <!DOCTYPE html>
//     <html>
//     <head>
//         <title>ESP32 Relay Controller</title>
//         <meta name='viewport' content='width=device-width, initial-scale=1'>
//         <style>
//             body {
//                 font-family: Arial, sans-serif;
//                 text-align: center;
//                 margin-top: 40px;
//                 background: #f4f4f4;
//             }
//             .relay {
//                 display: inline-block;
//                 margin: 20px;
//                 padding: 20px 30px;
//                 border-radius: 12px;
//                 background: #fff;
//                 box-shadow: 0 2px 8px rgba(0,0,0,0.08);
//             }
//             .relay-title {
//                 font-size: 1.3em;
//                 margin-bottom: 10px;
//             }
//             .relay-btn {
//                 width: 120px;
//                 height: 50px;
//                 font-size: 20px;
//                 border: none;
//                 border-radius: 8px;
//                 color: #fff;
//                 cursor: pointer;
//                 margin-top: 10px;
//                 transition: background 0.2s;
//             }
//             .on {
//                 background: #27ae60;
//             }
//             .off {
//                 background: #c0392b;
//             }
//         </style>
//     </head>
//     <body>
//         <h1>ESP32 Relay Controller</h1>
//         <div id="relays"></div>
//         <button style="margin:30px auto 0 auto;display:block;padding:12px 32px;font-size:18px;border-radius:8px;background:#3498db;color:#fff;border:none;cursor:pointer;" onclick="location.href='/update'">OTA Update</button>
//         <script>
//         function fetchStates() {
//             fetch('/api/relay_states')
//                 .then(r => r.json())
//                 .then(states => {
//                     let html = '';
//                     for (let i = 0; i < 3; i++) {
//                         let state = states[i] ? 'ON' : 'OFF';
//                         let btnClass = states[i] ? 'relay-btn on' : 'relay-btn off';
//                         let btnText = states[i] ? 'Turn OFF' : 'Turn ON';
//                         html += `<div class='relay'><div class='relay-title'>Relay ${i+1}</div>`;
//                         html += `<button class='${btnClass}' onclick='toggleRelay(${i+1},${states[i] ? 0 : 1})'>${btnText}</button>`;
//                         html += `<div style='margin-top:8px;font-size:1.1em;'>State: <b style='color:${states[i] ? '#27ae60' : '#c0392b'}'>${state}</b></div></div>`;
//                     }
//                     document.getElementById('relays').innerHTML = html;
//                 });
//         }
//         function toggleRelay(relay, state) {
//             fetch(`/api/relay/${relay}/${state ? 'on' : 'off'}`)
//                 .then(fetchStates); // update UI immediately after relay toggled
//         }
//         setInterval(fetchStates, 1000); // poll every 1s for better responsiveness
//         window.onload = fetchStates;
//         </script>
//     </body>
//     </html>
//     )rawliteral";

//     return html;
// }
String htmlPage()
{
    String html = R"rawliteral(

<!DOCTYPE html>
<html>

<head>

    <title>ESP32 Smart Controller</title>

    <meta name="viewport" content="width=device-width, initial-scale=1">

    <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet">

    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;500;600;700&display=swap" rel="stylesheet">

    <link href="https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/css/materialize.min.css" rel="stylesheet">

    <style>

        *
        {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body
        {
            font-family: 'Poppins', sans-serif;

            background:
                linear-gradient(
                    135deg,
                    #0f172a 0%,
                    #111827 50%,
                    #1e293b 100%
                );

            min-height: 100vh;

            color: white;

            overflow-x: hidden;
        }

        .background-blur
        {
            position: fixed;

            width: 300px;
            height: 300px;

            border-radius: 50%;

            background: rgba(59,130,246,0.15);

            filter: blur(100px);

            z-index: -1;
        }

        .blur1
        {
            top: -100px;
            left: -100px;
        }

        .blur2
        {
            bottom: -100px;
            right: -100px;
        }

        nav
        {
            background: rgba(255,255,255,0.05);

            backdrop-filter: blur(20px);

            box-shadow: none;

            border-bottom:
                1px solid rgba(255,255,255,0.08);
        }

        .brand-logo
        {
            font-weight: 600 !important;

            font-size: 1.3rem !important;
        }

        .dashboard
        {
            padding-top: 30px;
            padding-bottom: 40px;
        }

        .status-card
        {
            background: rgba(255,255,255,0.06);

            border-radius: 24px;

            padding: 20px;

            margin-bottom: 30px;

            backdrop-filter: blur(18px);

            border:
                1px solid rgba(255,255,255,0.08);

            box-shadow:
                0 8px 32px rgba(0,0,0,0.2);
        }

        .status-title
        {
            font-size: 1rem;

            opacity: 0.7;
        }

        .status-value
        {
            font-size: 1.8rem;

            font-weight: 600;

            margin-top: 8px;
        }

        .electricity-on { color: #00e676; }
        .electricity-off { color: #ff5252; }

        .relay-grid
        {
            display: grid;

            grid-template-columns:
                repeat(auto-fit, minmax(280px, 1fr));

            gap: 25px;
        }

        .relay-card
        {
            position: relative;

            background:
                rgba(255,255,255,0.06);

            border-radius: 28px;

            padding: 28px;

            overflow: hidden;

            backdrop-filter: blur(20px);

            border:
                1px solid rgba(255,255,255,0.08);

            transition: 0.3s;

            box-shadow:
                0 10px 35px rgba(0,0,0,0.25);
        }

        .relay-card:hover
        {
            transform: translateY(-6px);
        }

        .relay-icon
        {
            font-size: 70px !important;

            margin-bottom: 10px;
        }

        .icon-on
        {
            color: #00e676;

            animation: glow 1.5s infinite;
        }

        .icon-off
        {
            color: #607d8b;
        }

        @keyframes glow
        {
            0%
            {
                opacity: 0.7;
                transform: scale(1);
            }

            50%
            {
                opacity: 1;
                transform: scale(1.08);
            }

            100%
            {
                opacity: 0.7;
                transform: scale(1);
            }
        }

        .relay-title
        {
            font-size: 1.4rem;

            font-weight: 600;
        }

        .relay-status
        {
            margin-top: 10px;

            font-size: 1rem;

            opacity: 0.9;
        }

        .status-on
        {
            color: #00e676;
            font-weight: 600;
        }

        .status-off
        {
            color: #ff5252;
            font-weight: 600;
        }

        .relay-btn
        {
            margin-top: 25px;

            width: 100%;

            height: 52px;

            border-radius: 18px;

            font-weight: 600;

            letter-spacing: 0.5px;

            font-size: 1rem;

            border: none;
        }

        .on-btn
        {
            background:
                linear-gradient(
                    135deg,
                    #00c853,
                    #00e676
                );
        }

        .off-btn
        {
            background:
                linear-gradient(
                    135deg,
                    #d50000,
                    #ff1744
                );
        }

        .ota-section
        {
            margin-top: 40px;

            text-align: center;
        }

        .ota-btn
        {
            height: 58px;

            line-height: 58px;

            padding: 0 35px;

            border-radius: 18px;

            background:
                linear-gradient(
                    135deg,
                    #2196f3,
                    #21cbf3
                );

            font-weight: 600;

            font-size: 1rem;

            box-shadow:
                0 10px 30px rgba(33,150,243,0.3);
        }

        .footer
        {
            text-align: center;

            margin-top: 40px;

            opacity: 0.5;

            font-size: 0.9rem;
        }

        @media(max-width: 600px)
        {
            .brand-logo
            {
                font-size: 1rem !important;
            }

            .relay-card
            {
                padding: 22px;
            }

            .relay-icon
            {
                font-size: 55px !important;
            }
        }

    </style>

</head>

<body>

    <div class="background-blur blur1"></div>

    <div class="background-blur blur2"></div>

    <nav>

        <div class="nav-wrapper container">

            <a class="brand-logo center">

                <i class="material-icons left">
                    memory
                </i>

                ESP32 Smart Controller

            </a>

        </div>

    </nav>



    <div class="container dashboard">

        <div class="status-card">

            <div class="status-title">

                ELECTRICITY SUPPLY

            </div>

            <div class="status-value" id="electricity-status">

                CHECKING...

            </div>

        </div>



        <div class="relay-grid" id="relays"></div>



        <div class="ota-section">

            <a href="/update"
               class="btn-large waves-effect waves-light ota-btn">

                <i class="material-icons left">
                    system_update_alt
                </i>

                Firmware OTA Update

            </a>

        </div>



        <div class="footer">

            ESP32 Industrial IoT Relay Controller

        </div>

    </div>



<script>

function fetchStates()
{
    fetch('/api/relay_states')

    .then(r => r.json())

    .then(states =>
    {
        let html = '';

        for(let i = 0; i < 3; i++)
        {
            let isOn = states[i];

            let stateText = isOn ? 'ACTIVE' : 'INACTIVE';

            let stateClass = isOn
                ? 'status-on'
                : 'status-off';

            let btnClass = isOn
                ? 'off-btn'
                : 'on-btn';

            let btnText = isOn
                ? 'TURN OFF'
                : 'TURN ON';

            let iconClass = isOn
                ? 'icon-on'
                : 'icon-off';

            let relayIcon = isOn
                ? 'toggle_on'
                : 'toggle_off';

            let buttonIcon = isOn
                ? 'power_settings_new'
                : 'flash_on';

            html += `

            <div class="relay-card">

                <center>

                    <i class="material-icons relay-icon ${iconClass}">
                        ${relayIcon}
                    </i>

                    <div class="relay-title">

                        Relay ${i + 1}

                    </div>

                    <div class="relay-status">

                        Status:

                        <span class="${stateClass}">
                            ${stateText}
                        </span>

                    </div>

                </center>

                <button
                    class="btn relay-btn ${btnClass} waves-effect waves-light"
                    onclick="toggleRelay(${i+1}, ${isOn ? 0 : 1})">

                    <i class="material-icons left">
                        ${buttonIcon}
                    </i>

                    ${btnText}

                </button>

            </div>

            `;
        }

        document.getElementById('relays').innerHTML = html;
    });
}

function fetchElectricityStatus()
{
    fetch('/api/electricity_status')
    .then(r => r.json())
    .then(data =>
    {
        const status = document.getElementById('electricity-status');
        status.textContent = data.electricity_on ? 'POWER ON' : 'POWER CUT OFF';
        status.className = 'status-value ' +
            (data.electricity_on ? 'electricity-on' : 'electricity-off');
    });
}



function toggleRelay(relay, state)
{
    fetch(`/api/relay/${relay}/${state ? 'on' : 'off'}`)

    .then(() =>
    {
        fetchStates();
    });
}



setInterval(fetchStates, 1000);
setInterval(fetchElectricityStatus, 1000);

window.onload = function()
{
    fetchStates();
    fetchElectricityStatus();
};

</script>

</body>

</html>

)rawliteral";

    return html;
}
void webServerInit()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", htmlPage());
    });

    // API: Get relay states
    server.on("/api/relay_states", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "[";
        for (int i = 1; i <= 3; i++) {
            json += getRelayState(i) ? "true" : "false";
            if (i < 3) json += ",";
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    server.on("/api/electricity_status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = String("{\"electricity_on\":") +
            (isElectricityOn() ? "true" : "false") + "}";
        request->send(200, "application/json", json);
    });

    // API: Set relay state - handled in onNotFound

    // Handle /api/relay/{relay}/{on|off} in onNotFound
    server.onNotFound([](AsyncWebServerRequest *request) {
        String url = request->url();
        if (url.startsWith("/api/relay/")) {
            String path = url.substring(11); // after /api/relay/
            int slash = path.indexOf('/');
            if (slash > 0) {
                int relay = path.substring(0, slash).toInt();
                String action = path.substring(slash + 1);
                if (relay >= 1 && relay <= 3 && (action == "on" || action == "off")) {
                    Serial.printf("[WEB] Button pressed: Relay %d -> %s\n", relay, action.c_str());
                    setRelay(relay, action == "on");
                    Serial.printf("[WEB] Relay %d state is now: %s\n", relay, getRelayState(relay) ? "ON" : "OFF");
                    request->send(200, "text/plain", "OK");
                    return;
                }
            }
            Serial.println("[WEB] Invalid relay request");
            request->send(400, "text/plain", "Invalid relay request");
            return;
        }
        request->send(404, "text/plain", "Not found");
    });
    // OTA update page (GET)
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  String page = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <title>ESP32 OTA Update</title>
        <meta name='viewport' content='width=device-width, initial-scale=1'>
        <style>
            body { font-family: Arial; text-align: center; margin-top: 40px; background: #f4f4f4; }
            .progress { width: 80%; background: #eee; border-radius: 8px; margin: 20px auto; height: 30px; }
            .progress-bar { height: 100%; background: #27ae60; border-radius: 8px; width: 0%; color: #fff; line-height: 30px; font-weight: bold; transition: width 0.2s; }
            #log { width: 80%; margin: 20px auto; background: #fff; border-radius: 8px; min-height: 60px; padding: 10px; text-align: left; font-size: 1em; box-shadow: 0 2px 8px rgba(0,0,0,0.08); }
        </style>
    </head>
    <body>
        <h2>ESP32 Firmware Update</h2>
        <form id="otaForm">
            <input type="file" id="file" name="update" required>
            <input type="submit" value="Update">
        </form>
        <div class="progress"><div class="progress-bar" id="progressBar">0%</div></div>
        <div id="log"></div>
        <script>
        const form = document.getElementById('otaForm');
        const progressBar = document.getElementById('progressBar');
        const log = document.getElementById('log');
        form.onsubmit = function(e) {
            e.preventDefault();
            const fileInput = document.getElementById('file');
            if (!fileInput.files.length) return;
            const file = fileInput.files[0];
            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/update', true);
            xhr.upload.onprogress = function(e) {
                if (e.lengthComputable) {
                    const percent = Math.round((e.loaded / e.total) * 100);
                    progressBar.style.width = percent + '%';
                    progressBar.textContent = percent + '%';
                    log.textContent = `[OTA] Uploading: ${percent}% (${e.loaded}/${e.total} bytes)\n`;
                }
            };
            xhr.onload = function() {
                if (xhr.status == 200) {
                    log.textContent += '[OTA] Update successful! Rebooting...\n';
                } else {
                    log.textContent += '[OTA] Update failed: ' + xhr.responseText + '\n';
                }
            };
            xhr.onerror = function() {
                log.textContent += '[OTA] Upload error.\n';
            };
            log.textContent = '[OTA] Starting upload...\n';
            progressBar.style.width = '0%';
            progressBar.textContent = '0%';
            xhr.send(new FormData(form));
        };
        </script>
    </body>
    </html>
    )rawliteral";
                  request->send(200, "text/html", page);
              });

    // OTA firmware upload (POST)
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {},
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if(!index)
            {
                Serial.printf("[OTA] Update Start: %s\n", filename.c_str());
            }

            if(len)
            {
                if(index == 0) {
                    Serial.printf("[OTA] Begin update, total size unknown.\n");
                    if(!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                        Update.printError(Serial);
                    }
                }
                Serial.printf("[OTA] Writing %u bytes at offset %u...\n", len, index);
                if(Update.write(data, len) != len)
                {
                    Update.printError(Serial);
                }
            }

            if(final)
            {
                if(Update.end(true))
                {
                    Serial.printf("[OTA] Success: %u bytes\n", index + len);
                    // Redirect to main page after update
                    request->redirect("/");
                    Serial.println("[OTA] Waiting before reboot...");
                    delay(1200); // Wait 1.2s to allow browser to follow redirect
                    Serial.println("[OTA] Rebooting now...");
                    ESP.restart();
                }
                else
                {
                    Update.printError(Serial);
                    request->send(500, "text/plain", "OTA Update Failed");
                }
            }
        }
    );

    server.begin();

    Serial.println("Web Server Started");
}
