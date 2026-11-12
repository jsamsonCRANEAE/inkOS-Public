import re, sys
T = open('/root/.apollo-kit/template.html').read()
head, rest = T.split('<div class="page" id="page">', 1)
script = rest[rest.index('<script>'):]
defs = re.search(r'<defs>.*?</defs>', T, re.S).group(0)
F = []
def box(x, y, w, h, lines, cls='ln'):
    s = f'<rect class="{cls}" x="{x}" y="{y}" width="{w}" height="{h}" rx="0.6"/>'
    n = len(lines); y0 = y + h/2 - (n-1)*1.6 + 0.8
    for i, t in enumerate(lines):
        s += f'<text class="mid sm" x="{x+w/2:.1f}" y="{y0+i*3.2:.1f}">{t}</text>'
    return s
def dia(cx, cy, w, h, lines):
    s = f'<path class="ln wht" d="M {cx-w/2} {cy} L {cx} {cy-h/2} L {cx+w/2} {cy} L {cx} {cy+h/2} Z"/>'
    n = len(lines); y0 = cy - (n-1)*1.5 + 0.75
    for i, t in enumerate(lines):
        s += f'<text class="mid sm" x="{cx}" y="{y0+i*3.0:.1f}">{t}</text>'
    return s
Y = 14
# main line
F.append(box(1, 7, 24, 14, ['PROOF LOAD', 'CROSSPOINT', 'RELEASE FILE']))
F.append('<line class="flow" x1="25" y1="14" x2="30" y2="14"/>')
F.append(dia(42, 14, 24, 14, ['SCREEN', 'CHANGED?']))
F.append('<line class="flow" x1="54" y1="14" x2="61" y2="14"/><text class="sm" x="55" y="12.6">YES</text>')
F.append(box(61, 7, 24, 14, ['LOAD INKOS', 'UPDATE.BIN']))
F.append('<line class="flow" x1="85" y1="14" x2="90" y2="14"/>')
F.append(dia(102, 14, 24, 14, ['"INKOS"', 'MENU?']))
F.append('<line class="flow" x1="114" y1="14" x2="121" y2="14"/><text class="sm" x="115" y="12.6">YES</text>')
F.append(box(121, 7, 24, 14, ['DIAGNOSTICS', 'PRESS ALL 6']))
F.append('<line class="flow" x1="145" y1="14" x2="150" y2="14"/>')
F.append(dia(162, 14, 24, 14, ['ALL IN', 'BAND?']))
F.append('<line class="flow" x1="174" y1="14" x2="174" y2="35"/><text class="sm" x="175.4" y="22">YES</text>')
F.append(box(166, 35, 17, 8, ['LOADED'], cls='hvy'))
# NO branches
def no(cx, x_box, lines, tagn):
    out = [f'<line class="flow" x1="{cx}" y1="21" x2="{cx}" y2="35"/>', f'<text class="sm" x="{cx+1.4}" y="28.4">NO</text>',
           box(cx-17, 35, 34, 16, lines, cls='hvy')]
    out.append(f'<circle class="ln wht" cx="{cx-17}" cy="35" r="2.2"/><text class="mid sm" x="{cx-17}" y="35.75">{tagn}</text>')
    return out
F += no(42, 0, ['STOP. RETRY ONCE,', 'THEN SUSPECT A', 'USB-LOCKED UNIT;', 'DO NOT LOAD INKOS'], 'A')
F += no(102, 0, ['ROLL BACK TO STOCK.', 'PHOTO OF SCREEN →', 'DISPLAY DRIVER', 'CHECK (DISPLAY.H)'], 'B')
F += no(146, 0, ['NOTE RAW COUNT.', 'EDIT BOARD.H ON', 'GITHUB.COM, WAIT', 'FOR BUILD, RELOAD'], 'C')
fig = f'<svg viewBox="0 0 183.9 53" xmlns="http://www.w3.org/2000/svg">{defs}{"".join(F)}</svg>'
# fix C branch: from diamond at 162 go down then left
fig = fig.replace('<line class="flow" x1="146" y1="21" x2="146" y2="35"/>', '<path class="ln" d="M 162 21 V 27.5 H 146"/><line class="flow" x1="146" y1="27.5" x2="146" y2="35"/>')
fig = fig.replace('<text class="sm" x="147.4" y="28.4">NO</text>', '<text class="sm" x="163.4" y="25.6">NO</text>')


def card(x, y):
    return f'<path class="hvy" d="M {x} {y} h 9 l 3 3 v 15 h -12 z"/><rect class="dots" x="{x+2}" y="{y+1.5}" width="6" height="3"/>'
