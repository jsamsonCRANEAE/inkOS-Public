# Generates the InkOS SD-load handbook page from the kit template.
import re, sys
T = open('/root/.apollo-kit/template.html').read()
head, rest = T.split('<div class="page" id="page">', 1)
script = rest[rest.index('<script>'):]
defs = re.search(r'<defs>.*?</defs>', T, re.S).group(0)

# ---------- HERO: load path block diagram ----------
def box(x, y, w, h, l1, l2=None, cls='ln'):
    s = f'<rect class="{cls}" x="{x}" y="{y}" width="{w}" height="{h}" rx="0.6"/>'
    cy = y + h/2 + (0.8 if l2 is None else -0.7)
    s += f'<text class="mid" x="{x+w/2}" y="{cy:.1f}">{l1}</text>'
    if l2: s += f'<text class="mid sm" x="{x+w/2}" y="{cy+3.2:.1f}">{l2}</text>'
    return s
def tag(x, y, n, tx, ty):
    return (f'<line class="thin" x1="{x}" y1="{y+2.2}" x2="{tx}" y2="{ty}"/>'
            f'<circle class="ln wht" cx="{x}" cy="{y}" r="2.2"/><text class="mid sm" x="{x}" y="{y+0.75}">{n}</text>')
H = []
Y = 14; BH = 12
H.append(box(2, Y, 28, BH, 'GITHUB ACTIONS', 'CLOUD BUILD'))
H.append('<line class="flow" x1="30" y1="20" x2="38" y2="20"/>')
H.append(box(38, Y, 26, BH, 'IPHONE', 'SAFARI → FILES'))
H.append('<line class="flow" x1="64" y1="20" x2="72" y2="20"/>')
H.append(box(72, Y, 22, BH, 'READER', 'USB-C, MICROSD'))
H.append('<line class="flow" x1="94" y1="20" x2="103.6" y2="20"/>')
# X4 enclosure
H.append('<rect class="hvy" x="104" y="4" width="78" height="40" rx="1.2"/>')
H.append('<text class="ttl" x="106.5" y="8.6">X4  (ESP32-C3)</text>')
H.append(box(108, Y, 18, BH, 'SD SLOT', '/UPDATE.BIN'))
H.append('<line class="flow" x1="126" y1="20" x2="131" y2="20"/>')
H.append(box(131, Y, 22, BH, 'BOOTLOADER', 'SD LOADER *'))
H.append('<line class="flow" x1="153" y1="20" x2="158" y2="20"/>')
H.append(box(158, Y, 21, BH, 'APP SLOT', '', cls='dash'))
H.append('<line class="flow" x1="168.5" y1="26" x2="168.5" y2="31"/>')
H.append(box(158, 31, 21, 10, 'E-PAPER', '800 × 480'))
# controls into bootloader
H.append('<line class="flow" x1="138" y1="34.5" x2="138" y2="26.2"/>')
H.append('<rect class="ln" x="128" y="34.5" width="20" height="7" rx="0.6"/><text class="mid sm" x="138" y="38.9">POWER + UP</text>')
H.append('<line class="flow" x1="117" y1="49.5" x2="117" y2="44.2"/>')
H.append('<text class="sm" x="118.6" y="48.8">USB 5 V, CONNECTED</text>')
# rollback dashed path
# station tags
H.append(tag(16, 5, 1, 16, Y))
H.append(tag(51, 5, 2, 51, Y))
H.append(tag(83, 5, 3, 83, Y))
H.append('<line class="thin" x1="126.4" y1="38" x2="128" y2="38"/>')
H.append(tag(124.2, 38, 4, 124.2, 38))
H.append('<line class="thin" x1="155.6" y1="36" x2="158" y2="36"/>')
H.append(tag(153.2, 36, 5, 153.2, 36))
# station data legend
L = ['STATION DATA:', '1  UPDATE.BIN, 406 KB (415,616 B)', '2  FILES › ON MY IPHONE', '3  ROOT OF CARD, NO FOLDER', '4  HOLD POWER + UP', '5  "INKOS" HOME MENU']
for i, s in enumerate(L):
    H.append(f'<text class="sm" x="2" y="{30 + i*3.1:.1f}">{s}</text>')
