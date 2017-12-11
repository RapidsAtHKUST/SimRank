import networkx as nx
import sys
if __name__ == '__main__':
    if len(sys.argv) < 3:
        print 'Usage: genData <graphType [SF, ER]> <nodeNum> <perIndegree>'
        sys.exit(1)
    nodeN = int (sys.argv[2])
    inDeg = int (sys.argv[3])
    gtype = str (sys.argv[1])
	
    graph_name = gtype + str(nodeN) + '-' + str(inDeg) + '.data'
    if gtype == 'ER':
        print gtype
        G = nx.random_graphs.erdos_renyi_graph(nodeN, 1.0 * inDeg / nodeN, None, True)
    elif gtype == 'WS':
        G = nx.random_graphs.watts_strogatz_graph(nodeN, inDeg, 0.3)
    elif gtype == 'SF':
        g = nx.empty_graph(100);
        print g
        # G = nx.scale_free_graph(nodeN, 0.49, 0.50, 0.01, 0.2, 0, create_using = g)
    elif gtype == 'BA':
        G = nx.barabasi_albert_graph(nodeN, inDeg)
    else:
        print 'invalid graph type. Valid ones are [ER, WS, SF]'
        sys.exit(1)
    print '------'
    nx.write_edgelist(G, graph_name, data = False)
