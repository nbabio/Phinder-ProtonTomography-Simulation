import json, sys, optparse
import copy



if __name__=='__main__':

    parser = optparse.OptionParser(usage='usage: %prog [options] path', version='%prog 1.0')
    parser.add_option('-i', '--input', action='store', type='string', dest='inputFile', default='confExample.json', help='Configuration file template')
    parser.add_option('-o', '--output', action='store', type='string', dest='outputFile', default='conf.json', help='Final configuration file')

    (opts, args) = parser.parse_args()

    with open(opts.inputFile, 'r') as file:
        data = json.load(file)

    theWorld = copy.copy(data['theWorld'])
    theBeam = copy.copy(data['theBeam'])
    phantom = copy.copy(data['Phantoms'][0])
    detector = copy.copy(data['Detectors'][0])
    layer = copy.copy(data['Detectors'][0]['Layers'][0])
    fiber = copy.copy(data['Detectors'][0]['Layers'][0]['Fibers'][0])
    
    #Modify world directly in data
    theWorld['xSizeWorld'] = 300
    theWorld['ySizeWorld'] = 300
    theWorld['zSizeWorld'] = 300
    #Modify beam directly in data
    theBeam['xBeamPosition'] = 0
    theBeam['yBeamPosition'] = 0
    theBeam['zBeamPosition'] = 125
    theBeam['xBeamSigma'] = 0.1
    theBeam['yBeamSigma'] = 0.1
    theBeam['xDir'] = 0
    theBeam['yDir'] = 0
    theBeam['zDir'] = 0
    theBeam['maxOpenAngle'] = 0.02
    theBeam['nStep'] = 100
    theBeam['nParticles'] = 1
    theBeam['nParticlesDistribution'] = "Constant"
    theBeam['energy'] = 250.0
    theBeam['energyDistribution'] = "Constant"
    theBeam['energySigma'] = 1.0
    theBeam['tBeamSigma'] = 1.0
    #Modify phantoms directly in data
    thePhantoms = []
    phantom['name'] = "lung"
    phantom['xPos'] = 0
    phantom['yPos'] = 0
    phantom['zPos'] = 0
    phantom['xDir'] = 0
    phantom['yDir'] = 0
    phantom['zDir'] = 1
    phantom['zsize'] = 1
    phantom['radius'] = 1
    phantom['material'] = "lung"
    thePhantoms.append(phantom)
    #Modify the detectors
    theDetectors = []
    zPos = [70, -70]
    for z in zPos:
        theDet = copy.copy(detector)
        theDet['xPosDetector'] = 0.0
        theDet['yPosDetector'] = 0.0
        theDet['zPosDetector'] = z
        theDet['xDirDetector'] = 0.0
        theDet['xDirDetector'] = 0.0
        theDet['xPosDetector'] = 0.0
        theDet['xSizeDetector'] = 80.0
        theDet['ySizeDetector'] = 80.0
        theDet['zSizeDetector'] = 15.0
        theLayers = []
        zLayerPos = [7.0, 5.0, -5.0, -7.0]
        zDirLayer = [0.0, 90.0, 0.0, 90.0]
        for i, ilayer in enumerate(zLayerPos):
            theLayer = copy.copy(layer)
            theLayer['xPosLayer'] = 0.0
            theLayer['yPosLayer'] = 0.0
            theLayer['zPosLayer'] = zLayerPos[i]
            theLayer['xDirLayer'] = 0.0
            theLayer['yDirLayer'] = 0.0
            theLayer['zDirLayer'] = zDirLayer[i]
            theLayer['xSizeLayer'] = 80.0
            theLayer['ySizeLayer'] = 80.0
            theLayer['zSizeLayer'] = 1.0
            theFibers = []
            NFibers = 21
            FiberZsize = 0.8
            FiberXsize = 0.8
            FiberYsize = theLayer['ySizeLayer']
            step = FiberYsize / NFibers
            for j in range(NFibers):
                xfiber = -theLayer['xSizeLayer']/2.0 + j * step
                yfiber = 0.0
                zfiber = theLayer['zPosLayer']
                theFiber = copy.copy(fiber)
                theFiber['xPosSensor'] = xfiber
                theFiber['yPosSensor'] = yfiber
                theFiber['zPosSensor'] = zfiber
                theFiber['xDirSensor'] = 90
                theFiber['yDirSensor'] = 0
                theFiber['zDirSensor'] = 0
                theFiber['xSizeSensor'] = FiberXsize
                theFiber['ySizeSensor'] = FiberYsize
                theFiber['zSizeSensor'] = FiberZsize
                theFiber['coreRadius'] = 0.25
                theFiber['claddingRadius'] = 0.30
                theFiber['outerRadius'] = 0.35
                theFiber['length'] = FiberYsize/2.0
                theFiber['coreMaterial'] = 'lead'
                theFiber['claddingMaterial'] = 'lead'
                theFibers.append(theFiber)
            theLayer['Fibers'] = theFibers
            theLayers.append(theLayer)
        theDet['Layers'] = theLayers
        theDetectors.append(theDet)


    newData = dict()
    newData['theWorld'] = theWorld
    newData['theBeam'] = theBeam
    newData['Phantoms'] = thePhantoms
    newData['Detectors'] = theDetectors

    print('Making a copy')
    json_str = json.dumps(newData, indent=4)
    with open(opts.outputFile, "w") as f:
        f.write(json_str)