def tree(x, y, title, rows, ok):
    o = [f'<text class="ttl" x="{x}" y="{y}">{title}</text>', card(x, y+3)]
    o.append(f'<text class="sm" x="{x+14}" y="{y+6.6}">MICROSD  (FAT32)</text>')
    for i, (name, note) in enumerate(rows):
        ry = y + 12 + i*5.2
        o.append(f'<path class="thin" d="M {x+16} {y+7.8} V {ry-0.8} H {x+19}"/>')
        o.append(f'<text x="{x+20}" y="{ry}">{name}</text>')
        if note: o.append(f'<text class="sm" x="{x+(62 if not ok[i] else 46)}" y="{ry}">{note}</text>')
        mark = '<path class="ln" d="M {a} {b} l 1 1.2 l 2.2 -2.8"/>' if ok[i] else '<path class="ln" d="M {a} {b} l 2.4 2.4 M {a} {c} l 2.4 -2.4"/>'
        o.append(mark.format(a=x+(57.5 if not ok[i] else 41.5), b=ry-2.2 if not ok[i] else ry-1.4, c=ry+0.2))
    return o
D = []
D += tree(2, 4, 'CORRECT', [('/UPDATE.BIN', '406 KB, AT THE TOP LEVEL')], [True])
D += tree(96, 4, 'INCORRECT', [('/DOWNLOADS/UPDATE.BIN', 'IN A FOLDER'), ('/UPDATE-1.BIN', 'RENAMED BY SAFARI'), ('/UPDATE.BIN.ZIP', 'STILL ZIPPED'), ('/UPDATE.BIN.TXT', 'WRONG EXTENSION')], [False]*4)
fig2 = f'<svg viewBox="0 0 183.9 33.5" xmlns="http://www.w3.org/2000/svg">{"".join(D)}</svg>'

page = f'''<div class="page" id="page">
  <div class="head">
    <div class="doc">IK4A-02-BLOCK I-(1)</div>
    <div class="book">INKOS OPERATIONS HANDBOOK</div>
    <div class="kind">SYSTEMS DATA</div>
  </div>
  <div class="sect">SECTION 2  ·  SUBSECTION 2.1 (CONTINUED)<br>SOFTWARE LOAD, MALFUNCTION AND ROLLBACK</div>

  <div class="para"><div class="num">2.1.4</div><div class="hd">Prerequisites.</div>
    <div class="body">A microSD card formatted FAT32, a USB-C card reader for the iPhone, a USB charger and the X4 charged. No full-flash backup can be made from an iPhone; the stock file in 2.1.6 is the only restore point, so keep a copy in Files before the first load.</div></div>

  <div class="para"><div class="num">2.1.5</div><div class="hd">Load Decision Sequence.</div>
    <div class="body">The proof load separates the route from the code: if the CrossPoint release file will not load, InkOS will not either, and the fault is the unit or the route (figure 2.1-3).</div></div>

  <figure>{fig}<figcaption>Figure 2.1-3.   Load Decision Sequence and Malfunction Branches</figcaption><div class="code">IKS-503A</div></figure>

  <div class="para sub chg"><div class="num">2.1.5.1</div><div class="hd">Unverified Elements.</div>
    <div class="body">The SD loader, the load keys and the SSD1677 controller come from community reports, not from this unit. Treat the first load as a test, not a routine.</div></div>

  <figure>{fig2}<figcaption>Figure 2.1-4.   Card Root, Correct and Incorrect Placement</figcaption><div class="code">IKS-504A</div></figure>

  <div class="band" id="band">
    <div>
      <div class="para" style="margin-bottom:1mm"><div class="num">2.1.6</div><div class="hd">Malfunction Data.</div></div>
      <table class="data">
        <caption>Table 2.1-2.  Load Malfunctions</caption>
        <tr><th>Symptom</th><th>Action</th></tr>
        <tr><td>No reaction to POWER + UP</td><td>Check name, root, FAT32, USB; retry once (branch A)</td></tr>
        <tr><td>Name ends in -1</td><td>Delete old; rename (fig 2.1-4)</td></tr>
        <tr><td>Blank screen</td><td>Roll back; photo (branch B)</td></tr>
        <tr><td>Button misreads</td><td>Note the raw count (branch C)</td></tr>
        <tr><td>Clock shows ~</td><td>Power was lost; set date and time</td></tr>
        <tr><td>GitHub run failed</td><td>LATEST holds the prior build</td></tr>
      </table>
    </div>
    <div>
      <div class="notice note"><div class="t">NOTE</div>POWER is not shown in DIAGNOSTICS. Verify it apart: a short press forces one full refresh.</div>
      <div class="card"><div class="ct"><span>ROLLBACK - STOCK</span><span>OPR</span></div>
        <ol>
          <li>SAFARI - CROSSPOINTREADER.COM/DEBUG</li>
          <li>STOCK FIRMWARE - DOWNLOAD</li>
          <li>FILES - RENAME UPDATE.BIN</li>
          <li>FILES - COPY TO CARD ROOT</li>
          <li>CARD IN, USB ON - POWER + UP HOLD</li>
          <li class="v">STOCK READER SCREEN</li>
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
      <span class="f">Page<u>2.1-2</u></span>
    </div>
  </div>
</div>
'''
head = head.replace('<title>[PAGE TITLE]</title>', '<title>InkOS Software Load, Malfunction</title>')
open(sys.argv[1], 'w').write(head + page + script)
