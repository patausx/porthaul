#!/usr/bin/env python3
# PORTHAUL — accent color candidates (replacing orange == pornhub lol)
# Shows each candidate as a signal stripe + selection bar + progress on the earthy base.
from PIL import Image, ImageDraw, ImageFont

INK      = (0x18,0x1C,0x14)
SURFACE  = (0x24,0x28,0x22)
SURFACE2 = (0x3C,0x3D,0x37)
MOSS     = (0x69,0x75,0x65)
BONE     = (0xEC,0xDF,0xCC)

CANDIDATES = [
    ("SAGE / your pick",   (0xC2,0xD0,0x99)),
    ("SAGE brighter",      (0xAE,0xC9,0x4F)),
    ("SAGE deeper",        (0x9A,0xAD,0x6A)),
    ("LIME / chartreuse",  (0xB5,0xD3,0x35)),
    ("CYAN / electric",    (0x3D,0xD6,0xC4)),
    ("HAZARD YELLOW",      (0xE8,0xC5,0x47)),
]

def font(sz):
    for p in ["/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
              "/usr/share/fonts/TTF/DejaVuSansMono-Bold.ttf",
              "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"]:
        try: return ImageFont.truetype(p,sz)
        except: pass
    return ImageFont.load_default()
F=font(15); FS=font(11)

rowH=92; pad=14; W=720
H=pad+(rowH+pad)*len(CANDIDATES)+30
im=Image.new("RGB",(W,H),INK); d=ImageDraw.Draw(im)
d.text((pad,8),"PORTHAUL — pick a new accent (orange is out)",font=FS,fill=MOSS)

y=30
for name,acc in CANDIDATES:
    # panel
    d.rectangle([pad,y,W-pad,y+rowH],fill=SURFACE)
    # left signal stripe (full height) — the wordmark bar
    d.rectangle([pad,y,pad+6,y+rowH],fill=acc)
    # name
    d.text((pad+18,y+8),name,font=F,fill=BONE)
    d.text((pad+18,y+30),"#%02X%02X%02X"%acc,font=FS,fill=MOSS)
    # mock: active tab underline
    d.text((pad+18,y+50),"SERVER",font=FS,fill=BONE)
    tw=d.textlength("SERVER",font=FS)
    d.rectangle([pad+18,y+66,pad+18+tw,y+68],fill=acc)
    d.text((pad+90,y+50),"CLIENT",font=FS,fill=MOSS)
    d.text((pad+170,y+50),"CONNECT",font=FS,fill=MOSS)
    # mock: selection tick + row
    d.rectangle([320,y+50,323,y+62],fill=acc)
    d.text((330,y+50),"porthaul.3dsx   1.4M",font=FS,fill=BONE)
    # mock: progress bar
    pbx=320; pbw=W-pad-pbx-10; pby=y+70
    d.rectangle([pbx,pby,pbx+pbw,pby+12],fill=SURFACE2)
    d.rectangle([pbx,pby,pbx+int(pbw*0.62),pby+12],fill=acc)
    d.text((pbx,y+30),"TRANSFERRING",font=FS,fill=acc)
    y+=rowH+pad

im.save("brand/accent_candidates.png")
print("saved brand/accent_candidates.png")