H.append('<text class="sm" x="52" y="46.6">* SD LOADER: COMMUNITY-REPORTED,</text><text class="sm" x="52" y="49.6">  UNVERIFIED ON THIS UNIT</text>')
H.append('<text class="sm" x="52" y="33.1">ROLLBACK (PAGE 2.1-2):</text><text class="sm" x="52" y="36.2">STOCK FILE, CROSSPOINT-</text><text class="sm" x="52" y="39.3">READER.COM/DEBUG, RENAMED</text><text class="sm" x="52" y="42.4">UPDATE.BIN, SAME ROUTE</text>')
hero = f'<svg viewBox="0 0 183.9 50" xmlns="http://www.w3.org/2000/svg">{defs}{"".join(H)}</svg>'

# ---------- SECONDARY: ADC decode bands (exact board.h values) ----------
X0, X1 = 24, 176
sx = lambda c: X0 + (X1 - X0) * c / 4095
S = []
def strip(y, label, bands, marks):
    out = [f'<text class="end" x="{X0-2}" y="{y+3.9}">{label}</text>',
           f'<rect class="ln" x="{X0}" y="{y}" width="{X1-X0}" height="5.6"/>']
    for lo, hi, name, fill in bands:
        a, b = sx(lo), sx(hi)
        if fill: out.append(f'<rect class="hatch" x="{a:.2f}" y="{y}" width="{b-a:.2f}" height="5.6"/>')
        if lo > 0: out.append(f'<line class="ln" x1="{a:.2f}" y1="{y}" x2="{a:.2f}" y2="{y+5.6}"/>')
        mx = (a + b) / 2; w = len(name) * 1.75 + 1.6
        out.append(f'<rect class="wht" x="{mx-w/2:.2f}" y="{y+1.3}" width="{w:.2f}" height="3.2"/>')
        out.append(f'<text class="mid sm" x="{mx:.2f}" y="{y+3.75}">{name}</text>')
    for c in marks:
        x = sx(c); out.append(f'<path class="blk" d="M {x:.2f} {y-0.3} l -1 -1.7 h 2 z"/>')
    return out
S += strip(5, 'GPIO 1', [(0, 740, 'RIGHT', 0), (740, 2060, 'LEFT', 1), (2060, 3060, 'CONFIRM', 0), (3060, 3800, 'BACK', 1), (3800, 4095, 'IDLE', 0)], [3, 1470, 2655, 3470])
S += strip(17, 'GPIO 2', [(0, 1100, 'DOWN', 0), (1100, 3150, 'UP', 1), (3150, 4095, 'IDLE', 0)], [3, 2205])
# axis
S.append(f'<line class="ln" x1="{X0}" y1="26" x2="{X1}" y2="26"/>')
for c in [0, 740, 1100, 2060, 3060, 3150, 3800, 4095]:
    x = sx(c); S.append(f'<line class="thin" x1="{x:.2f}" y1="26" x2="{x:.2f}" y2="27.4"/>')
for c, dy in [(0,0),(740,0),(1100,2.6),(2060,0),(3060,0),(3150,2.6),(3800,0),(4095,0)]:
    x = sx(c); S.append(f'<text class="mid sm" x="{x:.2f}" y="{29.9+dy:.1f}">{c}</text>')
S.append(f'<text class="end sm" x="{X1}" y="35.2">RAW ADC COUNTS, 12-BIT, 11 DB ATTENUATION</text>')
S.append(f'<path class="blk" d="M {X0+0.5} 35.1 l -1 -1.7 h 2 z"/><text class="sm" x="{X0+2}" y="35.2">NOMINAL READING (COMMUNITY MEASUREMENT)</text>')
sec = f'<svg viewBox="0 0 183.9 36" xmlns="http://www.w3.org/2000/svg">{"".join(S)}</svg>'

