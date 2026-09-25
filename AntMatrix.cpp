// AntMatrix - band x output antenna switch logic, see AntMatrix.h

#include "AntMatrix.h"
#include <string.h>
#include <stdio.h>

static AmConfig cfg;
static AmTrx    st[AM_TRX];
static AmHooks  hk;

static inline uint16_t outBit(int8_t o){
  return (o >= 0 && o < AM_OUT) ? (uint16_t)(1u << o) : 0;
}

static int8_t highestOut(uint16_t m){
  for(int8_t o=AM_OUT-1; o>=0; o--){
    if(m & outBit(o)) return o;
  }
  return AM_NONE;
}

//-------------------------------------------------------------------------------------------------------
uint16_t amTakenByOthers(uint8_t trx){
  uint16_t m = 0;
  for(uint8_t t=0; t<AM_TRX; t++){
    if(t == trx) continue;
    m |= outBit(st[t].want) | outBit(st[t].active);
  }
  return m;
}

// band rows matching the frequency
static uint16_t matchRows(uint32_t hz){
  if(hz == 0) return 0;
  uint32_t khz = hz / 1000;
  uint16_t r = 0;
  for(uint8_t i=0; i<AM_ROWS; i++){
    if(cfg.rows[i].fMax != 0 && khz >= cfg.rows[i].fMin && khz <= cfg.rows[i].fMax) r |= (1u << i);
  }
  return r;
}

// ext code of output for TRX: first matching row enabling that output and having a code
static int16_t extCodeFor(uint8_t trx, uint16_t rows, int8_t out){
  for(uint8_t i=0; i<AM_ROWS; i++){
    if(((rows >> i) & 1) && (cfg.rows[i].mask[trx] & outBit(out)) && cfg.rows[i].extCode >= 0){
      return cfg.rows[i].extCode;
    }
  }
  return -1;
}

static uint16_t poolFor(uint8_t trx, uint16_t rows){
  uint16_t p = 0;
  for(uint8_t i=0; i<AM_ROWS; i++){
    if((rows >> i) & 1) p |= cfg.rows[i].mask[trx];
  }
  for(int8_t o=0; o<AM_OUT; o++){
    if(!(p & outBit(o))) continue;
    if(cfg.out[o].disabled || (cfg.out[o].extConfirm && extCodeFor(trx, rows, o) < 0)) p &= ~outBit(o);
  }
  return p;
}

// Recompute one TRX, returns true if its selection/state changed
static bool evaluate(uint8_t trx){
  AmTrx& s = st[trx];
  uint16_t rows = matchRows(s.hz);
  bool sameBand = (rows != 0 && rows == s.rows);
  s.rows = rows;
  s.pool = s.disabled ? 0 : poolFor(trx, rows);
  if(!sameBand) s.manual = AM_NONE;

  uint16_t taken = amTakenByOthers(trx);
  uint16_t avail = s.pool & ~taken;
  int8_t want = AM_NONE;
  if(!s.released){
    if(s.manual != AM_NONE && (avail & outBit(s.manual))){
      want = s.manual;
    }else{
      s.manual = AM_NONE;
      if(sameBand && (avail & outBit(s.want))){
        want = s.want;                // sticky inside the band
      }else{
        want = highestOut(avail);
      }
    }
  }

  // external confirmation
  int8_t  active  = want;
  uint8_t ext     = AM_EXT_NONE;
  int16_t code    = -1;
  bool    request = false;
  bool    cancel  = false;
  if(want != AM_NONE && cfg.out[want].extConfirm){
    code = extCodeFor(trx, rows, want);
    if(want == s.want && code == s.extCode && s.ext != AM_EXT_NONE){
      ext = s.ext;                    // same target, keep PENDING/OK/FAILED
    }else{
      ext = AM_EXT_PENDING;
      request = true;
    }
    if(ext != AM_EXT_OK){
      int8_t fb = cfg.out[want].fallback;
      bool fbOk = (fb >= 0 && fb < AM_OUT && fb != want && !cfg.out[fb].disabled && !(taken & outBit(fb)));
      active = fbOk ? fb : AM_NONE;
    }
  }else if(s.ext != AM_EXT_NONE){
    cancel = true;
  }

  bool starved = (!s.released && s.pool != 0 && want == AM_NONE);
  bool changed = (want != s.want || active != s.active || ext != s.ext || code != s.extCode || starved != s.starved);
  s.want    = want;
  s.active  = active;
  s.ext     = ext;
  s.extCode = code;
  s.starved = starved;
  if(cancel && hk.extCancel) hk.extCancel(trx);
  if(request && hk.extRequest) hk.extRequest(trx, (uint8_t)want, (uint8_t)code);
  return changed;
}

// Evaluate 'first' TRX before the others (first come, first served), then settle
static void update(int first){
  uint32_t dirty = 0;
  if(first >= 0 && evaluate(first)) dirty |= (1u << first);
  for(uint8_t pass=0; pass<=AM_TRX; pass++){
    bool any = false;
    for(uint8_t t=0; t<AM_TRX; t++){
      if(evaluate(t)){
        dirty |= (1u << t);
        any = true;
      }
    }
    if(!any) break;
  }
  if(dirty){
    int8_t a[AM_TRX];
    for(uint8_t t=0; t<AM_TRX; t++) a[t] = st[t].active;
    if(hk.writeOutputs) hk.writeOutputs(a);
    for(uint8_t t=0; t<AM_TRX; t++){
      if(((dirty >> t) & 1) && hk.changed) hk.changed(t);
    }
  }
}

