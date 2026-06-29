#!/usr/bin/env python3
# PORTHAUL logo — FINAL marks.
#  wordmark: wm_left, orange vertical bar extended to full height (top of P .. bottom of H)
#  icon: user's P-with-arrow-rising — cleaned to pixel grid, brutalist, our palette.
from PIL import Image, ImageDraw, ImageFont

INK    = (0x18,0x1C,0x14)
MOSS   = (0x69,0x75,0x65)
BONE   = (0xEC,0xDF,0xCC)
ORANGE = (0x9A,0xAD,0x6A)  # was orange; now SAGE accent #9AAD6A

def grid(w,h,bg=INK):
    im=Image.new("RGB",(w,h),bg); return im, im.load()
def rect(px,x0,y0,x1,y1,c,W,H):
    for y in range(y0,y1+1):
        for x in range(x0,x1+1):
            if 0<=x<W and 0<=y<H: px[x,y]=c

# 3x5 stencil font
F = {
 'P':["111","101","111","100","100"],
 'O':["111","101","101","101","111"],
 'R':["111","101","111","110","101"],
 'T':["111","010","010","010","010"],
 'H':["101","101","111","101","101"],
 'A':["111","101","111","101","101"],
 'U':["101","101","101","101","111"],
 'L':["100","100","100","100","111"],
}
def word(px,s,x,y,c,W,H,sc=2):
    for ch in s:
        for r,row in enumerate(F[ch]):
            for cc,v in enumerate(row):
                if v=='1':
                    rect(px,x+cc*sc,y+r*sc,x+cc*sc+sc-1,y+r*sc+sc-1,c,W,H)
        x+=(3*sc)+sc
    return x

# ---- FINAL WORDMARK: vertical orange bar, FULL height ----
def wordmark(sc=2):
    W,H=58,28; im,px=grid(W,H)
    topY=3
    botY=topY+10+2+10-1     # PORT(10) + gap(2) + HAUL(10)
    word(px,"PORT",6,topY,BONE,W,H,sc)
    word(px,"HAUL",6,topY+12,BONE,W,H,sc)
    # orange vertical bar on the far left, spanning the full height of the lockup
    rect(px,0,topY,1,botY,ORANGE,W,H)
    return im

# ---- FINAL ICON: explicit pixel map (full control) ----
# 24x24 grid. '.'=bg  'B'=bone P  'O'=orange arrow. Mirrors the user's sketch:
# square P on the left, orange arrow rising NE on the right, clean sharp head.
ICON_MAP = [
    "........................",
    ".BBBBBBBBBB.............".ljust(24,'.'),
    ".BBBBBBBBBB.............".ljust(24,'.'),
    ".BBB.....BBB...OOOOOOO..".ljust(24,'.'),
    ".BBB.....BBB...OOOOOOO..".ljust(24,'.'),
    ".BBB.....BBB.....OOOOO..".ljust(24,'.'),
    ".BBBBBBBBBB.....OOOOOO..".ljust(24,'.'),
    ".BBBBBBBBBB....OOOOOO...".ljust(24,'.'),
    ".BBB.........OOOOOO.....".ljust(24,'.'),
    ".BBB........OOOOO.......".ljust(24,'.'),
    ".BBB.......OOOOO........".ljust(24,'.'),
    ".BBB......OOOOO.........".ljust(24,'.'),
    ".BBB.....OOOOO..........".ljust(24,'.'),
    ".BBB....OOOOO...........".ljust(24,'.'),
    ".BBB...OOOOO............".ljust(24,'.'),
    ".BBB..OOOOO.............".ljust(24,'.'),
    ".BBBOOOOOO.............".ljust(24,'.'),
    ".BBBOOOOO..............".ljust(24,'.'),
    ".BBB...................",
    ".BBB...................",
    ".BBB...................",
    "........................",
]
def icon():
    H=len(ICON_MAP); W=len(ICON_MAP[0])
    im,px=grid(W,H)
    for y,row in enumerate(ICON_MAP):
        for x,ch in enumerate(row):
            if ch=='B': px[x,y]=BONE
            elif ch=='O': px[x,y]=ORANGE
    return im

wmk=wordmark()
ic=icon()
wmk.resize((wmk.width*10,wmk.height*10),Image.NEAREST).save("brand/FINAL_wordmark.png")
ic.resize((ic.width*12,ic.height*12),Image.NEAREST).save("brand/FINAL_icon.png")
# also a 48px icon (app-size sanity) and 24px (tiny)
ic.resize((48,48),Image.NEAREST).save("brand/FINAL_icon_48.png")
ic.resize((24,24),Image.NEAREST).save("brand/FINAL_icon_24.png")

# contact sheet
bg=(0x10,0x12,0x0e)
sheet=Image.new("RGB",(900,360),bg)
dd=ImageDraw.Draw(sheet)
try: fnt=ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",15)
except: fnt=ImageFont.load_default()
sheet.paste(wmk.resize((wmk.width*8,wmk.height*8),Image.NEAREST),(20,30))
dd.text((20,12),"WORDMARK",font=fnt,fill=MOSS)
sheet.paste(ic.resize((ic.width*8,ic.height*8),Image.NEAREST),(560,30))
dd.text((560,12),"ICON / MONOGRAM",font=fnt,fill=MOSS)
# tiny sizes row
dd.text((560,300),"48px",font=fnt,fill=MOSS); sheet.paste(ic.resize((48,48),Image.NEAREST),(560,310-260+260))
sheet.save("brand/FINAL_marks.png")
print("saved brand/FINAL_marks.png, FINAL_wordmark.png, FINAL_icon.png")