page = f'''<div class="page" id="page">
  <div class="head">
    <div class="doc">IK4A-02-BLOCK I-(1)</div>
    <div class="book">INKOS OPERATIONS HANDBOOK</div>
    <div class="kind">SYSTEMS DATA</div>
  </div>
  <div class="sect">SECTION 2  ·  SUBSECTION 2.1<br>SOFTWARE LOAD, SD CARD ROUTE</div>

  <div class="para"><div class="num">2.1.1</div><div class="hd">Introduction.</div>
    <div class="body">InkOS loads onto the X4 with an iPhone only: the cloud builds the image, the phone carries it to the microSD card, and the bootloader is reported to write it. The file must be named update.bin, in the card root (figure 2.1-1).</div></div>

  <figure>{hero}<figcaption>Figure 2.1-1.   Software Load Path, Cloud to App Slot</figcaption><div class="code">IKS-501A</div></figure>

  <div class="para"><div class="num">2.1.2</div><div class="hd">Functional Description.</div>
    <div class="body">A passing build posts update.bin to the release LATEST. Prove the route first with the CrossPoint release file, then load InkOS the same way; any failure goes to page 2.1-2. On first boot, SETTINGS › DIAGNOSTICS shows each raw reading (figure 2.1-2).</div></div>
  <div class="para sub chg"><div class="num">2.1.2.1</div><div class="hd">Button Decode Bands.</div>
    <div class="body">Six buttons share two resistor ladders on GPIO 1 and 2. A reading outside its band is fixed in BOARD.H, not in hardware.</div></div>

  <figure>{sec}<figcaption>Figure 2.1-2.   Button Decode Bands, Firmware v0.1</figcaption><div class="code">IKS-502A</div></figure>

  <div class="band" id="band">
    <div>
      <div class="para" style="margin-bottom:1mm"><div class="num">2.1.3</div><div class="hd">Load Data.</div></div>
      <table class="data">
        <caption>Table 2.1-1.  InkOS v0.1 Load Data</caption>
        <tr><th>Parameter</th><th>Value</th></tr>
        <tr><td>Image file</td><td>UPDATE.BIN, 406 KB</td></tr>
        <tr><td>Partition used</td><td>30.1 % of 1.25 MB build</td></tr>
        <tr><td>RAM at build</td><td>62 KB of 320 KB</td></tr>
        <tr><td>Load keys</td><td>POWER + UP, USB on</td></tr>
        <tr><td>Display</td><td>800 × 480, SSD1677 assumed</td></tr>
        <tr><td>Button poll</td><td>5 ms / 20 ms idle</td></tr>
      </table>
    </div>
    <div>
      <div class="notice"><div class="t">CAUTION</div>Keep USB power on until the load ends; a cut can corrupt the app slot.</div>
      <div class="card"><div class="ct"><span>LOAD - INKOS</span><span>OPR</span></div>
        <ol>
          <li>FILES - UPDATE.BIN TO CARD ROOT</li>
          <li class="v">SIZE - 406 KB, NO "-1" IN NAME</li>
          <li>CARD IN, USB ON - POWER + UP HOLD</li>
          <li class="v">HOME MENU - "INKOS"</li>
          <li>DIAGNOSTICS - PRESS ALL 6</li>
          <li class="v">COUNTS - IN BAND (FIG 2.1-2)</li>
        </ol></div>
    </div>
  </div>

  <div class="tab" style="top: 112mm; height: 22mm;">LOAD</div>
  <div class="foot" id="foot">
    <div class="rule">SOFTWARE LOAD</div>
    <div class="row">
      <span class="f">Mission<u>PERSONAL</u></span>
      <span class="f">Basic Date<u>12 November 2026</u></span>
      <span class="f">Change Date<u>-</u></span>
      <span class="f">Page<u>2.1-1</u></span>
    </div>
  </div>
</div>
'''
head = head.replace('<title>[PAGE TITLE]</title>', '<title>InkOS Software Load</title>')
open(sys.argv[1], 'w').write(head + page + script)
