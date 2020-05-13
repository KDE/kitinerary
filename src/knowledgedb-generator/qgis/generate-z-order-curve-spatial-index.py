#
# SPDX-FileCopyrightText: Volker Krause <vkrause@kde.org>
#
# SPDX-License-Identifier: LGPL-2.0-or-later
#

import functools
import datetime
import time
import pytz
import qgis.core

#
# parameters for the spatial index
#

featureAreaRatioThreshold = 0.02 # 1% at zDepth 11 is ~150m
zDepth = 11 # minimum tile size is 1/(2^zdepth), amount of bits needed to store z index is 2*zDepth


#
# z-order curve coordinate primitives
#
xStart = -180
xRange = 360
# cut out artic regions (starting at 65°S and 80°N), that saves about 20% z-order curve coverage which we
# can better use to increase precision in more relevant areas
yStart = -65
yRange = 145

xStep = xRange / (1 << zDepth)
yStep = yRange / (1 << zDepth)

def z2x(z):
    x = 0
    for i in range(0, zDepth):
        x += (z & (1 << i * 2)) >> i
    return x

def z2y(z):
    y = 0
    for i in range(0, zDepth):
        y += (z & (1 << (1 + i * 2))) >> (i + 1)
    return y

def rectForZ(z, depth):
    mask = (1 << (2*(zDepth - depth))) - 1
    x = z2x(z & ~mask) * xStep + xStart
    y = z2y(z & ~mask) * yStep + yStart
    xSize = xRange / (1 << depth)
    ySize = yRange / (1 << depth)
    return QgsRectangle(x, y, x + xSize, y + ySize)


#
# Parallelized spatial index computation of a single sub-tile
#
LOG_CATEGORY = 'SpatialIndexBuilder'

class SpatialIndexerSubTask(QgsTask):
    def __init__(self, layer, zStart, zStartDepth):
        super().__init__('Compute spatial index sub-tile ' + str(zStart), QgsTask.CanCancel)
        self.layer = layer
        self.zStart = zStart
        self.zStartDepth = zStartDepth
        self.lastFeature = []
        self.exception = None
        self.result = []

    def run(self):
        try:
            self.computeTile(self.zStart, self.zStartDepth)
        except Exception as e:
            self.exception = e
            QgsMessageLog.logMessage('Exception in task "{}"'.format(self.exception), LOG_CATEGORY, Qgis.Info)
        return True

    def computeTile(self, zStart, depth):
        if self.isCanceled() or depth < 1:
            return
        z = zStart
        d = depth - 1
        zIncrement = 1 << (2*d)
        for i in range(0, 4):
            # find features in the input vector layer inside our current tile
            layerFeatures = []
            for f in self.layer.getFeatures(rectForZ(z, zDepth -d)):
                layerFeatures.append(f)

            feature = []
            featureCount = len(layerFeatures)

            # recurse on conflicts
            if depth > 1 and featureCount > 1:
                self.computeTile(z, d)
            # leaf tile, process the result
            else:
                # translate this into our result format: a list of (feature,areaRatio) tuples
                if featureCount == 1: # we can skip the expensive area ratio computation in this case
                    feature = [(layerFeatures[0]['tzid'], 1)]
                elif featureCount > 1:
                    rectGeo = QgsGeometry.fromRect(rectForZ(z, zDepth - d))
                    for f in layerFeatures:
                        featureArea = f.geometry().intersection(rectGeo).area()
                        feature.append((f['tzid'], featureArea / rectGeo.area()))
                feature = self.normalizeAndFilter(feature)

                # if there's a change to the previous value, propagate to the result output
                if self.lastFeature != feature and feature != []:
                    self.result.append((z, feature))
                    self.lastFeature = feature

            z += zIncrement

    def isValidFeature(self, f):
        return not f.startswith("Etc/")

    def normalizeAndFilter(self, r):
        if len(r) == 0:
            return r
        r = list(filter(lambda x: x[1] > featureAreaRatioThreshold, r))
        r = list(filter(lambda x: self.isValidFeature(x[0]), r))

        if len(r) < 1:
            return r
        n = functools.reduce(lambda n, f: n + f[1], r, 0)
        r = [(k, v/n) for (k, v) in r]
        r.sort(key = lambda x: x[1], reverse = True)
        return r

    def finished(self, result):
        if not result and self.exception != None:
            QgsMessageLog.logMessage('Task "{name}" Exception: {exception}'.format(name=self.description(), exception=self.exception), LOG_CATEGORY, Qgis.Critical)
            raise self.exception


