# Plots the output of Carlo's nanowire simulator (<prefix>_nw_current_nA.bin) together with the
# Monte Carlo truth written by exportArr.C (<prefix>_hits_truth.csv).
#
#   event mode: current of the two ends (channel_minus, channel_plus) of every fiber crossed by the
#               primary in one event, with the true hit time and the expected light arrival times
#   dt mode:    for every crossing in every chunk, measures the arrival time at each end with a constant
#               fraction discriminator (time at which the leading edge reaches frac * peak) and compares
#               dt_meas = t_minus - t_plus with the truth dt = 2 * localz / vFiber (> 0 if localz > 0)
#
# Usage (from ProtonTomographySimulator-build):
#   python ../ProtonTomographySimulator/dataAnalysis/plotNanowire.py -m event -p bridge/all_000 -e 0
#   python ../ProtonTomographySimulator/dataAnalysis/plotNanowire.py -m dt -p "bridge/all_*"

import glob, optparse, os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

NChannels = 336
vFiber = 18.7       # cm/ns, same value as exportArr.C
halfLength = 10.0   # cm, fiber half length (length in the json file)


def readSummary(prefix):
    summary = {}
    for line in open(prefix + '_nw_summary.txt'):
        if ' = ' in line:
            key, value = line.split(' = ', 1)
            summary[key.strip()] = value.strip()
    return summary


def loadChunk(prefix):
    # memmap: the file is not loaded in RAM, only the waveforms that are used are read
    summary = readSummary(prefix)
    nEvents, nChannels, nTime = int(summary['n_events']), int(summary['n_channels']), int(summary['n_time'])
    dt = float(summary['dt_ns'])
    current = np.memmap(prefix + '_nw_current_nA.bin', dtype='<f4', mode='r', shape=(nEvents, nChannels, nTime))
    truth = np.genfromtxt(prefix + '_hits_truth.csv', delimiter=',', names=True)
    truth = np.atleast_1d(truth)
    return current, np.arange(nTime) * dt, truth


def cfdTime(w, t, frac, minPeak):
    # Leading edge time at which the waveform reaches frac * peak (linear interpolation between samples)
    p = int(np.argmax(w))
    if w[p] < minPeak:
        return np.nan
    thr = frac * w[p]
    i = int(np.argmax(w[:p + 1] >= thr))
    if i == 0:
        return t[0]
    return t[i - 1] + (thr - w[i - 1]) / (w[i] - w[i - 1]) * (t[i] - t[i - 1])


def plotEvent(prefix, event, outDir):
    current, t, truth = loadChunk(prefix)
    hits = truth[truth['event'] == event]
    if len(hits) == 0:
        print('No primary crossings in event %d of %s' % (event, prefix))
        return
    fig, axes = plt.subplots(len(hits), 1, figsize=(9, 2.6 * len(hits)), sharex=True, squeeze=False)
    for ax, h in zip(axes[:, 0], hits):
        chM, chP = int(h['channel_minus']), int(h['channel_plus'])
        ax.plot(t, current[event, chM], label='ch %d (side -1)' % chM)
        ax.plot(t, current[event, chP], label='ch %d (side +1)' % chP)
        # Direct light: hit time + distance to each end / vFiber
        ax.axvline(h['t_hit_ns'], color='k', ls=':', lw=1, label='t_hit')
        ax.axvline(h['t_hit_ns'] + (halfLength + h['localz_cm']) / vFiber, color='C0', ls='--', lw=1)
        ax.axvline(h['t_hit_ns'] + (halfLength - h['localz_cm']) / vFiber, color='C1', ls='--', lw=1)
        ax.set_title('det %d  layer %d  fiber %d   localz = %.2f cm   dt_true = %.3f ns' %
                     (h['det'], h['layer'], h['fiber'], h['localz_cm'], h['dt_true_ns']), fontsize=9)
        ax.set_ylabel('I [nA]')
        ax.legend(fontsize=7, loc='upper right')
    axes[-1, 0].set_xlabel('t [ns]')
    fig.tight_layout()
    fileName = os.path.join(outDir, '%s_event%d.png' % (os.path.basename(prefix), event))
    fig.savefig(fileName, dpi=120)
    print('Saved ' + fileName)


