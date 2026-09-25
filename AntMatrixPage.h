// AntMatrix - web page (PROGMEM). Page = AM_PAGE_HEAD + host nav links + AM_PAGE_BODY.
// AM_CSS is served as /am.css and may be reused by host pages.

#ifndef ANTMATRIX_PAGE_H
#define ANTMATRIX_PAGE_H

#include <pgmspace.h>

const char AM_CSS[] PROGMEM = R"AM(
:root{color-scheme:dark;--bg:#000;--panel:#07090b;--panel2:#0d1116;--line:#1f2731;--text:#e7eef8;--muted:#91a2b9;
--blue:#3d9dff;--red:#ff453a;--green:#7eff66;--orange:#ffb347;--accent:#d5e8ff}
*{box-sizing:border-box}
body{margin:0;min-height:100vh;color:var(--text);font-family:Tahoma,Verdana,sans-serif;
background:radial-gradient(circle at top,rgba(40,86,146,.3),transparent 30%),linear-gradient(180deg,#06080b 0,var(--bg) 100%)}
.topbar{border-bottom:1px solid var(--line);background:rgba(5,8,12,.96);position:sticky;top:0;z-index:5}
.tabs{width:min(1320px,calc(100vw - 24px));margin:0 auto;display:flex;gap:6px;padding:10px 0;flex-wrap:wrap}
.tab{color:var(--muted);text-decoration:none;padding:8px 12px;border-radius:999px;border:1px solid transparent;font-weight:700;letter-spacing:.08em}
.tab-active{color:var(--accent);border-color:#2c4f77;background:rgba(46,100,165,.22)}
.topbar-fw{margin-left:auto;align-self:center;font-size:13px;color:var(--muted);letter-spacing:.06em;white-space:nowrap}
.shell{width:min(1320px,calc(100vw - 24px));margin:20px auto 40px}
.card{background:var(--panel2);border:1px solid var(--line);border-radius:16px;box-shadow:0 12px 35px rgba(0,0,0,.5);padding:16px 20px;margin-bottom:16px}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:16px}
.hdr{display:flex;align-items:center;justify-content:space-between;gap:10px;flex-wrap:wrap;margin-bottom:10px}
.title{font-size:18px;font-weight:700;color:var(--accent);letter-spacing:.04em}
.muted{color:var(--muted)}
.red{color:var(--red)}
.freq{font-size:34px;font-weight:700;font-variant-numeric:tabular-nums;color:#66b2ff}
.freq.old{color:var(--muted)}
.st{margin:6px 0 10px;font-size:16px;font-weight:700}
.badge{display:inline-block;margin-left:6px;padding:2px 8px;border-radius:999px;font-size:11px;letter-spacing:.06em;border:1px solid var(--line)}
.badge.pending{color:var(--orange);border-color:#7a5a1f}
.badge.ok{color:var(--green);border-color:#2f7a2f}
.badge.failed{color:var(--red);border-color:#7a1f1f}
.badge.man{color:var(--blue);border-color:#2c4f77}
.pool{display:flex;flex-wrap:wrap;gap:6px;margin-bottom:12px}
.pb{padding:6px 10px;border-radius:8px;border:1px solid var(--line);background:var(--panel);color:var(--text);cursor:pointer;font-weight:700;font-size:13px}
.pb.act{background:#003a00;border-color:#2f7a2f;color:var(--green)}
.pb.wait{background:#3a2a00;border-color:#7a5a1f;color:var(--orange)}
.pb.tkn{background:#2a0000;color:#aa5555;cursor:not-allowed}
.btn{padding:5px 14px;border-radius:8px;border:1px solid var(--line);background:var(--panel2);color:var(--muted);font-weight:700;cursor:pointer;font-size:13px}
.btn.pri{background:#12345a;border-color:#2c4f77;color:var(--accent)}
.btn:disabled{opacity:.4;cursor:default}
.tbar{display:flex;align-items:center;gap:8px;flex-wrap:wrap}
.tabb{padding:5px 12px;border-radius:999px;border:1px solid var(--line);background:var(--panel);color:var(--muted);font-weight:700;cursor:pointer}
.tabb.on{color:var(--accent);border-color:#2c4f77;background:rgba(46,100,165,.22)}
input,select{background:var(--panel);color:var(--text);border:1px solid var(--line);border-radius:4px;padding:3px 5px;font-size:13px}
input.bad{border-color:var(--red);color:var(--red)}
input:disabled{opacity:.4}
.wrap{overflow-x:auto}
table.mx{border-collapse:collapse;font-size:12px}
.mx th,.mx td{border:1px solid var(--line);padding:3px 3px;text-align:center;white-space:nowrap}
.mx th{background:var(--panel);color:var(--muted);font-weight:700}
.mx th.lbl{text-align:right}
.mx input,.mx select{font-size:12px;padding:2px 3px}
.mx input[type=number]{width:68px;text-align:right}
.mx input.nm{width:58px}
.mx select{width:46px}
.mx input.code{width:34px;text-align:center;text-transform:uppercase}
.mx input[type=checkbox]{accent-color:var(--blue)}
.mx tr.hit td{background:#003a00}
.mx tr.hit input{background:transparent;color:var(--green)}
.mx td.cA{background:#3a0000 !important}
.mx td.cW{background:#3a2a00 !important}
.oh.act{background:#3a0000 !important;color:#ff6666 !important}
.oh.wait{background:#3a2a00 !important;color:var(--orange) !important}
.oh.tkn{box-shadow:inset 0 -3px 0 var(--blue)}
.oh.dis{text-decoration:line-through;opacity:.5}
.legend{font-size:12px;color:var(--muted);margin-top:10px;line-height:1.6}
.sw{display:inline-block;width:10px;height:10px;border-radius:2px;margin:0 4px 0 12px;vertical-align:middle}
.form{display:grid;grid-template-columns:200px 1fr;gap:8px 12px;align-items:center;margin-bottom:12px}
.form label{color:var(--muted);font-size:13px;text-align:right}
.form span{font-size:13px}
.form input[type=text]{width:180px}
.sec{font-size:13px;font-weight:700;color:var(--accent);letter-spacing:.08em;margin:14px 0 8px}
@media (max-width:600px){.freq{font-size:26px}.form{grid-template-columns:1fr}.form label{text-align:left}}
)AM";

const char AM_PAGE_HEAD[] PROGMEM = R"AM(<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Antenna switch</title><link rel="stylesheet" href="/am.css"></head><body>
<header class="topbar"><nav class="tabs"><a class="tab tab-active" href="/">ANTENNAS</a>
)AM";

const char AM_PAGE_BODY[] PROGMEM = R"AM(<span id="conn" class="topbar-fw"></span></nav></header>
<main class="shell">
<section id="cards" class="cards"></section>
<section class="card">
  <div class="hdr">
    <div class="title">Band &times; Output</div>
    <div class="tbar"><span id="msg" class="muted"></span>
      <button id="bDef" class="btn">Defaults</button><button id="bSave" class="btn pri" disabled>Save</button></div>
  </div>
  <div class="tbar" id="tabs" style="margin-bottom:10px"></div>
  <div class="wrap"><table class="mx" id="mx"></table></div>
  <div class="legend">
    <span class="sw" style="background:#003a00;margin-left:0"></span>band matching TRX frequency
    <span class="sw" style="background:#3a0000"></span>active output
    <span class="sw" style="background:#3a2a00"></span>waiting for external confirmation
    <span class="sw" style="background:var(--blue)"></span>held by another TRX<br>
    Rows are shared by all TRX, checkboxes are per TRX (tabs). The highest free output wins, lower ones are backup.
    Ext code (hex) is sent to the external device for outputs with <i>Ext confirm</i>; empty = output not usable in that band.
  </div>
</section>
</main>
<script>
const N=16,$=id=>document.getElementById(id);
let cfg=null,saved=null,st=null,tab=0,dirty=false,busy=false;
const esc=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
const bit=(m,o)=>((m>>o)&1)===1;
const hex=n=>n<0?'':n.toString(16).toUpperCase().padStart(2,'0');
const clone=o=>JSON.parse(JSON.stringify(o));
function outLbl(o){return o<0?'off':'#'+(o+1)+' '+esc(saved?saved.out[o].name:'');}
function fmtHz(hz){return hz?(hz/1e6).toFixed(4)+' MHz':'&mdash;';}
function fmtAge(ms){if(ms<0)return'';const s=Math.round(ms/1000);if(s<60)return s+' s ago';if(s<3600)return Math.round(s/60)+' min ago';return Math.round(s/3600)+' h ago';}
function setDirty(v){dirty=v;$('bSave').disabled=!v;$('msg').textContent=v?'Unsaved changes':'';}

async function loadCfg(def){
  const r=await fetch('/api/am/config'+(def?'?defaults=1':''));
  if(!r.ok){$('msg').textContent='Config load failed';return;}
  const c=await r.json();
  if(def){c.trxName=cfg.trxName;}
  cfg=c;if(!def)saved=clone(c);
  render();setDirty(!!def);if(def)$('msg').textContent='Defaults loaded, press Save to apply';
}

function render(){
  let h='<thead><tr><th>fMin kHz</th><th>fMax kHz</th><th title="Code for external device (hex)">Ext</th>';
  for(let o=0;o<N;o++)h+=`<th id="h${o}" class="oh">${o+1}</th>`;
  h+='</tr><tr><th colspan="3" class="lbl">Name</th>';
  for(let o=0;o<N;o++)h+=`<td><input class="nm" maxlength="10" data-k="name" data-o="${o}" value="${esc(cfg.out[o].name)}"></td>`;
  h+='</tr><tr><th colspan="3" class="lbl" title="Output is never switched on">Disabled</th>';
  for(let o=0;o<N;o++)h+=`<td><input type="checkbox" data-k="dis" data-o="${o}"${cfg.out[o].dis?' checked':''}></td>`;
  h+='</tr><tr><th colspan="3" class="lbl" title="External device must confirm the row ext code before the output is switched">Ext confirm</th>';
  for(let o=0;o<N;o++)h+=`<td><input type="checkbox" data-k="ext" data-o="${o}"${cfg.out[o].ext?' checked':''}></td>`;
  h+='</tr><tr><th colspan="3" class="lbl" title="Output active while waiting for confirmation">Fallback</th>';
  for(let o=0;o<N;o++){
    let s=`<select data-k="fb" data-o="${o}"><option value="-1">&ndash;</option>`;
    for(let f=0;f<N;f++)if(f!==o)s+=`<option value="${f}"${cfg.out[o].fb===f?' selected':''}>${f+1}</option>`;
    h+=`<td>${s}</select></td>`;
  }
  h+='</tr></thead><tbody>';
  cfg.rows.forEach((r,i)=>{
    h+=`<tr id="r${i}"><td><input type="number" min="0" data-k="fMin" data-i="${i}" value="${r.fMin}"></td>`+
       `<td><input type="number" min="0" data-k="fMax" data-i="${i}" value="${r.fMax}"></td>`+
       `<td><input class="code" maxlength="2" placeholder="&ndash;" data-k="code" data-i="${i}" value="${hex(r.code)}"></td>`;
    for(let o=0;o<N;o++)h+=`<td class="c${o}"><input type="checkbox" data-k="m" data-i="${i}" data-o="${o}"${bit(r.mask[tab],o)?' checked':''}></td>`;
    h+='</tr>';
  });
  $('mx').innerHTML=h+'</tbody>';
  renderTabs();check();highlight();
}

function renderTabs(){
  let h='';
  for(let t=0;t<cfg.trx;t++)h+=`<button class="tabb${t===tab?' on':''}" data-t="${t}">${esc(cfg.trxName[t])}</button>`;
  h+=`<label class="muted">TRX name <input id="tn" maxlength="10" value="${esc(cfg.trxName[tab])}"></label>`;
  $('tabs').innerHTML=h;
}

// mark invalid fields, returns true when all fine
function check(){
  let ok=true;
  cfg.rows.forEach((r,i)=>{
    const bad=r.fMax!==0&&r.fMin>r.fMax;
    document.querySelectorAll(`#r${i} input[type=number]`).forEach(e=>e.classList.toggle('bad',bad));
    const c=document.querySelector(`#r${i} input.code`),v=c.value.trim(),cb=v!==''&&!/^[0-9a-f]{1,2}$/i.test(v);
    c.classList.toggle('bad',cb);
    if(bad||cb)ok=false;
  });
  return ok;
}

$('mx').addEventListener('input',e=>{
  const el=e.target,k=el.dataset.k;if(!k)return;
  const i=+el.dataset.i,o=+el.dataset.o;
  if(k==='name')cfg.out[o].name=el.value;
  else if(k==='dis'||k==='ext')cfg.out[o][k]=el.checked;
  else if(k==='fb')cfg.out[o].fb=+el.value;
  else if(k==='fMin'||k==='fMax')cfg.rows[i][k]=parseInt(el.value)||0;
  else if(k==='code'){const v=el.value.trim();cfg.rows[i].code=/^[0-9a-f]{1,2}$/i.test(v)?parseInt(v,16):-1;}
  else if(k==='m'){if(el.checked)cfg.rows[i].mask[tab]|=1<<o;else cfg.rows[i].mask[tab]&=~(1<<o);}
  setDirty(true);check();highlight();
});

$('tabs').addEventListener('click',e=>{const t=e.target.dataset.t;if(t!==undefined){tab=+t;render();}});
$('tabs').addEventListener('input',e=>{
  if(e.target.id!=='tn')return;
  cfg.trxName[tab]=e.target.value;
  $('tabs').querySelector(`[data-t="${tab}"]`).textContent=e.target.value;setDirty(true);
});

$('bSave').onclick=async()=>{
  if(!check()){$('msg').textContent='Fix the red fields first';return;}
  $('bSave').disabled=true;$('msg').textContent='Saving...';
  try{
    const r=await fetch('/api/am/config',{method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({trxName:cfg.trxName,out:cfg.out,rows:cfg.rows})});
    const j=await r.json().catch(()=>({}));
    if(r.ok){saved=clone(cfg);setDirty(false);$('msg').textContent='Saved';poll();}
    else{$('msg').textContent='Error: '+(j.error||r.status);$('bSave').disabled=false;}
  }catch(e){$('msg').textContent='Save failed';$('bSave').disabled=false;}
};
$('bDef').onclick=()=>{if(confirm('Load default table into the form? Nothing changes until Save.'))loadCfg(true);};

let cardsKey='';
function renderCards(){
  const key=JSON.stringify(st.t.map(s=>[s.name,s.hz,s.rows,s.pool,s.taken,s.want,s.active,s.manual,s.ext,s.rel,s.starved,s.dis,s.age>60000]))+(saved?JSON.stringify(saved.out):'');
  if(key!==cardsKey){cardsKey=key;buildCards();}
  st.t.forEach((s,t)=>{const a=$('age'+t);if(a)a.textContent=fmtAge(s.age);});
}
function buildCards(){
  let h='';
  st.t.forEach((s,t)=>{
    let txt,cls='';
    if(s.dis){txt='Disabled';cls='muted';}
    else if(s.rel){txt='Released';cls='muted';}
    else if(!s.hz){txt='No frequency';cls='muted';}
    else if(s.starved){txt='Collision &ndash; no free antenna';cls='red';}
    else if(s.want<0){txt='No antenna for this frequency';cls='muted';}
    else{
      txt=outLbl(s.want);
      if(s.ext==='pending')txt+='<span class="badge pending">waiting for confirmation</span>';
      if(s.ext==='ok')txt+='<span class="badge ok">confirmed</span>';
      if(s.ext==='failed')txt+='<span class="badge failed">confirmation failed</span>';
      if(s.active!==s.want)txt+=`<div class="muted" style="font-size:13px">now on ${outLbl(s.active)}</div>`;
    }
    if(s.manual>=0)txt+='<span class="badge man">manual</span>';
    let pb='';
    for(let o=0;o<N;o++){
      if(!bit(s.pool,o))continue;
      const tk=bit(s.taken,o),c=o===s.active?' act':o===s.want?' wait':tk?' tkn':'';
      pb+=`<button class="pb${c}" data-t="${t}" data-o="${o}"${tk?' disabled title="held by another TRX"':''}>${outLbl(o)}</button>`;
    }
    const old=s.age>60000||s.dis;
    h+=`<div class="card"><div class="hdr"><span class="title">${esc(s.name)}</span><span class="muted" id="age${t}"></span></div>`+
       `<div class="freq${old?' old':''}">${fmtHz(s.hz)}</div><div class="st ${cls}">${txt}</div>`+
       (s.dis?'</div>':`<div class="pool">${pb||'<span class="muted">No antenna allowed for this frequency</span>'}</div>`+
       `<button class="btn" data-rel="${t}"${s.rel||!s.hz?' disabled':''}>Release</button></div>`);
  });
  $('cards').innerHTML=h;
}

async function select(t,o){
  const r=await fetch('/api/am/select',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({trx:t,out:o})});
  if(!r.ok){const j=await r.json().catch(()=>({}));alert(j.error||('Error '+r.status));}
  poll();
}
$('cards').addEventListener('click',e=>{
  const b=e.target.closest('button');if(!b||b.disabled)return;
  if(b.dataset.rel!==undefined)select(+b.dataset.rel,-1);
  else if(b.dataset.o!==undefined)select(+b.dataset.t,+b.dataset.o);
});

function highlight(){
  if(!cfg||!st||!st.t[tab])return;
  const s=st.t[tab];
  document.querySelectorAll('#mx td.cA,#mx td.cW').forEach(e=>e.classList.remove('cA','cW'));
  for(let i=0;i<cfg.rows.length;i++){
    const tr=$('r'+i),hit=bit(s.rows,i);tr.classList.toggle('hit',hit);
    if(hit&&s.active>=0)tr.querySelector('.c'+s.active).classList.add('cA');
    if(hit&&s.want>=0&&s.want!==s.active)tr.querySelector('.c'+s.want).classList.add('cW');
  }
  for(let o=0;o<N;o++){
    $('h'+o).className='oh'+(o===s.active?' act':o===s.want?' wait':'')+(bit(s.taken,o)?' tkn':'')+(cfg.out[o].dis?' dis':'');
  }
}

async function poll(){
  if(busy)return;busy=true;
  try{const r=await fetch('/api/am/state');st=await r.json();$('conn').textContent='';renderCards();highlight();}
  catch(e){$('conn').textContent='offline';}
  busy=false;
}
window.addEventListener('beforeunload',e=>{if(dirty){e.preventDefault();e.returnValue='';}});
loadCfg(false).then(poll);
setInterval(poll,1000);
</script>
</body></html>
)AM";

#endif