#
# Tasks for spawning the sub-tasks doing the actual work, and accumulating the result
#
class SpatialIndexerTask(QgsTask):
    def __init__(self, layer, outputFileName):
        super().__init__('Compute spatial index', QgsTask.CanCancel)
        self.setDependentLayers([layer])
        self.tasks = []
        self.outputFileName = outputFileName
        self.exception = None
        self.conflictTiles = 0
        self.hardConflictTiles = 0
        self.startTime = time.time()

        startDepth = 4
        startIncrement = 1 << (2 * (zDepth - startDepth))
        for i in range(0, (1 << (2 * startDepth))):
            task = SpatialIndexerSubTask(layer, i * startIncrement, zDepth - startDepth)
            self.addSubTask(task, [], QgsTask.ParentDependsOnSubTask)
            self.tasks.append(task)

    def run(self):
        try:
            QgsMessageLog.logMessage('Aggregating results...', LOG_CATEGORY, Qgis.Info)

            out = open(self.outputFileName, "w")
            out.write("""/*
 * SPDX-License-Identifier: ODbL-1.0
 *
 * Autogenerated spatial index generated using QGIS.
 */

#include "timezonedb_p.h"

namespace KItinerary {
namespace KnowledgeDb {

""")
            out.write('static constexpr uint8_t timezone_index_zDepth = ' + str(zDepth) + ';\n\n')
            out.write('static constexpr TimezoneZIndexEntry timezone_index[] = {\n')

            prevFeature = ""
            prevAmbiguous = False
            for task in self.tasks:
                for (z,res) in task.result:
                    feature = ""

                    isAmbiguous = False
                    if len(res) > 1:
                        self.conflictTiles += 1
                        isAmbiguous = True
                    if len(res) > 1 and self.isConflict(res):
                        feature = "Undefined"
                        self.hardConflictTiles += 1
                    else:
                        feature = res[0][0].replace('/', '_').replace('-', '_')
                    coverage = res[0][1]

                    if prevFeature == feature and prevAmbiguous == isAmbiguous:
                        continue
                    prevFeature = feature
                    prevAmbiguous = isAmbiguous
                    if isAmbiguous:
                        out.write("    { " + str(z) + ", Tz::" + feature + ", true }, // " + str(coverage) + "\n")
                    else:
                        out.write("    { " + str(z) + ", Tz::" + feature + ", false },\n")

            out.write("};\n}\n}\n")
            out.close()
            return True

        except Exception as e:
            self.exception = e
            QgsMessageLog.logMessage('Exception in task "{}"'.format(self.exception), LOG_CATEGORY, Qgis.Info)
            return False

    def isConflict(self, r):
        tz = pytz.timezone(r[0][0])
        return not all(self.isSameTimezone(tz, pytz.timezone(x[0])) for x in r[1:])

    def isSameTimezone(self, lhs, rhs):
        try:
            # hacky tz comparison, lacking access to the rules for comparing actual DST transition times
            dt = datetime.datetime.today().toordinal()
            return all(lhs.utcoffset(datetime.datetime.fromordinal(dt + 30*x)) == rhs.utcoffset(datetime.datetime.fromordinal(dt + 30*x)) for x in range(0, 11))
        except:
            return False

    def finished(self, result):
        QgsMessageLog.logMessage('Finished task "{}"'.format(self.description()), LOG_CATEGORY, Qgis.Info)
        QgsMessageLog.logMessage(' "{}" of the area is conflicting'.format(str(self.conflictTiles / (1 << (2 * zDepth)))), LOG_CATEGORY, Qgis.Info)
        QgsMessageLog.logMessage(' "{}" of the area is not covered'.format(str(self.hardConflictTiles / (1 << (2 * zDepth)))), LOG_CATEGORY, Qgis.Info)
        QgsMessageLog.logMessage(' computation took "{}" seconds'.format(str(time.time() - self.startTime)), LOG_CATEGORY, Qgis.Info)
        if not result and self.exception != None:
            QgsMessageLog.logMessage('Task "{name}" Exception: {exception}'.format(name=self.description(), exception=self.exception), LOG_CATEGORY, Qgis.Critical)
            raise self.exception


#
# actually launch things
#
task = SpatialIndexerTask(iface.activeLayer(), '/k/kde5/src/kitinerary/src/knowledgedb/timezone_zindex.cpp')
QgsApplication.taskManager().addTask(task)
