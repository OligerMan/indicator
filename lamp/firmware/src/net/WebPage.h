#pragma once
// Minimal phone-friendly control page. Every control sends a text command to
// /api/cmd, the same protocol as USB serial and the UART link.

static const char kWebPage[] = R"HTML(<!doctype html>
<html lang="ru"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Индикатор</title>
<style>
:root{--bg:#141218;--fg:#eee;--card:#221e29;--acc:#8a2be2}
body{margin:0;padding:16px;font-family:system-ui,sans-serif;background:var(--bg);color:var(--fg)}
h1{font-size:20px;margin:0 0 12px}
.card{background:var(--card);border-radius:12px;padding:12px;margin-bottom:12px}
button{background:#332d3d;color:var(--fg);border:0;border-radius:8px;padding:10px 12px;margin:4px;font-size:15px}
button.acc{background:var(--acc)}
input[type=range]{width:100%}
input[type=text]{width:100%;box-sizing:border-box;padding:8px;border-radius:8px;border:0;background:#332d3d;color:var(--fg)}
pre{white-space:pre-wrap;font-size:13px;margin:8px 0 0}
label{display:block;margin:6px 0 2px;font-size:14px;opacity:.8}
</style></head><body>
<h1>Индикатор</h1>
<div class="card" id="scenes"></div>
<div class="card">
<label>Яркость <span id="bv"></span></label>
<input type="range" id="bright" min="0" max="255" value="96">
<label>Лимит тока, мА <span id="lv"></span></label>
<input type="range" id="limit" min="500" max="16000" step="100" value="6000">
<label>Цвет</label>
<input type="color" id="color" value="#6d00cc">
</div>
<div class="card">
<button onclick="cmd('gesture single')">стук</button>
<button onclick="cmd('gesture double')">двойной</button>
<button onclick="cmd('gesture triple')">тройной (меню)</button>
</div>
<div class="card">
<input type="text" id="line" placeholder="команда, например: help" onkeydown="if(event.key=='Enter'){cmd(this.value);this.value=''}">
<pre id="out"></pre>
</div>
<script>
const out=document.getElementById('out');
async function cmd(c){const r=await fetch('/api/cmd',{method:'POST',body:c});out.textContent=await r.text();return out.textContent}
async function load(){
 const t=await (await fetch('/api/cmd',{method:'POST',body:'scenes'})).text();
 const box=document.getElementById('scenes');box.innerHTML='';
 t.trim().split('\n').forEach(l=>{const p=l.split(' ');const b=document.createElement('button');
  b.textContent=p[1];if(p[2]=='*')b.className='acc';b.onclick=async()=>{await cmd('scene '+p[1]);load()};box.appendChild(b)});
}
const br=document.getElementById('bright'),li=document.getElementById('limit');
br.oninput=()=>{document.getElementById('bv').textContent=br.value;cmd('bright '+br.value)};
li.oninput=()=>{document.getElementById('lv').textContent=li.value;cmd('limit '+li.value)};
document.getElementById('color').oninput=e=>{const v=e.target.value;
 cmd('color '+[1,3,5].map(i=>parseInt(v.substr(i,2),16)).join(' '))};
load();cmd('status');
</script></body></html>)HTML";
