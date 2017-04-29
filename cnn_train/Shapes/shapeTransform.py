'''Title: Master_Transform_Code.py
Author: Rojigan Gengatharan and the writer of the processImages.py code
NOTES: Only works for images of 525 length and 519 width, these are the global
constants. Requires the file names in the list shown at the beginning of the code, refer to base images folder 
to be present and requires a "Background.png" file which is a image of
525 width and 519 height and colour is the same as the background of the images
you are using. For example, A black background would mean Backgroud.png would
have to be the colour black
Also requires processImages.py with the rotate_image function'''


import numpy as np
import cv2
import processImages
import os
import random

ROTATION_AMOUNT = 12
WIDTH = 525
HEIGHT = 519
#originally scale_factor was 1.1
SCALE_FACTOR = 1.2
DIMENSION = 40
KERNEL_0 = (2,2)
KERNEL_1 = (3,3)

#classes from 0 to 11, 0, 1, 2, ... 11
NUMCLASSES = 11
NUMUNIQUE = 3

##FIX CODE GIVING LIKE 2+ COPIES OF IMAGE ON TOP OF EACH OTHER

'''input image is assumed to already be cropped and of proper size'''
def addnoise(image,kernel, background):



    circular_Kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, kernel)
    #High scale factor is to make sure none of the noise is cropped out
    image = crop_image(image,background, 1.45)
    image = cv2.resize(image,(40,40),interpolation = cv2.INTER_AREA)


    #Blur is to get gray pixels
    #See what happens if you use rectangular kernel here and circular kernel elsewhere
    #GaussianBlur vs Blur? Blur replaces all the pixels of a kernel with the average value of that kernel
    #This creates grey pixels near the edges of the shape, (Also in the shape, but these will be addressed later)
    blur = cv2.GaussianBlur(image,(3,3),0)
    #(5,5) was used originallzy and it was cv2.GaussianBlur(image,(5,5),0)

    #Creating the noise like this is satisfactory, for now at least
    noise = np.array(background, copy=True) 
    noise = cv2.resize(noise, (int(blur.shape[1]) ,int (blur.shape[0])), interpolation = cv2.INTER_AREA)
    make_random_white(noise)

    #blur = blur*noise, this line makes it so that all black pixels will stay black and only the gray pixels 
    #will be affected, i.e. some of the gray pixels will become black due to the random noise, will be addressed in next
    #section of code

    for i in range(0, noise.shape[0]):
        for j in range(0, noise.shape[1]):
            blur[i,j] = blur[i,j] * noise[i,j]



    #Add the modified blur to the original image, grey + white = white, grey + black = grey, black + white = white
    new = cv2.add(blur,image)
    #new = blur + image <-- original line


    #need to make all greys white
    threshold(new)

    
    #if i use scale_factor of 1.1 then resize to 40 x 40 then the noise gets cut off, if i use scale factor of 1.2 then resize to 40 x 40 then the triangle will appear a bit smaller
    rez = cv2.dilate(new,circular_Kernel,iterations = 1)

    threshold(rez)

    
    return rez

''' Turns greys into blacks '''
def threshold(image):
    for i in range(0, image.shape[0]):
        for j in range(0, image.shape[1]):
            if (image[i,j][0] != 0 and image[i,j][0] != 255):
                for x in range(3):
                    (image[i,j])[x] = 255
    return

'''puts random white spots on the background.png'''
def make_random_white(image):

    for i in range(0, image.shape[0]):
        for j in range(0, image.shape[1]):
            x = random.randint(0, 1)
            if (x == 0):
                #image[i,j] = random.randint(0,1)
                a = random.randint(0,1)
                for k in range(3):
                    image[i,j][k] = a * 255

    return





