#!/usr/bin/env python3
# Clean the user's hand-made PORTHAUL icon into a crisp brand asset:
#  1) snap every pixel to the nearest canonical palette color (INK/BONE/ORANGE)
#  2) auto-crop the background border
#  3) export master + 48/24px sizes
# This keeps the user's superior shape, just removes compression noise.
from PIL import Image

INK    = (0x18,0x1C,0x14)
BONE   = (0xEC,0xDF,0xCC)
ORANGE = (0x9A,0xAD,0x6A)  # was orange; now SAGE accent
PAL = [INK, BONE, ORANGE]

def nearest(c):
    return min(PAL, key=lambda p:(p[0]-c[0])**2+(p[1]-c[1])**2+(p[2]-c[2])**2)

src = Image.open("brand/icon_master_src.png").convert("RGB")
W,H = src.size
px = src.load()

# 1) quantize
q = Image.new("RGB",(W,H))
qp = q.load()
for y in range(H):
    for x in range(W):
        qp[x,y] = nearest(px[x,y])

# 2) auto-crop: trim rows/cols that are entirely INK
def is_ink_row(y): return all(qp[x,y]==INK for x in range(W))
def is_ink_col(x): return all(qp[x,y]==INK for y in range(H))
top=0
while top<H and is_ink_row(top): top+=1
bot=H-1
while bot>top and is_ink_row(bot): bot-=1
left=0
while left<W and is_ink_col(left): left+=1
right=W-1
while right>left and is_ink_col(right): right-=1
# add a small uniform margin so it breathes
m = max(4,(right-left)//20)
left=max(0,left-m); top=max(0,top-m); right=min(W-1,right+m); bot=min(H-1,bot+m)
crop = q.crop((left,top,right+1,bot+1))
print("cropped to", crop.size)

# 3) exports
crop.save("brand/FINAL_icon.png")                 # master (replaces old napikselennaya)
crop.resize((48,48),Image.LANCZOS).save("brand/icon_48.png")
crop.resize((24,24),Image.LANCZOS).save("brand/icon_24.png")
# also nearest-downsample versions for crisp pixel feel at small sizes
crop.resize((48,48),Image.NEAREST).save("brand/icon_48_px.png")
print("saved FINAL_icon.png + 48/24")
