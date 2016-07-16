import numpy

"""DBSCAN implementation"""

class dbscan:
    # Global variables

    def __init__(self, maxDist):
        self.checkedPoints = []
        self.maxDist = maxDist;
        self.maxClusterIndex = -1

    '''Converts raw keypoints from openCV to a regular array'''
    def copyKpts(self, kpts):
        for i, kpt in enumerate(kpts):
            point = (int(kpt.pt[1]), int(kpt.pt[0]))
            self.checkedPoints.append([point[0], point[1], -1]) # X, Y, ClusterIndex

    def initPoints(self, kpts):
        for i, kpt in enumerate(kpts):
            self.checkedPoints.append([kpt[1], kpt[0], -1]) # X, Y, ClusterIndex

    def distanceBetween(self, one, two):
        return ((one[0] - two[0]) ** 2 + (one[1] - two[1]) ** 2) ** 0.5

    '''Region query'''
    def getClosestNeighbours(self, j, clustered):
        minDist = -1
        closestNeighbour = -1
        closestNeighbours = []
        for i, pointCheck in enumerate(self.checkedPoints):
            if i != j:
                distance = self.distanceBetween((self.checkedPoints[j][0], self.checkedPoints[j][1]), (pointCheck[0], pointCheck[1]))
                mode = True
                if clustered:
                    mode = pointCheck[2] > -1
                else:
                    mode = pointCheck[2] == -1
                if mode and distance <= self.maxDist: # If not itself and not clustered and within a fair distance
                    closestNeighbours.append(i) # stores pointIndices
                    if minDist == -1:
                        minDist = distance
                    if distance <= minDist:
                        minDist = distance
                        closestNeighbour = i
        return closestNeighbour, closestNeighbours

    '''Expanding for DBSCAN algorithm'''
    def checkPoint(self, i):

        # Set the cluster index
        self.checkedPoints[i][2] = self.maxClusterIndex

        # Get neighbours / do region query for unclustered neighbours
        closestUnclusteredNeighbour, closestUnclusteredNeighbours = self.getClosestNeighbours(i, False)

        k = 0
        while k < len(closestUnclusteredNeighbours):
            unclusteredNeighbour = closestUnclusteredNeighbours[k]
            k += 1

            # Recursive call for neighbours
            #neighbourClustered = checkPoint(unclusteredNeighbour)

            # Non-recursive method
            self.checkedPoints[unclusteredNeighbour][2] = self.maxClusterIndex
            newClosestUnclusteredNeighbour, newClosestUnclusteredNeighbours = self.getClosestNeighbours(unclusteredNeighbour, False)
            # Combine without repeats
            closestUnclusteredNeighbours = closestUnclusteredNeighbours + list(set(newClosestUnclusteredNeighbours) - set(closestUnclusteredNeighbours))

    '''Runs DBSCAN and false positive filter'''
    def getClusters(self, kpts, kptsSizes):
        #self.copyKpts(kpts)
        self.initPoints(kpts)
        for i, point in enumerate(self.checkedPoints):
            if point[2] == -1:
                self.maxClusterIndex += 1
                self.checkPoint(i)

        clusters = []
        clusterSizes = []
        for i in range(self.maxClusterIndex + 1):
            clusters.append([])
            clusterSizes.append([])
            for j, point in enumerate(self.checkedPoints):
                if point[2] == i:
                    clusters[i].append((point[0], point[1]))
                    clusterSizes[i].append((kptsSizes[j][0], kptsSizes[j][1]))

        filteredClusters = []
        filteredClusterSizes = []
        # find average amount of clusters
        lengths = []
        for cluster in clusters:
            lengths.append(len(cluster))
        avgLen = numpy.mean(lengths)
        stdLen = numpy.std(lengths)

        for i, cluster in enumerate(clusters):
            #if len(cluster) >= _MYPARAMS['MIN_POINTS_IN_CLUSTER'] and len(cluster) >= (avgLen + stdLen): # plus stdLen?
            #if len(cluster) <= avgLen + stdLen:
            filteredClusters.append(cluster)
            filteredClusterSizes.append(clusterSizes[i])

        return filteredClusters, filteredClusterSizes # Indicates high confidence in results
