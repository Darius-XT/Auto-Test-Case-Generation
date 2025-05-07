import re
from collections import defaultdict
import chardet

def detect_encoding(file_path):
    """ 自动检测文件编码 """
    with open(file_path, 'rb') as f:
        raw_data = f.read(1024)  # 读取前 1024 字节
        result = chardet.detect(raw_data)
        return result['encoding']

def parse_graphs(file_path):
    """ 解析图结构文件，统计规模最大的图的节点数、边数，并返回其 ID """
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()

    graph_entries = re.findall(r'graph_id: (\S+)', content)  # 提取所有图的 ID
    graphs = re.split(r'graph_id: \S+', content)[1:]  # 分割多个图
    max_nodes, max_edges = 0, 0
    max_graph_id = None  # 记录规模最大的图 ID

    for i, graph in enumerate(graphs):
        node_count = len(re.findall(r'node_id:', graph))
        edge_count = len(re.findall(r'edge_id:', graph))

        if node_count > max_nodes:
            max_nodes, max_edges = node_count, edge_count
            max_graph_id = graph_entries[i]  # 记录当前规模最大的图 ID

    return max_graph_id, max_nodes, max_edges

def count_paths_in_graphs(file_path):
    """ 解析路径文件，统计包含最多路径的图 """
    encoding = detect_encoding(file_path)  # 检测编码
    graph_paths = defaultdict(int)  # 存储每个图的路径数量
    current_graph = None  # 当前图的 ID

    with open(file_path, 'r', encoding=encoding) as file:
        for line in file:
            line = line.strip()

            # 识别图 ID
            match_graph = re.match(r'图id：([\w-]+)', line)
            if match_graph:
                current_graph = match_graph.group(1)
                continue

            # 识别路径 ID，并增加对应图的路径计数
            if current_graph and line.startswith("路径id："):
                graph_paths[current_graph] += 1

    # 找到拥有最多路径的图
    if graph_paths:
        max_graph = max(graph_paths, key=graph_paths.get)
        max_paths = graph_paths[max_graph]
        return max_graph, max_paths
    else:
        return None, 0

# 文件路径（替换为你的实际路径）
graph_file_path = "D:\\me\\华师\\北汽\\代码\\BeiQi_1.0\\datas\\input.txt"  # 图结构文件
path_file_path = "D:\\me\\华师\\北汽\\代码\\BeiQi_1.0\\datas\\path_dic.txt"  # 路径文件

# 解析图结构文件
max_graph_id, max_nodes, max_edges = parse_graphs(graph_file_path)
print(f"规模最大的图 ID：{max_graph_id}，包含 {max_nodes} 个节点，{max_edges} 条边。")

# 解析路径文件
max_graph, max_paths = count_paths_in_graphs(path_file_path)
if max_graph:
    print(f"路径最多的图 ID：{max_graph}，包含路径数量：{max_paths}")
else:
    print("未找到任何路径信息。")
