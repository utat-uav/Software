import cv2
import numpy
import os
import shutil
import copy
import random
from dbscan import dbscan
import sys
import csv

from time import strftime

# TODO: 
#   Improve false positive filter
#   Possibly using standard deviation of hue variance (compare the std of cropped image to master image or only the std within the cropped image?)
#   Test if making the BKS (blur amount) proportional to the standard deviation in contrast works

_MYPARAMS = {
    'ACTIVE_CHANNEL' : [1,2],
    'IMAGE' : "IMG_0526.JPG",
    'HAS_BLUR' : 1,
    'BKS' : 6, # Blur Kernal size
    'SIZE_OF_ROI' : 300, # Cluster size
    'MIN_POINTS_IN_CLUSTER' : 5,
    'USE_TREE_FILTER' : 1, # Filters out crops that are "tree colored"
	# USC SETTINGS
    #'MAX_AREA' : 38000,
    #'MIN_AREA' : 3500,
	# TARGET SETTINGS
    'MAX_AREA' : 30000,
    'MIN_AREA' : 2650,
	'CROP_PADDING' : 8,
    'GPS_LOG' : "<unknown>",
    'OUTPUT_FOLDER' : "Output"
}


'''Used to make sure that a point is within a certain ROI'''
def clamp(num, mymin, mymax):
    return min(mymax, max(num, mymin))

