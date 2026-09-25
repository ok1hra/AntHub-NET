// AntMatrix - ESP32 glue, see AntMatrixWeb.h

#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "AntMatrixWeb.h"
#include "AntMatrixPage.h"

#define AM_NVS_NS   "antmx"
#define AM_CFG_VER  1

static WebServer*  amSrv = nullptr;
static const char* amNav = nullptr;

//-------------------------------------------------------------------------------------------------------
bool amStoreLoad(AmConfig& cfg){
  Preferences p;
  if(!p.begin(AM_NVS_NS, true)) return false;   // namespace does not exist yet
  bool ok = p.getUChar("ver", 0) == AM_CFG_VER
         && p.getBytesLength("cfg") == sizeof(AmConfig)
         && p.getBytes("cfg", &cfg, sizeof(AmConfig)) == sizeof(AmConfig);
  p.end();
  const char* err;
  return ok && amValidate(cfg, &err);
}

bool amStoreSave(const AmConfig& cfg){
  Preferences p;
  if(!p.begin(AM_NVS_NS, false)) return false;
  bool ok = p.putBytes("cfg", &cfg, sizeof(AmConfig)) == sizeof(AmConfig);
  ok = ok && p.putUChar("ver", AM_CFG_VER) == 1;
  p.end();
  return ok;
}

//-------------------------------------------------------------------------------------------------------
static void amSendJson(int code, JsonDocument& doc){
  String out;
  serializeJson(doc, out);
  amSrv->sendHeader("Cache-Control", "no-cache");
  amSrv->send(code, "application/json", out);
}

static void amSendError(int code, const char* msg){
  JsonDocument d;
  d["error"] = msg;
  amSendJson(code, d);
}

static void amHandlePage(){
  amSrv->sendHeader("Cache-Control", "no-cache");
  amSrv->setContentLength(CONTENT_LENGTH_UNKNOWN);
  amSrv->send(200, "text/html", "");
  amSrv->sendContent_P(AM_PAGE_HEAD);
  if(amNav) amSrv->sendContent(amNav);
  amSrv->sendContent_P(AM_PAGE_BODY);
  amSrv->sendContent("");
}

static void amHandleCss(){
  amSrv->sendHeader("Cache-Control", "max-age=3600");
  amSrv->send_P(200, "text/css", AM_CSS);
}

static void amHandleState(){
  JsonDocument d;
  uint32_t now = millis();
  d["now"] = now;
  JsonArray a = d["t"].to<JsonArray>();
  for(uint8_t t=0; t<AM_TRX; t++){
    const AmTrx& s = amTrx(t);
    JsonObject o = a.add<JsonObject>();
    o["name"]    = amConfig().trxName[t];
    o["hz"]      = s.hz;
    o["age"]     = s.hz ? (long)(now - s.hzMs) : -1L;
    o["rows"]    = s.rows;
    o["pool"]    = s.pool;
    o["taken"]   = amTakenByOthers(t);
    o["want"]    = s.want;
    o["active"]  = s.active;
    o["manual"]  = s.manual;
    o["ext"]     = amExtName(s.ext);
    o["rel"]     = s.released;
    o["starved"] = s.starved;
    o["dis"]     = s.disabled;
  }
  amSendJson(200, d);
}

static void amHandleConfigGet(){
  AmConfig def;
  const AmConfig* c = &amConfig();
  if(amSrv->hasArg("defaults")){
    amLoadDefaults(def);
    c = &def;
  }
  JsonDocument d;
  d["trx"] = AM_TRX;
  JsonArray tn = d["trxName"].to<JsonArray>();
  for(uint8_t t=0; t<AM_TRX; t++) tn.add(c->trxName[t]);
  JsonArray out = d["out"].to<JsonArray>();
  for(uint8_t o=0; o<AM_OUT; o++){
    JsonObject j = out.add<JsonObject>();
    j["name"] = c->out[o].name;
    j["dis"]  = c->out[o].disabled;
    j["ext"]  = c->out[o].extConfirm;
    j["fb"]   = c->out[o].fallback;
  }
  JsonArray rows = d["rows"].to<JsonArray>();
  for(uint8_t i=0; i<AM_ROWS; i++){
    JsonObject j = rows.add<JsonObject>();
    j["fMin"] = c->rows[i].fMin;
    j["fMax"] = c->rows[i].fMax;
    j["code"] = c->rows[i].extCode;
    JsonArray m = j["mask"].to<JsonArray>();
    for(uint8_t t=0; t<AM_TRX; t++) m.add(c->rows[i].mask[t]);
  }
  amSendJson(200, d);
}

