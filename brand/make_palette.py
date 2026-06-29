#!/usr/bin/env python3
# PORTHAUL design system — palette preview (brutalist, sharp edges, no rounding)
# base: user's earthy military palette + signal orange accent
from PIL import Image, ImageDraw, ImageFont

# ---- THE SYSTEM ----
# base earthy ramp (user-picked)
INK      = (0x18,0x1C,0x14)  # #181C14  bg (near-black, green undertone)
SURFACE  = (0x24,0x28,0x22)  # derived darker surface
SURFACE2 = (0x3C,0x3D,0x37)  # #3C3D37  raised surface / borders
MOSS     = (0x69,0x75,0x65)  # #697565  dim text / muted accent
BONE     = (0xEC,0xDF,0xCC)  # #ECDFCC  text (warm bone)
BONE_DIM = (0x9a,0x94,0x86)  # derived dim bone

# signal accents — candidates to pick from
ORANGE_A = (0xE8,0x7A,0x2B)  # warmer, deeper "cargo orange"
ORANGE_B = (0xF2,0x8C,0x1E)  # brighter signal/safety orange
ORANGE_C = (0xD9,0x5A,0x1E)  # rusted/burnt — most earthy fit

W,H = 900, 560
im = Image.new("RGB", (W,H), INK)
d = ImageDraw.Draw(im)

def font(sz, bold=True):
    paths = [
        "/usr/share/fonts/TTF/DejaVuSansMono-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
    ]
    for p in paths:
        try: return ImageFont.truetype(p, sz)
        except: pass
    return ImageFont.load_default()

F_BIG = font(34); F = font(16); F_SM = font(12)

def block(x,y,w,h,c,label=None,sub=None,txtc=BONE):
    d.rectangle([x,y,x+w,y+h], fill=c)            # sharp, no radius
    if label:
        d.text((x+10,y+8), label, font=F, fill=txtc)
    if sub:
        d.text((x+10,y+h-22), sub, font=F_SM, fill=txtc)

# title
d.text((24,18), "PORTHAUL", font=F_BIG, fill=BONE)
d.text((26,60), "design system // brutalist // no curves", font=F_SM, fill=MOSS)
# a hard orange rule under the title (the "signal stripe")
d.rectangle([24,86,24+360,86+6], fill=ORANGE_A)

# --- base ramp row ---
y=120
d.text((24,y-22), "BASE RAMP", font=F_SM, fill=MOSS)
cols = [("INK / bg",INK,BONE),("SURFACE",SURFACE,BONE),("SURFACE2",SURFACE2,BONE),
        ("MOSS / dim",MOSS,INK),("BONE_DIM",BONE_DIM,INK),("BONE / text",BONE,INK)]
bw=136; gap=6; x=24
for name,c,tc in cols:
    block(x,y,bw,90,c,name,None,tc)
    x+=bw+gap

# --- signal accents row ---
y=250
d.text((24,y-22), "SIGNAL ORANGE — pick one", font=F_SM, fill=MOSS)
acc=[("ORANGE_A\ncargo",ORANGE_A),("ORANGE_B\nsafety",ORANGE_B),("ORANGE_C\nburnt",ORANGE_C)]
bw=200; x=24
for name,c in acc:
    block(x,y,bw,90,c,None,None,INK)
    d.multiline_text((x+10,y+8), name, font=F, fill=INK, spacing=2)
    x+=bw+gap

# --- mock UI strip: shows the system in action ---
y=380
d.text((24,y-22), "IN CONTEXT", font=F_SM, fill=MOSS)
# window
wx,wy,ww,wh = 24, y, W-48, 150
d.rectangle([wx,wy,wx+ww,wy+wh], fill=SURFACE)
# title bar
d.rectangle([wx,wy,wx+ww,wy+28], fill=SURFACE2)
d.text((wx+10,wy+6), "PORTHAUL v0.1.0   [192.168.1.x]:5000", font=F_SM, fill=BONE)
# a "file row" list
rows=[("DIR  ","/3ds",BONE),("FILE ","porthaul.3dsx   1.4M",BONE),
      ("FILE ","boot.firm       0.2M",BONE_DIM)]
ry=wy+40
for tag,txt,tc in rows:
    d.text((wx+12,ry), tag, font=F_SM, fill=MOSS)
    d.text((wx+62,ry), txt, font=F_SM, fill=tc)
    ry+=22
# selected row highlight = orange left bar (brutalist marker, not rounded)
d.rectangle([wx+6,wy+40,wx+6+4,wy+40+18], fill=ORANGE_A)
# transfer status bar at bottom — the signal
d.rectangle([wx,wy+wh-30,wx+ww,wy+wh], fill=INK)
d.text((wx+10,wy+wh-24), "TRANSFERRING", font=F_SM, fill=ORANGE_A)
# progress: filled orange + empty surface2
pbx=wx+140; pbw=ww-160
d.rectangle([pbx,wy+wh-24,pbx+pbw,wy+wh-10], fill=SURFACE2)
d.rectangle([pbx,wy+wh-24,pbx+int(pbw*0.62),wy+wh-10], fill=ORANGE_A)

im.save("brand/palette_preview.png")
print("saved brand/palette_preview.png")
