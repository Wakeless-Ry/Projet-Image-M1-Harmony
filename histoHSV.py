from PIL import Image
import colorsys
import matplotlib.pyplot as plt
import math





template = 6
angle = 3.990526
img = Image.open("assets/img/14_Perroquet.ppm")








TEMPLATE_DEFAULT_S_WIDTH = math.pi / 6
TEMPLATE_DEFAULT_M_WIDTH = math.pi / 2
TEMPLATE_DEFAULT_L_WIDTH = math.pi

def congru(angle) :
    return angle - int(angle/(2*math.pi))*2*math.pi

def congruPos(angle) :
    return congru(congru(angle)+math.pi*2)

w, h = img.size
data = img.load()
precision = 256
histogramme = [0 for i in range(precision)]
for y in range(h) :
    for x in range(w) :
        r, g, b = data[x, y]
        h, s, v = colorsys.rgb_to_hsv(r/255, g/255, b/255)
        histogramme[int(h*precision)] += 1

anglesBars = [i/precision*2*math.pi for i in range(precision)]
ax = plt.subplot(111, polar=True)
bars = ax.bar(anglesBars, histogramme, width=2*math.pi/precision)

# Use custom colors and opacity
for a, bar in zip(anglesBars, bars):
    h = a/(2*math.pi)
    r, g, b = colorsys.hsv_to_rgb(h, 1, 1)
    bar.set_facecolor((r, g, b))
    #bar.set_alpha(0.8)

# template
templateBars = 0
maxValue = 0
anglesTmp = []
width = 0
for i in histogramme :
    maxValue = max(maxValue, i)
if (template == 0) :
    anglesTmp = [angle]
    width=TEMPLATE_DEFAULT_S_WIDTH
elif (template == 1) :
    anglesTmp = [angle]
    width=TEMPLATE_DEFAULT_M_WIDTH
elif (template == 2) :
    anglesTmp = [angle, angle-math.pi/2]
    width=[TEMPLATE_DEFAULT_S_WIDTH, TEMPLATE_DEFAULT_M_WIDTH]
elif (template == 3) :
    anglesTmp = [angle, angle-math.pi]
    width=[TEMPLATE_DEFAULT_S_WIDTH, TEMPLATE_DEFAULT_S_WIDTH]
elif (template == 4) :
    anglesTmp = [angle]
    width=TEMPLATE_DEFAULT_L_WIDTH
elif (template == 5) :
    anglesTmp = [angle, angle-math.pi]
    width=[TEMPLATE_DEFAULT_M_WIDTH, TEMPLATE_DEFAULT_S_WIDTH]
elif (template == 6) :
    anglesTmp = [angle, angle-math.pi]
    width=[TEMPLATE_DEFAULT_M_WIDTH, TEMPLATE_DEFAULT_M_WIDTH]
for i in range(len(anglesTmp)) :
    anglesTmp[i] = congruPos(anglesTmp[i])
templateBars = ax.bar(anglesTmp, [maxValue for _ in range(len(anglesTmp))], width=width)

for a, bar in zip(anglesTmp, templateBars):
    h = a/(2*math.pi)
    r, g, b = colorsys.hsv_to_rgb(h, 1, 1)
    bar.set_facecolor((r, g, b))
    bar.set_alpha(0.25)

plt.show()