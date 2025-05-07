# 节点
class Node:
    def __init__(self):
        self.node_id = None
        self.node_type = None
        self.branch_no = None # 只有判断节点才有
        self.branch_yes = None # 只有判断节点才有
        self.node_information = None


# 边
class Edge:
    def __init__(self):
        self.edge_id = None
        self.source_node_id = None
        self.target_node_id = None
        self.edge_information = None

# 图
class Graph:
    def __init__(self):
        self.graph_id = None
        self.graph_name = None
        self.nodes = []
        self.edges = []