'''Colorful drawings'''
def drawClusters(imgClusteredRegions, clusters, clusterLocations):
    for i, cluster in enumerate(clusters):
        #Get point
        clusterLocation = clusterLocations[i]
        hs = _MYPARAMS['SIZE_OF_ROI']/2
        # Draw rectangle
        cv2.rectangle(imgClusteredRegions, (clusterLocation[1]-hs, clusterLocation[0]-hs), (clusterLocation[1]+hs,clusterLocation[0]+hs), (255, 255, 255), 1)
        # Draw number of dots
        cv2.putText(imgClusteredRegions, str(len(cluster)) + ' points', (clusterLocation[1]-hs, clusterLocation[0]-hs), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255))
        # Draw dots
        randClusterColor = (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
        for clusterPoint in cluster:
            cv2.circle(imgClusteredRegions, (int(clusterPoint[1]), int(clusterPoint[0])), 4, randClusterColor, 2)
    return imgClusteredRegions

def drawCroppedRegions(imgCroppedRegions, locations, sizes):
    for i, location in enumerate(locations):
        hs = int((sizes[i][0] if sizes[i][0] > sizes[i][1] else sizes[i][1])/2)
        cv2.rectangle(imgCroppedRegions, (int(location[1])-hs, int(location[0])-hs), (int(location[1])+hs, int(location[0])+hs), (255, 255, 255), 1)
    return imgCroppedRegions

'''Pythag it'''
def distanceBetween(one, two):
    return ((one[0] - two[0]) ** 2 + (one[1] - two[1]) ** 2) ** 0.5

'''Converts hulls to useful data'''
def hulls2Points(hulls):
    points = []
    sizes = []

    for hull in hulls:
        maxX = 0
        minX = 100000
        maxY = 0
        minY = 100000
        x = []
        y = []
        for point in hull:
            maxX = max(point[0][0], maxX)
            minX = min(point[0][0], minX)
            maxY = max(point[0][1], maxY)
            minY = min(point[0][1], minY)
            x.append(point[0][0])
            y.append(point[0][1])
        points.append([numpy.mean(x), numpy.mean(y)])
        sizes.append([maxX-minX, maxY-minY])

    return points, sizes

def largestSize(clusters, sizes):

    #detailedCoordinates = []
    largestSizes = []
    for i, cluster in enumerate(clusters):
        #detailedCoordinates.append([])
        minX = 1000000
        minY = 1000000
        maxX = 0
        maxY = 0
        for j, point in enumerate(cluster):
            if point[0]-(sizes[i][j][0])/2 < minX:
                minX = point[0]-(sizes[i][j][0])/2
            if point[0]+(sizes[i][j][0])/2 > maxX:
                maxX = point[0]+(sizes[i][j][0])/2
            if point[1]-(sizes[i][j][1])/2 < minY:
                minY = point[1]-(sizes[i][j][1])/2
            if point[1]+(sizes[i][j][1])/2 > maxY:
                maxY = point[1]+(sizes[i][j][1])/2
            #detailedCoordinates[i].append([point[0]-(sizes[i][j][0])/2, point[0]+(sizes[i][j][0])/2, point[1]-(sizes[i][j][1])/2, point[1]+(sizes[i][j][1])/2]) #minX, maxX, minY, maxY
        largestSizes.append([maxX - minX, maxY - minY])

    #for i, size in enumerate(sizes):
    #    maxX = 0
    #    maxY = 0
    #    for point in size:
    #        if point[0] > maxX:
    #            maxX = point[0]
    #        if point[1] > maxY:
    #            maxY = point[1]
    #    largestSizes.append([maxX, maxY])
    return largestSizes


'''Filter for trees by color'''
def filterTrees(imgin, clusters, clusterSizes):
    filteredClusters = []
    filteredClusterSizes = []

    _IMBND = (imgin.shape[0], imgin.shape[1])
    avgColor = [0, 0, 0]
    for i, mypoint in enumerate(clusters):
        cropSize = clusterSizes[i][1]/2 if clusterSizes[i][1]/2 > clusterSizes[i][0]/2 else clusterSizes[i][0]/2
        row_crop = (clamp(mypoint[0]-cropSize, 0, _IMBND[0]), clamp(mypoint[0]+cropSize, 0, _IMBND[0]))
        col_crop = (clamp(mypoint[1]-cropSize, 0, _IMBND[1]), clamp(mypoint[1]+cropSize, 0, _IMBND[1]))
        new_crop = imgin[row_crop[0]:row_crop[1], col_crop[0]:col_crop[1]]

        # Create solid image and threshold
        new_crop = cv2.cvtColor(new_crop, cv2.COLOR_BGR2HSV)
        averagePixel = cv2.mean(new_crop)
        avgColor = [avgColor[0]+averagePixel[0], avgColor[1]+averagePixel[1], avgColor[2]+averagePixel[2]]
        if not (averagePixel[0] > 17 and averagePixel[0] < 58 and averagePixel[1] > 10 and averagePixel[1] < 105 and averagePixel[2] > 0 and averagePixel[2] < 103):
            # Add to return list
            filteredClusters.append(mypoint)
            filteredClusterSizes.append(clusterSizes[i])
    
    #34.5, 58.5, 48.2 in HSV approximately make tree-green
    return filteredClusters, filteredClusterSizes

'''Average the location of all clusters'''
def averageClusters(clusters):
    averagedClusters = []
    for i, cluster in enumerate(clusters):
        avgX = 0
        avgY = 0
        # sum points
        for point in cluster:
            avgX = avgX + point[0]
            avgY = avgY + point[1]
        # average points
        avgX = avgX / len(cluster)
        avgY = avgY / len(cluster)
        # add to list of clusters
        averagedClusters.append((int(avgX), int(avgY)))

    return averagedClusters

'''Get standard deviation of clusters'''
def stdClusters(clusters):
    clusterStds = []
    for i, cluster in enumerate(clusters):
        xVals = []
        yVals = []
        for point in cluster:
            xVals.append(point[0])
            yVals.append(point[1])
        xSTD = numpy.std(xVals)
        ySTD = numpy.std(yVals)
        clusterStds.append([xSTD, ySTD])

    return clusterStds

def headingToString(heading):
    if heading >= 45 and heading < 135:
        return "East"
    if heading >= 135 and heading < 225:
        return "South"
    if heading >= 225 and heading < 315:
        return "West"
    if heading < 45 or heading >= 315:
        return "North"
    
def compressImage(image):
    height, width = image.shape[:2]
    resized_image = cv2.resize(image, (700, int(700.0/width*height)), interpolation = cv2.INTER_CUBIC) 
    return resized_image
    
def main():
    heading = 0
    latitude = ""
    longitude = ""
    altitude = ""
    with open(_MYPARAMS['GPS_LOG'], 'rb') as gpsfile:
        spamreader = csv.reader(gpsfile, delimiter=',', quotechar='|')
        for row in spamreader:
            found = False
            count = 0;
            for name in row:
                count = count + 1
                if name == os.path.basename(_MYPARAMS['IMAGE']):
                    found = True
                if found and count == 5:
                    heading = float(name)
                if found and count == 2:
                    latitude = name
                if found and count == 3:
                    longitude = name
                if found and count == 4:
                    altitude = name
    headingString = headingToString(heading)

    # Initialize dbscan
    myDbscan = dbscan(_MYPARAMS['SIZE_OF_ROI']/2)

    PRINT_LOG_OUT = []
    PRINT_LOG_OUT.append("[Date]")
    PRINT_LOG_OUT.append("Date Analyzed = " + strftime("%Y-%m-%d %H:%M:%S"))
    PRINT_LOG_OUT.append("\n[Position]") # For some reason this code went missing in a commit
    PRINT_LOG_OUT += ["heading = " + headingString]
    PRINT_LOG_OUT += ["headingDegrees = " + str(heading)]
    PRINT_LOG_OUT += ["latitude = " + latitude]
    PRINT_LOG_OUT += ["longitude = " + longitude]
    PRINT_LOG_OUT += ["altitude = " + altitude]
    PRINT_LOG_OUT.append("\n[Analysis Parameters]")
    # print parameters
    PRINT_LOG_OUT += [str(k) + " = "  + str(_MYPARAMS[k]) for k in _MYPARAMS.keys()]
    
    # output folder
    #fileList = os.listdir(_MYPARAMS['OUTPUT_FOLDER'])
    #for fileName in fileList:
    #    os.remove(_MYPARAMS['OUTPUT_FOLDER'] + "/"+fileName)
    
    # import image from file
    print("Loading Image...")
    imgin = cv2.imread(_MYPARAMS['IMAGE'], cv2.IMREAD_COLOR)

    cv2.imwrite(os.path.join(_MYPARAMS['OUTPUT_FOLDER'], os.path.basename(_MYPARAMS['IMAGE'])), compressImage(imgin))

    height, width, channels = imgin.shape
    PRINT_LOG_OUT.append("Width = " + str(width))
    PRINT_LOG_OUT.append("Height = " + str(height))
    
    # Check if image imported correctly
    #if (imgin == None):
    #    print "Image does not exist! Aborting...\n"
    #    return;

    hsv_imgin = cv2.cvtColor(imgin, cv2.COLOR_BGR2HSV)

    # Detect image size [rows, columns]
    _IMBND = (hsv_imgin.shape[0], hsv_imgin.shape[1])

    hsv_chans = cv2.split(hsv_imgin); # split image into HSV channels

    # Get both blurred and not blurred files for image processing
    if (_MYPARAMS['HAS_BLUR']):
        hsv_chans =  [cv2.blur(hsvim, (_MYPARAMS['BKS'], _MYPARAMS['BKS'])) for hsvim in hsv_chans]
    
    # may use other feature detector for testing
    FD_TYPE = "MSER"
    PRINT_LOG_OUT.append("FD Type = " + FD_TYPE)
    print("Running MSER...")
    # delta, maxArea, minArea, maxVariation, minDiversity, maxEvolution, areaThreshold, minMargin, edgeBlurSize
    # Decreasing maxVariation increases how sharp edges need to be
    my_fd = cv2.MSER_create(5, _MYPARAMS['MIN_AREA'] / 2738 * _IMBND[0], _MYPARAMS['MAX_AREA'] / 2738 * _IMBND[0], 0.099, 0.65, 200, 1.01, 0.003, 5) # Default is 5, 60, 14400, 0.25, 0.2, 200, 1.01, 0.003, 5

    # FD_TYPE = "SimpleBlob"
    # PRINT_LOG_OUT.append("FD Type: " + FD_TYPE)
    # my_fd = cv2.SimpleBlobDetector_create() 
    
    imgClusteredRegions = copy.copy(imgin)

    PRINT_LOG_OUT.append("\n[Channel Keypoints]")

    kpts = [] # (k)ey(p)oin(t) out
    kptsSize = []
    dkpsout = [] # (d)isplay (k)ey(p)oint (out)put
    for i, im in enumerate(hsv_chans):
        local_kpt = my_fd.detect(im, None) # local keypoints

        if len([x for x in _MYPARAMS['ACTIVE_CHANNEL'] if x == i]) > 0:
            # Outputs image of regions
            vis = im.copy()
            regions = my_fd.detectRegions(im, None)
            hulls = [cv2.convexHull(s.reshape(-1, 1, 2)) for s in regions]
            hullLocations, hullSizes = hulls2Points(hulls)
            cv2.polylines(vis, hulls, 1, (0, 255, 0))
            #cv2.imwrite(os.path.join(_MYPARAMS['OUTPUT_FOLDER'], 'region visualization' + str(i) + '.jpg'), vis)

            for j, point in enumerate(hullLocations):
                kpts.append(point)
                kptsSize.append(hullSizes[j])

        if (local_kpt):
            # don't know how the third param works yet  -->
            local_dpksout = cv2.drawKeypoints(im, local_kpt, im) 
            dkpsout.append([local_dpksout]) # append to master list
            #cv2.imwrite(os.path.join(_MYPARAMS['OUTPUT_FOLDER'], 'dkpsout' + str(i) + '.jpg'), local_dpksout)

            # print out num of keypoints and other info 
            PRINT_LOG_OUT.append('Channel ' + str(i) + ' = ' + str(len(local_kpt)))

    # Crop out ROIs for active_channel
    clusters, clusterSizes = myDbscan.getClusters(kpts, kptsSize)
    averagedClusters = averageClusters(clusters)
    clusterSizes = largestSize(clusters, clusterSizes)
    # Tree filter
    print("Filtering trees...")
    if _MYPARAMS['USE_TREE_FILTER']:
        averagedClusters, clusterSizes = filterTrees(imgin, averagedClusters, clusterSizes)

    imageName = os.path.basename(_MYPARAMS['IMAGE']).split('.')[0]
		
    croppedImgNames = []
    print("Cropping...")
    for i, mypoint in enumerate(averagedClusters):
        cropSize = clusterSizes[i][1]/2 if clusterSizes[i][1]/2 > clusterSizes[i][0]/2 else clusterSizes[i][0]/2
        padding = _MYPARAMS['CROP_PADDING']
        row_crop = (clamp(mypoint[0]-cropSize - padding, 0, _IMBND[0]), clamp(mypoint[0]+cropSize + padding, 0, _IMBND[0]))
        col_crop = (clamp(mypoint[1]-cropSize - padding, 0, _IMBND[1]), clamp(mypoint[1]+cropSize + padding, 0, _IMBND[1]))
        new_crop = imgin[row_crop[0]:row_crop[1], col_crop[0]:col_crop[1]]
        croppedImgNames.append(imageName + 'roi' + str(i) + '.jpg')
        cv2.imwrite(os.path.join(_MYPARAMS['OUTPUT_FOLDER'], croppedImgNames[i]), new_crop)

    # Log clustering info
    PRINT_LOG_OUT.append("\n[Crop Info]")
    PRINT_LOG_OUT.append("Number of Crops = " + str(len(averagedClusters)))

    # Write log for the clusters
    for i, cluster in enumerate(averagedClusters):
        PRINT_LOG_OUT.append("\n[Crop " + str(i+1) + "]")
        PRINT_LOG_OUT.append("Image Name = " + croppedImgNames[i])
        PRINT_LOG_OUT.append("X = " + str(averagedClusters[i][1]))
        PRINT_LOG_OUT.append("Y = " + str(averagedClusters[i][0]))
        cropSize = clusterSizes[i][1]/2 if clusterSizes[i][1]/2 > clusterSizes[i][0]/2 else clusterSizes[i][0]/2
        padding = _MYPARAMS['CROP_PADDING']
        PRINT_LOG_OUT.append("Size = " + str(2*(cropSize + padding)))

    # Output cluster locations
    #imgClusteredRegions = drawClusters(imgClusteredRegions, clusters, averagedClusters)
    imgClusteredRegions = drawCroppedRegions(imgClusteredRegions, averagedClusters, clusterSizes)
    cv2.imwrite(os.path.join(_MYPARAMS['OUTPUT_FOLDER'], 'croppedRegions.jpg'), imgClusteredRegions)
	
    # print result info to log file
    with open(os.path.join(_MYPARAMS['OUTPUT_FOLDER'], imageName + ' Results.ini'), 'a') as f:
        for line in PRINT_LOG_OUT:
            f.write(line + '\n')

if __name__ == "__main__":
    _MYPARAMS['IMAGE'] = sys.argv[1];
    _MYPARAMS['GPS_LOG'] = sys.argv[2];
    _MYPARAMS['OUTPUT_FOLDER'] = sys.argv[3] + '\\';
    print (_MYPARAMS['OUTPUT_FOLDER'])
    main()
    
