# Bunch time structure plots, from the CSV files written by exportBunches.C.
# The simulation is run with 1 proton per event and the bunches are built here: bunch k = events
# [k * nPerBunch, (k + 1) * nPerBunch), and every time of bunch k is shifted by k * period.
# The time spread inside a bunch comes from the simulation itself (tBeamSigma in the json file).
#
#   primary mode: one vertical line per crossing of the primary proton, one panel per layer
#   fiber mode:   time vs fiber of each crossing of the primary proton, one panel per layer
#   photons mode: histogram of the arrival time of the photons at the fiber ends, one panel per layer
#
# Usage (from ProtonTomographySimulator-build):
#   python ../ProtonTomographySimulator/dataAnalysis/plotBunches.py -m primary -p bunches/run --nPerBunch 1
#   python ../ProtonTomographySimulator/dataAnalysis/plotBunches.py -m primary -p bunches/run --nPerBunch 20
#   python ../ProtonTomographySimulator/dataAnalysis/plotBunches.py -m photons -p bunches/run --side split

import optparse, os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

NLayers = 8
NFibers = 21
LayerSize = 20.0   # cm, xSizeLayer in makeConfiguration.py


def loadBunches(fileName, nPerBunch, nBunches, period, tOffset, firstEvent):
    d = np.atleast_1d(np.genfromtxt(fileName, delimiter=',', names=True))
    bunch = (d['event'].astype(int) - firstEvent) // nPerBunch
    sel = (bunch >= 0) & (bunch < nBunches)
    d, bunch = d[sel], bunch[sel]
    if bunch.max() < nBunches - 1:
        print('WARNING: only %d bunches in %s, simulate at least %d events' % (bunch.max() + 1, fileName, nPerBunch * nBunches))
    t = d['t_ns'] + bunch * period + tOffset   # common clock for all the bunches
    return d, bunch, t


def plotBunches(mode, d, bunch, t, nBunches, tMax, binWidth, sides):
    # One column per entry of sides: None = both fiber ends together, -1 / +1 = only that end
    fig, axes = plt.subplots(NLayers, len(sides), figsize=(10 * len(sides), 1.6 * NLayers), sharex=True, sharey='row', squeeze=False)
    for (layer, col), ax in np.ndenumerate(axes):
        side = sides[col]
        m = d['layer'] == layer
        if side is not None:
            m = m & (d['side'] == side)
        colors = ['C%d' % (b % 10) for b in bunch[m]]
        if mode == 'primary':
            ax.vlines(t[m], 0, 1, colors=colors, lw=1)
            ax.set_ylim(0, 1)
            ax.set_yticks([])
        elif mode == 'fiber':
            # Even layers are rotated 90 deg: fibers along x, the index measures -y. Odd layers: fibers along y, the index measures x
            # The size of each layer is 20.0 cm, and there are 21 fibers, so the pitch is 20.0 / 21.0 = 0.9523809523809523 cm
            pitch = LayerSize / NFibers
            sign, axis = (-1, 'y') if layer % 2 == 0 else (1, 'x')
            pos = sign * (d['fiber'][m] - (NFibers - 1) / 2) * pitch
            ax.scatter(t[m], pos, c=colors, s=6)
            ax.set_ylim(LayerSize/2, LayerSize/2)
            ax.set_ylabel('%s [cm]' % axis)
        else:
            bins = np.arange(0, tMax + binWidth, binWidth)
            for b in range(nBunches):
                ax.hist(t[m & (bunch == b)], bins=bins, color='C%d' % (b % 10), histtype='stepfilled', alpha=0.7)
            ax.set_ylabel('photons')
        sideLabel = '' if side is None else ', side %+d' % side
        ax.set_title('Detector %d, layer %d%s' % (layer // 4, layer, sideLabel), fontsize=9)
    axes[-1, 0].set_xlim(0, tMax)
    for ax in axes[-1]:
        ax.set_xlabel('t [ns]')
    fig.tight_layout()
    return fig


if __name__ == '__main__':

    parser = optparse.OptionParser(usage='usage: %prog [options]')
    parser.add_option('-m', '--mode', dest='mode', default='primary', help='primary, fiber or photons')
    parser.add_option('-p', '--prefix', dest='prefix', default='bunches/run', help='prefix of the exportBunches.C files')
    parser.add_option('-o', '--outDir', dest='outDir', default='bunchPlots', help='output directory')
    parser.add_option('--nPerBunch', dest='nPerBunch', type='int', default=20, help='protons (events) per bunch')
    parser.add_option('--nBunches', dest='nBunches', type='int', default=5, help='number of bunches')
    parser.add_option('--period', dest='period', type='float', default=14.2, help='time between bunches [ns]')
    parser.add_option('--tOffset', dest='tOffset', type='float', default=10.0, help='time added to all the bunches [ns]')
    parser.add_option('--binWidth', dest='binWidth', type='float', default=0.2, help='bin width in photons mode [ns]')
    parser.add_option('--side', dest='side', default='all', help='photons mode: all (both ends together), -1, +1 or split (one column per end)')
    parser.add_option('--firstEvent', dest='firstEvent', type='int', default=0, help='first event of the first bunch')
    (opts, args) = parser.parse_args()
    sides = {'all': [None], '-1': [-1], '+1': [1], 'split': [-1, 1]}[opts.side]
    if opts.mode != 'photons':
        sides = [None]   # the side only exists for the photons

    fileName = opts.prefix + ('_photons.csv' if opts.mode == 'photons' else '_primary.csv')
    d, bunch, t = loadBunches(fileName, opts.nPerBunch, opts.nBunches, opts.period, opts.tOffset, opts.firstEvent)
    tMax = 2 * opts.tOffset + opts.nBunches * opts.period

    fig = plotBunches(opts.mode, d, bunch, t, opts.nBunches, tMax, opts.binWidth, sides)
    os.makedirs(opts.outDir, exist_ok=True)
    sideTag = '' if opts.mode != 'photons' or opts.side == 'all' else '_side%s' % opts.side
    outFile = os.path.join(opts.outDir, 'bunches_%s_%dperBunch%s.png' % (opts.mode, opts.nPerBunch, sideTag))
    fig.savefig(outFile, dpi=120)
    print('Saved ' + outFile)