def plotDt(pattern, outDir, frac, minPeak):
    prefixes = sorted(f[:-len('_nw_current_nA.bin')] for f in glob.glob(pattern + '_nw_current_nA.bin'))
    if not prefixes:
        print('No files match ' + pattern + '_nw_current_nA.bin')
        return
    localz, dtMeas, peaks = [], [], []
    for prefix in prefixes:
        current, t, truth = loadChunk(prefix)
        for h in truth:
            ev = int(h['event'])
            wM, wP = current[ev, int(h['channel_minus'])], current[ev, int(h['channel_plus'])]
            tM, tP = cfdTime(wM, t, frac, minPeak), cfdTime(wP, t, frac, minPeak)
            localz.append(h['localz_cm'])
            dtMeas.append(tM - tP)
            peaks.append(min(wM.max(), wP.max()))
        print('%s: %d crossings' % (prefix, len(truth)))

    localz, dtMeas, peaks = np.array(localz), np.array(dtMeas), np.array(peaks)
    dtTrue = 2 * localz / vFiber    # signed: t_minus - t_plus
    ok = np.isfinite(dtMeas)
    res = dtMeas[ok] - dtTrue[ok]
    signOk = np.mean(np.sign(dtMeas[ok]) == np.sign(dtTrue[ok]))
    zRes = res * vFiber / 2

    print('crossings = %d, with both ends above %.3g nA = %d' % (len(localz), minPeak, ok.sum()))
    print('dt residual: mean = %.4f ns, std = %.4f ns  ->  localz resolution ~ %.2f cm' % (res.mean(), res.std(), zRes.std()))
    print('correct side (label) = %.1f %%' % (100 * signOk))

    fig, axes = plt.subplots(2, 2, figsize=(11, 8))
    ax = axes[0, 0]
    ax.scatter(dtTrue[ok], dtMeas[ok], s=3, alpha=0.4)
    lim = [dtTrue[ok].min(), dtTrue[ok].max()]
    ax.plot(lim, lim, 'r--', lw=1, label='dt_meas = dt_true')
    ax.set_xlabel('dt_true = 2 localz / v [ns]')
    ax.set_ylabel('dt_meas = t(-) - t(+) [ns]')
    ax.legend(fontsize=8)

    ax = axes[0, 1]
    ax.hist(res, bins=100)
    ax.set_xlabel('dt_meas - dt_true [ns]')
    ax.set_title('mean %.3f ns, std %.3f ns' % (res.mean(), res.std()), fontsize=9)

    ax = axes[1, 0]
    ax.scatter(localz[ok], zRes, s=3, alpha=0.4)
    ax.set_xlabel('localz true [cm]')
    ax.set_ylabel('localz meas - true [cm]')
    ax.set_title('localz resolution ~ %.2f cm' % zRes.std(), fontsize=9)

    ax = axes[1, 1]
    ax.hist(peaks, bins=100)
    ax.set_yscale('log')
    ax.set_xlabel('peak current of the weaker end [nA]')
    ax.set_title('correct side: %.1f %%' % (100 * signOk), fontsize=9)

    fig.suptitle('CFD %.0f %%, %d crossings' % (100 * frac, ok.sum()))
    fig.tight_layout()
    fileName = os.path.join(outDir, 'nanowire_dt.png')
    fig.savefig(fileName, dpi=120)
    print('Saved ' + fileName)


if __name__ == '__main__':

    parser = optparse.OptionParser(usage='usage: %prog [options]')
    parser.add_option('-m', '--mode', dest='mode', default='event', help='event or dt')
    parser.add_option('-p', '--prefix', dest='prefix', default='bridge/all_000', help='prefix (dt mode: glob pattern, e.g. "bridge/all_*")')
    parser.add_option('-e', '--event', dest='event', type='int', default=0, help='event index inside the chunk (event mode)')
    parser.add_option('-o', '--outDir', dest='outDir', default='nanowirePlots', help='output directory')
    parser.add_option('--frac', dest='frac', type='float', default=0.2, help='CFD fraction of the peak')
    parser.add_option('--minPeak', dest='minPeak', type='float', default=0.02, help='minimum peak current [nA]')
    (opts, args) = parser.parse_args()

    os.makedirs(opts.outDir, exist_ok=True)
    if opts.mode == 'event':
        plotEvent(opts.prefix, opts.event, opts.outDir)
    else:
        plotDt(opts.prefix, opts.outDir, opts.frac, opts.minPeak)
