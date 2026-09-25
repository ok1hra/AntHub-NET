// AntHub-NET /setup page (PROGMEM), styled by AntMatrix /am.css

#ifndef SETUP_PAGE_H
#define SETUP_PAGE_H

#include <pgmspace.h>

const char SETUP_PAGE[] PROGMEM = R"SP(<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>AntHub setup</title><link rel="stylesheet" href="/am.css"></head><body>
<header class="topbar"><nav class="tabs"><a class="tab" href="/">ANTENNAS</a><a class="tab tab-active" href="/setup">SETUP</a>
<span id="fw" class="topbar-fw"></span></nav></header>
<main class="shell"><section class="card">
  <div class="hdr"><div class="title">Setup</div>
    <div class="tbar"><span id="msg" class="muted"></span>
      <button id="bReboot" class="btn">Reboot</button><button id="bSave" class="btn pri">Save</button></div></div>
  <div class="sec">DEVICE</div>
  <div class="form">
    <label>Callsign</label><input type="text" id="call" maxlength="20" placeholder="empty = MAC address">
    <label>MAC / current IP</label><span id="info" class="muted"></span>
  </div>
  <div class="sec">NETWORK</div>
  <div class="form">
    <label>DHCP</label><span><input type="checkbox" id="dhcp"></span>
    <label>IP address</label><input type="text" id="ip">
    <label>Netmask</label><input type="text" id="mask">
    <label>Gateway</label><input type="text" id="gw">
    <label>DNS</label><input type="text" id="dns">
  </div>
  <div class="sec">TRXNET</div>
  <div class="form">
    <label>UDP port</label><input type="text" id="port">
    <label>Device ID (ANT.&lt;id&gt;)</label><input type="text" id="antId" maxlength="7">
    <label>Ext confirm device (DIN)</label><input type="text" id="din" maxlength="31">
    <label>Priority prefix 1</label><input type="text" id="prio0" maxlength="7">
    <label>Priority prefix 2</label><input type="text" id="prio1" maxlength="7">
  </div>
  <div class="muted" style="font-size:12px">Changes are applied after reboot.</div>
</section>
<section class="card">
  <div class="hdr"><div class="title">TrxNet devices</div><span class="muted" style="font-size:12px">frequency source &ndash; applied immediately</span></div>
  <div class="tbar" style="margin-bottom:10px;font-size:13px"><span class="muted">Inputs:</span>
    <label><input type="checkbox" id="en0"> TRX1 enabled</label>
    <label><input type="checkbox" id="en1"> TRX2 enabled</label></div>
  <div class="wrap"><table class="mx dev"><thead><tr><th>Device</th><th>IP:port</th><th>Seen</th><th>Last /hz</th><th>TRX1</th><th>TRX2</th></tr></thead>
  <tbody id="devs"></tbody></table></div>
  <div class="muted" style="font-size:12px;margin-top:8px">Devices announce themselves about every 30 s. /hz is sent only on frequency change.
  Click TRX1/TRX2 to use the device as frequency source, click again to clear.</div>
</section></main>
<script>
const $=id=>document.getElementById(id),F=['call','ip','mask','gw','dns','port','antId','din','prio0','prio1'];
const esc=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
function age(ms){if(ms===undefined)return'';const s=Math.round(ms/1000);return s<60?s+' s':s<3600?Math.round(s/60)+' min':Math.round(s/3600)+' h';}
let devKey='',dev=null;
async function peers(){
  try{dev=await (await fetch('/api/peers')).json();}catch(e){return;}
  dev.en.forEach((v,t)=>{const c=$('en'+t);if(document.activeElement!==c)c.checked=v;});
  const rows=dev.peers.slice();
  [dev.trx1,dev.trx2].forEach(n=>{if(n&&!rows.find(p=>p.name===n))rows.push({name:n,off:true});});
  const key=JSON.stringify([dev.trx1,dev.trx2,dev.din,rows.map(p=>[p.name,p.ip,p.port,p.hz,!!p.off])]);
  if(key!==devKey){
    devKey=key;
    $('devs').innerHTML=rows.map((p,i)=>{
      const b=t=>{const on=p.name===dev['trx'+(t+1)];return `<td><button class="tabb${on?' on':''}" data-t="${t}" data-n="${esc(p.name)}" data-on="${on?1:0}">TRX${t+1}</button></td>`;};
      return `<tr><td style="text-align:left"><b>${esc(p.name)}</b>${p.name===dev.din?' <span class="badge">DIN</span>':''}${p.off?' <span class="badge failed">offline</span>':''}</td>`+
        `<td>${p.off?'':esc(p.ip)+':'+p.port}</td><td id="sn${i}"></td>`+
        `<td>${p.hz!==undefined?(p.hz/1e6).toFixed(4)+' MHz <span class="muted" id="ha'+i+'"></span>':'&ndash;'}</td>${b(0)}${b(1)}</tr>`;
    }).join('')||'<tr><td colspan="6" class="muted">No device found yet</td></tr>';
  }
  rows.forEach((p,i)=>{const a=$('sn'+i),h=$('ha'+i);if(a)a.textContent=age(p.seen);if(h)h.textContent=age(p.hzAge)+' ago';});
}
$('devs').onclick=async e=>{
  const b=e.target.closest('button');if(!b)return;
  const t=+b.dataset.t,name=b.dataset.on==='1'?'':b.dataset.n;
  if(!name&&!confirm('Clear TRX'+(t+1)+' frequency source?'))return;
  const r=await fetch('/api/trxsrc',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({trx:t,name})});
  if(!r.ok){const j=await r.json().catch(()=>({}));alert(j.error||r.status);}
  devKey='';peers();
};
[0,1].forEach(t=>$('en'+t).onchange=async e=>{
  const en=e.target.checked;
  if(!en&&!confirm('Disable TRX'+(t+1)+'? Its antenna is switched off.')){e.target.checked=true;return;}
  const r=await fetch('/api/trxen',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({trx:t,en})});
  if(!r.ok){const j=await r.json().catch(()=>({}));alert(j.error||r.status);}
  peers();
});
setInterval(peers,2000);
function dhcpUi(){['ip','mask','gw','dns'].forEach(k=>$(k).disabled=$('dhcp').checked);}
async function load(){
  const j=await (await fetch('/api/setup')).json();
  F.forEach(k=>$(k).value=j[k]);$('dhcp').checked=j.dhcp;dhcpUi();
  $('info').textContent=j.mac+' / '+j.curIp;$('fw').textContent='FW '+j.fw;
}
$('dhcp').onchange=dhcpUi;
$('bSave').onclick=async()=>{
  const b={dhcp:$('dhcp').checked};F.forEach(k=>b[k]=$(k).value.trim());b.port=parseInt(b.port)||0;
  $('msg').textContent='Saving...';
  const r=await fetch('/api/setup',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(b)});
  const j=await r.json().catch(()=>({}));
  $('msg').textContent=r.ok?'Saved, reboot to apply':'Error: '+(j.error||r.status);
};
$('bReboot').onclick=async()=>{
  if(!confirm('Reboot device?'))return;
  await fetch('/api/reboot',{method:'POST'}).catch(()=>{});
  $('msg').textContent='Rebooting...';setTimeout(()=>location.reload(),8000);
};
load();peers();
</script>
</body></html>
)SP";

#endif