//-------------------------------------------------------------------------------------------------------
void amLoadDefaults(AmConfig& c){
  // IARU region 1 bands, kHz
  static const uint32_t band[][2] = {
    {1810, 2000}, {3500, 3800}, {5351, 5367}, {7000, 7200}, {10100, 10150}, {14000, 14350},
    {18068, 18168}, {21000, 21450}, {24890, 24990}, {28000, 29700}, {50000, 52000}
  };
  memset(&c, 0, sizeof(c));
  for(uint8_t i=0; i<AM_ROWS; i++){
    if(i < sizeof(band)/sizeof(band[0])){
      c.rows[i].fMin = band[i][0];
      c.rows[i].fMax = band[i][1];
    }
    c.rows[i].extCode = -1;
  }
  for(uint8_t o=0; o<AM_OUT; o++){
    snprintf(c.out[o].name, AM_NAME_LEN, "ANT %d", o+1);
    c.out[o].fallback = AM_NONE;
  }
  for(uint8_t t=0; t<AM_TRX; t++){
    snprintf(c.trxName[t], AM_NAME_LEN, "TRX%d", t+1);
  }
  if(hk.defaults) hk.defaults(c);
}

bool amValidate(const AmConfig& c, const char** err){
  static char buf[48];
  for(uint8_t i=0; i<AM_ROWS; i++){
    if(c.rows[i].fMax != 0 && c.rows[i].fMin > c.rows[i].fMax){
      snprintf(buf, sizeof(buf), "row %d: fMin > fMax", i+1);
      *err = buf;
      return false;
    }
    if(c.rows[i].extCode < -1 || c.rows[i].extCode > 255){
      snprintf(buf, sizeof(buf), "row %d: ext code out of 0-255", i+1);
      *err = buf;
      return false;
    }
  }
  for(uint8_t o=0; o<AM_OUT; o++){
    int8_t fb = c.out[o].fallback;
    if(fb < AM_NONE || fb >= AM_OUT || fb == (int8_t)o){
      snprintf(buf, sizeof(buf), "output %d: bad fallback", o+1);
      *err = buf;
      return false;
    }
    if(!memchr(c.out[o].name, 0, AM_NAME_LEN)){
      snprintf(buf, sizeof(buf), "output %d: name too long", o+1);
      *err = buf;
      return false;
    }
  }
  for(uint8_t t=0; t<AM_TRX; t++){
    if(!memchr(c.trxName[t], 0, AM_NAME_LEN)){
      snprintf(buf, sizeof(buf), "TRX %d: name too long", t+1);
      *err = buf;
      return false;
    }
  }
  return true;
}

void amInit(const AmHooks& hooks){
  hk = hooks;
  memset(st, 0, sizeof(st));
  for(uint8_t t=0; t<AM_TRX; t++){
    st[t].want    = AM_NONE;
    st[t].active  = AM_NONE;
    st[t].manual  = AM_NONE;
    st[t].extCode = -1;
  }
  amLoadDefaults(cfg);
  int8_t a[AM_TRX];
  for(uint8_t t=0; t<AM_TRX; t++) a[t] = AM_NONE;
  if(hk.writeOutputs) hk.writeOutputs(a);
}

void amApplyConfig(const AmConfig& c){
  cfg = c;
  update(-1);
}

void amSetFreq(uint8_t trx, uint32_t hz){
  if(trx >= AM_TRX) return;
  AmTrx& s = st[trx];
  s.hzMs = hk.millis ? hk.millis() : 0;
  if(s.released){
    if(hz == s.hz) return;          // periodic repeat does not cancel release
    s.released = false;
  }
  bool hzChanged = (hz != s.hz);
  s.hz = hz;
  update(trx);
  if(hzChanged && hk.changed) hk.changed(trx);
}

bool amSelect(uint8_t trx, int8_t out){
  if(trx >= AM_TRX) return false;
  AmTrx& s = st[trx];
  if(out == AM_NONE){
    s.released = true;
    s.manual   = AM_NONE;
    update(trx);
    return true;
  }
  if(out < 0 || out >= AM_OUT || s.disabled) return false;
  if(!(s.pool & outBit(out)) || (amTakenByOthers(trx) & outBit(out))) return false;
  s.released = false;
  s.manual   = out;
  update(trx);
  return true;
}

void amSetEnabled(uint8_t trx, bool en){
  if(trx >= AM_TRX || st[trx].disabled == !en) return;
  st[trx].disabled = !en;
  st[trx].manual   = AM_NONE;
  update(trx);
  if(hk.changed) hk.changed(trx);
}

void amExtResult(uint8_t trx, bool ok){
  if(trx >= AM_TRX) return;
  AmTrx& s = st[trx];
  if(s.want == AM_NONE || s.ext == AM_EXT_NONE) return;
  s.ext = ok ? AM_EXT_OK : AM_EXT_FAILED;
  update(trx);
  if(hk.changed) hk.changed(trx);
}

const AmConfig& amConfig(){ return cfg; }
const AmTrx& amTrx(uint8_t trx){ return st[trx < AM_TRX ? trx : 0]; }

const char* amExtName(uint8_t ext){
  switch(ext){
    case AM_EXT_PENDING: return "pending";
    case AM_EXT_OK:      return "ok";
    case AM_EXT_FAILED:  return "failed";
    default:             return "none";
  }
}
