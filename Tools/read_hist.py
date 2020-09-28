import numpy as np
import matplotlib.pyplot as plt
import boost_histogram as bh
import operator
import functools

def read_hist(filename):
    """
    read numpy file produced with CORSIKA 8's save_hist() function into
    boost-histogram object.
    """

    d = np.load(filename)
    axistypes = d['axistypes'].view('c')    
    
    axes = []
    for i, at in enumerate(axistypes):
        if at == b'c':
            axes.append(bh.axis.Variable(d[f'binedges_{i}'], overflow=True, underflow=True))
        elif at == b'd':
            axes.append(bh.axis.IntCategory(d[f'binedges_{i}']))
        
    h = bh.Histogram(*axes)
    h.view(flow=True)[:] = d['data']
    
    return h

