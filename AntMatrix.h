// AntMatrix - band x output antenna switch logic (portable, no Arduino/HW dependency)
//
// Model: AM_ROWS band rows (fMin..fMax kHz, inclusive) shared by all TRX, each row
// holds a 16-bit output mask per TRX. For a TRX frequency the pool is the OR of the
// masks of all matching rows minus disabled outputs. Selection rules:
//  - the highest free output of the pool wins (low numbers act as fallback, e.g. Dummy)
//  - an output held by another TRX is never taken (first come, first served)
//  - the selection is sticky while the band (set of matching rows) does not change
//  - manual choice (amSelect) is valid until the band changes
//  - after every change, other TRX without an antenna are re-evaluated
// Outputs with extConfirm (e.g. antenna behind a remote band switch) need the external
// device to confirm the row extCode first; meanwhile the fallback output is active.
//
// Host glue: fill AmHooks, call amInit(), amApplyConfig(), feed amSetFreq().

#ifndef ANTMATRIX_H
#define ANTMATRIX_H

#include <stdint.h>
#include <stdbool.h>

#ifndef AM_TRX
#define AM_TRX 2          // number of transceivers (inputs)
#endif
#define AM_OUT      16    // outputs (antennas), fits uint16_t mask
#define AM_ROWS     16    // band rows
#define AM_NAME_LEN 11    // 10 chars + NUL
#define AM_NONE     (-1)

enum AmExt : uint8_t { AM_EXT_NONE = 0, AM_EXT_PENDING, AM_EXT_OK, AM_EXT_FAILED };

struct AmRow {
  uint32_t fMin;           // kHz, inclusive
  uint32_t fMax;           // kHz, inclusive, 0 = unused row
  uint16_t mask[AM_TRX];   // allowed outputs per TRX, bit0 = output #1
  int16_t  extCode;        // code for extConfirm outputs, -1 = none (output unavailable)
};

struct AmOut {
  char   name[AM_NAME_LEN];
  bool   disabled;         // never switched on
  bool   extConfirm;       // needs external confirmation of row extCode
  int8_t fallback;         // output active while waiting for confirmation, -1 = none
};

struct AmConfig {
  AmRow rows[AM_ROWS];
  AmOut out[AM_OUT];
  char  trxName[AM_TRX][AM_NAME_LEN];
};

struct AmTrx {
  uint32_t hz;             // last frequency, 0 = unknown
  uint32_t hzMs;           // millis() of last frequency update
  uint16_t rows;           // matching rows (band identity)
  uint16_t pool;           // candidate outputs for current frequency
  int8_t   want;           // selected output (may wait for ext confirmation)
  int8_t   active;         // output really switched on
  int8_t   manual;         // manual choice, -1 = auto
  uint8_t  ext;            // AmExt state of want
  int16_t  extCode;        // code requested for want
  bool     released;       // user released the antenna, until a different freq arrives
  bool     starved;        // pool not empty but everything held by other TRX
  bool     disabled;       // TRX input disabled by host, never gets an antenna
};

struct AmHooks {
  void     (*writeOutputs)(const int8_t active[AM_TRX]);           // required
  void     (*extRequest)(uint8_t trx, uint8_t out, uint8_t code);  // ask external device
  void     (*extCancel)(uint8_t trx);                              // TRX left ext output
  void     (*changed)(uint8_t trx);                                // state of TRX changed
  uint32_t (*millis)();                                            // required
  void     (*defaults)(AmConfig& cfg);                             // optional host defaults
};

void            amInit(const AmHooks& hooks);          // loads defaults, all outputs off
void            amLoadDefaults(AmConfig& cfg);         // generic IARU rows + host hook
bool            amValidate(const AmConfig& cfg, const char** err);
void            amApplyConfig(const AmConfig& cfg);    // copy + re-evaluate all TRX
void            amSetFreq(uint8_t trx, uint32_t hz);
bool            amSelect(uint8_t trx, int8_t out);     // AM_NONE = release
void            amSetEnabled(uint8_t trx, bool en);    // disabled TRX keeps no antenna
void            amExtResult(uint8_t trx, bool ok);
const AmConfig& amConfig();
const AmTrx&    amTrx(uint8_t trx);
uint16_t        amTakenByOthers(uint8_t trx);          // outputs blocked for this TRX
const char*     amExtName(uint8_t ext);

#endif