def compress_img(image, background, compression_amount):
    '''image is simply the opencv image file
    compression_amount is how much you want the file to be compressed, For
    example, a compression_amount of 2 indicates you want the horizontal size of the
    new image to be 1/2 of what it was originally (everything else stays the same)'''

    dim = (WIDTH//compression_amount, HEIGHT)
    resized = cv2.resize(image, dim, interpolation = cv2.INTER_AREA)

    l_img = np.array(background, copy=True) 
    s_img = resized

    x_offset= WIDTH// 2 - s_img.shape[1]//2
    y_offset= HEIGHT // 2 - s_img.shape[0]//2
    l_img[y_offset:y_offset+s_img.shape[0], x_offset:x_offset+s_img.shape[1]] = s_img
    image = crop_image(l_img, background, SCALE_FACTOR)

    res = cv2.resize(image, (WIDTH, HEIGHT), interpolation = cv2.INTER_LINEAR)
    return res




def crop_image(image, background, sf):
    '''Returns a cropped image to a constant scale factor, i.e the shape will take up 1/scale_factor X 100% of the final image'''
    s_img = image

    parameters = (find_limiting_p(image))


    bg = np.array(background, copy=True) 

    a1 = parameters[0][1][0]
    a2 = parameters[0][1][1]

    b1 = parameters[0][0][0]
    b2 = parameters[0][0][1]

    dimm = max(parameters[1][0],parameters[1][1])

    #next line for debugging purposes only


    #+ 1 wasn't here originally
    new_bg = cv2.resize(bg, ((int(b2) - int(b1)  + 1),(int(a2) - int(a1)  + 1)), interpolation = cv2.INTER_AREA)


    final_dim = int(dimm * sf)

    final = cv2.resize(bg, (final_dim, final_dim), interpolation = cv2.INTER_LINEAR)

    #+ 1 wasn't here originally
    new_bg = image[int(a1):int(a2) + 1, int(b1):int(b2) + 1]   #yoffset:yoffset + zz, xoffset: xoffset + zzz


    new_new_bg = cv2.resize(bg, (final_dim,final_dim), interpolation = cv2.INTER_LINEAR)


    hoff = (final_dim - (int(b2) - int(b1))) // 2
    voff = (final_dim - (int(a2) - int(a1)))// 2
    #+ 1 wasn't here originally
    new_new_bg[voff:voff + int(a2) - int(a1) + 1, hoff:hoff + int(b2) - int(b1) + 1] = new_bg
    return new_new_bg

def find_limiting_p(image):
    minC = image.shape[1] - 1 
    minR = image.shape[0] - 1
    maxC = 0
    maxR = 0
    for i in range(0, image.shape[0]):
        for j in range(0, image.shape[1]):
            if (image[i,j])[1] == 0:
                continue
            if i < minR:
                minR = i
            if j < minC:
                minC = j
            if j > maxC:
                maxC = j
            if i > maxR:
                maxR = i

    return ((minC, maxC), (minR, maxR)), ((maxC - minC),(maxR -  minR))





def rotate_img(image, rot_amount):
    '''image is simply the opencv image file
    rotation_amount is how much you want the file to be rotated, For
    example, a rotation_amount of 12 means you want to rotate the image 12
    degrees CCW'''
    new_img = processImages.rotate_image(image, rot_amount)
    return new_img



if __name__ == '__main__':
    print(NUMCLASSES)
    count = 0 
    res = np.empty([(NUMCLASSES + 1) * (NUMUNIQUE + 1) * 3 * 30 * 3,DIMENSION**2])
  
    res_labels = np.empty([(NUMCLASSES + 1) * (NUMUNIQUE + 1) * 3 * 30 * 3, 1])
    
    p_labels = np.identity(NUMCLASSES + 1)
    background = cv2.imread("Background.png", cv2.IMREAD_COLOR)
    #NUMCLASSES + 1
    for i in range(NUMCLASSES + 1):
        #NUMUNIQUE + 1
        for p in range(NUMUNIQUE + 1):
            print("Starting on " + str(i) + "_" + str(p))
            
            os.chdir("Base Set of Shapes")
            #img str(i) + '_= cv2.imread("Base Set of Shapes\\" + str(i) + '_' + str(p) + '.png', cv2.IMREAD_COLOR)
            img = cv2.imread(str(i) + '_' + str(p) + '.png', cv2.IMREAD_COLOR)
            os.chdir('..')
            
            final = img
            
            # newpath = str(i) + '_' + str(p) + "kendrick"
            # if not os.path.exists(newpath):
            #     os.makedirs(newpath)
            # os.chdir(newpath)
            # cv2.imwrite("Background.png", background)
            
            # new_imgs = [new_img]
            #compress first and then rotate, because of circle, and hexagon
            for k in range(1,4):
                new_img = compress_img(img, background, k)
                new_img = cv2.resize(new_img, (DIMENSION,DIMENSION), interpolation = cv2.INTER_AREA)     
                new_img = crop_image(new_img, background, SCALE_FACTOR)       
                # new_imgs.extend(new_img)
    
                for j in range(1, 31):
                    
                    new_new_img = rotate_img(new_img, j*12)
                    new_new_img = crop_image(new_new_img, background, SCALE_FACTOR)
                    new_new_img = cv2.resize(new_new_img, (DIMENSION,DIMENSION), interpolation = cv2.INTER_AREA)     
                    for l in range(0,3):
                        if (l == 1):
                            new_new_img = addnoise(new_new_img, KERNEL_0, background)
                        elif (l == 2):
                            new_new_img = addnoise(new_new_img, KERNEL_1, background)
                        
                        
                        threshold(new_new_img)
                        final = cv2.cvtColor(new_new_img, cv2.COLOR_BGR2GRAY)
                        final_f = final.flatten()
                        # cv2.imshow("image", final)
                        # cv2.waitKey(0)
                        # cv2.destroyAllWindows
                        res[count] = final_f / 255.0
                        res_labels[count] = i
                        count = count + 1
                        # a = str(i) + '_' + str(p) 'shape' + str(j*12) + 'rot_' + str(k) + 'squish' + str(l) + 'blur.png'
                        # cv2.imwrite(a, final)
            # os.chdir('..')
                    
    
    np.save('shapeData.npy', res)
    np.save('shapeLabels.npy', res_labels)
            #np.savetxt('roji_training_images.txt', res)