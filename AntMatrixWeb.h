// AntMatrix - ESP32 glue: NVS storage (Preferences) and web UI/API (WebServer)
//
// Routes registered by amWebBegin():
//   GET  /                      switch page (status + band x output matrix)
//   GET  /am.css                shared stylesheet (host pages may link it)
//   GET  /api/am/state          live state of all TRX
//   GET  /api/am/config         saved config, ?defaults=1 returns defaults (not applied)
//   POST /api/am/config         validate, save to NVS, apply
//   POST /api/am/select         {"trx":0,"out":12} manual choice, out -1 = release
// Depends on ArduinoJson 7.

#ifndef ANTMATRIX_WEB_H
#define ANTMATRIX_WEB_H

#include <WebServer.h>
#include "AntMatrix.h"

bool amStoreLoad(AmConfig& cfg);        // false = nothing stored or incompatible
bool amStoreSave(const AmConfig& cfg);
// navHtml: extra top bar links, e.g. "<a class=\"tab\" href=\"/setup\">SETUP</a>" (may be NULL)
void amWebBegin(WebServer& srv, const char* navHtml);

#endif
