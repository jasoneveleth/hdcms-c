# https://stackoverflow.com/questions/49829783/draw-a-gradual-change-ellipse-in-skimage

import cv2
import numpy as np

xpixels, ypixels = (1200, 400)

# proportion of screen dedicated as border
YBORDER = 0.1
XBORDER = 1.2 * (ypixels*YBORDER) / xpixels

# how much to scale the stddev
STD_SCALE = 1
# how much to add to the stddev after scale
FAKE_NOISE = 3e-2

# thickness of axes
THICCNESS = 1

# Rotated elliptical gradient - faster, vectorized numpy approach
def make_gradient(width, height, h, k, a, b):
    # Generate (x,y) coordinate arrays
    y,x = np.mgrid[-k:height-k,-h:width-h] 
    # returns an array [[[-k, -k+1, ..., height-k-1, height-k], ...], 
    #                   [[-k, ..., -k], ..., [height-k, ..., height-k]]]

    weights = np.exp((-1/2) * ((x / a)**2 + ((y / b)**2)))
    if np.max(weights):
        return weights / np.max(weights)
    else:
        return weights

def write_image(data, filename):
    data = np.array(data)
    data[:,2] = data[:,2] * STD_SCALE + FAKE_NOISE
    data[:,3] = data[:,3] * STD_SCALE + FAKE_NOISE

    max_x = np.max(data[:, 0])
    max_y = np.max(data[:, 1])
    min_x = 0
    min_y = 0
    assert(np.min(data[:, 1]) >= 0 and  np.min(data[:,0]) >= 0)

    # coords are x from 0 to ~150, y from 0 to 1.0
    # we want pixels to see a zoomed out version of this, but xpixels by ypixels
    scaling2pix = lambda x,y: (x * ((xpixels*(1 - 2 * XBORDER)) / (max_x - min_x)), 
                               y * ((ypixels*(1 - 2 * YBORDER)) / (max_y - min_y)))
    coords2pix = lambda x,y: ((xpixels*XBORDER) + scaling2pix(x, y)[0], ypixels - (ypixels*YBORDER + scaling2pix(x,y)[1]))

    img = np.zeros((ypixels, xpixels))

    for peak in data:
        a, b = scaling2pix(peak[2], peak[3]) # standard deviations
        h, k = coords2pix(peak[0], peak[1])
        img += make_gradient(xpixels, ypixels, h, k, a, b)

    # invert -> most of image is ~1, and peaks are ~0
    img = np.clip(1 - img, 0, 1)

    # convert to B, G, R image rather than just single channel black and white
    one = np.ones(img.shape)
    img = cv2.merge((img, img, one)) # causes red to fill in peak areas

    # convert to bytes 0..1 -> 0..255
    img = np.uint8(img * 255)

    # xaxis
    cv2.line(img, (0, int(ypixels*(1-YBORDER))), (xpixels, int(ypixels*(1-YBORDER))), color=(0, 0, 0), thickness=THICCNESS)
    # yaxis
    cv2.line(img, (int(xpixels*XBORDER), 0), (int(xpixels*XBORDER), ypixels), color=(0, 0, 0), thickness=THICCNESS)

    # x-axis tick mark
    tick_x, tick_y = coords2pix(max_x, 0)
    tick_len = (ypixels * YBORDER / 5)
    # tick
    cv2.line(img, (int(tick_x), int(tick_y)), (int(tick_x), int(tick_y + tick_len)), color=(0,0,0))
    font = cv2.FONT_HERSHEY_PLAIN
    # label
    cv2.putText(img, str(int(max_x*10) / 10), (int(tick_x - 3 * tick_len), int(tick_y + 3.5 * tick_len)), fontFace=font, fontScale=1, color=(0,0,0))

    # y-axis tick mark
    tick_x, tick_y = coords2pix(0, max_y)
    tick_len = (ypixels * YBORDER / 5)
    # tick
    cv2.line(img, (int(tick_x - tick_len), int(tick_y)), (int(tick_x), int(tick_y)), color=(0,0,0))
    font = cv2.FONT_HERSHEY_PLAIN
    # label
    cv2.putText(img, str(int(max_y*100) / 100), (int(tick_x - 5 * tick_len), int(tick_y)), fontFace=font, fontScale=1, color=(0,0,0))

    cv2.imwrite(filename, img)


# data = [[200, 0, 2, 0.01], [200, 1, 2, 0.01], [0, 0, 2, 0.01], [0, 1, 2, 0.01], [177, 1, 5e-4, 0]]

data = [[1.77103295e+02, 1.00000000e+00, 5.57520328e-04, 0.00000000e+00],
        [1.78106441e+02, 1.31232952e-01, 6.10816598e-04, 1.40479694e-02],
        [1.77631846e+02, 4.10934257e-02, 2.94211459e-03, 1.94570627e-03],
        [1.78520981e+02, 2.83488834e-02, 2.21788586e-03, 5.68467067e-03],
        [1.79397521e+02, 1.49197455e-02, 1.93705900e-03, 5.39074010e-03],
        [1.79001455e+02, 1.01229994e-02, 7.18630830e-02, 7.02366418e-03],
        [1.79826522e+02, 1.33477228e-02, 8.01960292e-03, 6.02056731e-03],
        [1.80276585e+02, 1.01548095e-02, 1.73067178e-02, 5.00714661e-03],
        [1.91119627e+02, 8.54168439e-03, 1.03438355e-03, 3.69555574e-03],
        [1.80692301e+02, 7.50014182e-03, 2.72640788e-02, 3.85771478e-03],
        [1.79620537e+02, 8.12444386e-03, 1.02272732e+00, 1.54908354e-03],
        [1.63124069e+02, 5.56139697e-03, 5.88170823e-04, 2.72568007e-03],
        [1.96098186e+02, 5.27513837e-03, 1.26142020e-03, 3.21444473e-03],
        [1.76095802e+02, 6.59382514e-03, 2.60700339e-04, 1.99146701e-03],
        [1.81142758e+02, 4.57685134e-03, 3.47476710e-01, 2.61308267e-03],
        [1.61107489e+02, 5.10338137e-03, 1.05167054e-03, 2.12733978e-03],
        [9.80616308e+01, 5.08153619e-03, 2.78807431e-04, 1.55053551e-03],
        [1.75087828e+02, 3.93192444e-03, 9.67726718e-04, 2.95191344e-04],
        [1.45565724e+02, 3.26842654e-03, 9.89809714e-01, 7.81720533e-04],
        [1.54230369e+02, 2.22948921e-03, 1.49717255e+01, 1.28032283e-03]]

write_image(data, "jason.png")