// read integer in range, false if missing or out of range
static bool amGetInt(JsonVariantConst v, long lo, long hi, long& out){
  if(!v.is<long>()) return false;
  out = v.as<long>();
  return out >= lo && out <= hi;
}

static bool amGetName(JsonVariantConst v, char* dst){
  const char* s = v | "";
  if(strlen(s) >= AM_NAME_LEN) return false;
  strcpy(dst, s);
  return true;
}

static void amHandleConfigPost(){
  JsonDocument d;
  if(deserializeJson(d, amSrv->arg("plain"))){
    amSendError(400, "bad json");
    return;
  }
  JsonArrayConst rows = d["rows"];
  JsonArrayConst out  = d["out"];
  JsonArrayConst tn   = d["trxName"];
  if(rows.size() != AM_ROWS || out.size() != AM_OUT || tn.size() != AM_TRX){
    amSendError(400, "wrong table size");
    return;
  }
  AmConfig c;
  memset(&c, 0, sizeof(c));
  long v;
  for(uint8_t i=0; i<AM_ROWS; i++){
    JsonObjectConst r = rows[i];
    if(!amGetInt(r["fMin"], 0, 10000000L, v)){ amSendError(400, "bad fMin"); return; }
    c.rows[i].fMin = v;
    if(!amGetInt(r["fMax"], 0, 10000000L, v)){ amSendError(400, "bad fMax"); return; }
    c.rows[i].fMax = v;
    if(!amGetInt(r["code"], -1, 255, v)){ amSendError(400, "bad ext code"); return; }
    c.rows[i].extCode = v;
    JsonArrayConst m = r["mask"];
    if(m.size() != AM_TRX){ amSendError(400, "bad mask"); return; }
    for(uint8_t t=0; t<AM_TRX; t++){
      if(!amGetInt(m[t], 0, 0xFFFF, v)){ amSendError(400, "bad mask"); return; }
      c.rows[i].mask[t] = v;
    }
  }
  for(uint8_t o=0; o<AM_OUT; o++){
    JsonObjectConst j = out[o];
    if(!amGetName(j["name"], c.out[o].name)){ amSendError(400, "output name too long"); return; }
    c.out[o].disabled   = j["dis"] | false;
    c.out[o].extConfirm = j["ext"] | false;
    if(!amGetInt(j["fb"], AM_NONE, AM_OUT-1, v)){ amSendError(400, "bad fallback"); return; }
    c.out[o].fallback = v;
  }
  for(uint8_t t=0; t<AM_TRX; t++){
    if(!amGetName(tn[t], c.trxName[t])){ amSendError(400, "TRX name too long"); return; }
  }
  const char* err;
  if(!amValidate(c, &err)){
    amSendError(400, err);
    return;
  }
  if(!amStoreSave(c)){
    amSendError(500, "NVS write failed");
    return;
  }
  amApplyConfig(c);
  JsonDocument ok;
  ok["ok"] = true;
  amSendJson(200, ok);
}

static void amHandleSelect(){
  JsonDocument d;
  if(deserializeJson(d, amSrv->arg("plain"))){
    amSendError(400, "bad json");
    return;
  }
  long trx, out;
  if(!amGetInt(d["trx"], 0, AM_TRX-1, trx) || !amGetInt(d["out"], AM_NONE, AM_OUT-1, out)){
    amSendError(400, "bad trx/out");
    return;
  }
  if(!amSelect(trx, out)){
    amSendError(409, "output not available");
    return;
  }
  JsonDocument ok;
  ok["ok"] = true;
  amSendJson(200, ok);
}

//-------------------------------------------------------------------------------------------------------
void amWebBegin(WebServer& srv, const char* navHtml){
  amSrv = &srv;
  amNav = navHtml;
  srv.on("/",               HTTP_GET,  amHandlePage);
  srv.on("/am.css",         HTTP_GET,  amHandleCss);
  srv.on("/api/am/state",   HTTP_GET,  amHandleState);
  srv.on("/api/am/config",  HTTP_GET,  amHandleConfigGet);
  srv.on("/api/am/config",  HTTP_POST, amHandleConfigPost);
  srv.on("/api/am/select",  HTTP_POST, amHandleSelect);
}
