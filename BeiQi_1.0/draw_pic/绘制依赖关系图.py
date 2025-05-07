from graphviz import Digraph
import os

#路径版本
# 创建一个有向图
dot_1 = Digraph(comment='依赖图')
dot_1.attr('node', fontname='SimHei')
dot_1.attr('edge', fontname='SimHei')

# 设置要读取的文件路径
file_path_1 = os.path.join("..", "datas//dependency_extract_path.txt")

# 定义椭圆形节点存储，避免重复创建
ellipse_nodes = set()

# 打开文件（使用GBK编码读取）
with open(file_path_1, 'r', encoding='GBK') as file:
    for line in file:
        # 去除行末的换行符并检查是否为空行
        stripped_line = line.strip()

        # 去除可能出现的尖括号
        stripped_line = stripped_line.strip('<>')

        if not stripped_line:
            continue

        # 解析依赖路径和数据名称
        if "依赖路径id：" in stripped_line and "被依赖路径id：" in stripped_line and "数据名称：" in stripped_line:
            # 按逻辑分割字符串来获取id和数据名称
            from_id = stripped_line.split("依赖路径id：")[1].split(" ")[0]
            to_id = stripped_line.split("被依赖路径id：")[1].split(" ")[0]
            data_name = stripped_line.split("数据名称：")[1].strip()

            # 如果数据名称对应的信息节点还没有创建，则创建它
            if data_name not in ellipse_nodes:
                dot_1.node(data_name, data_name, shape='ellipse')
                ellipse_nodes.add(data_name)

            # 设置从和到的节点为矩形
            dot_1.node(from_id, from_id, shape='box')
            dot_1.node(to_id, to_id, shape='box')

            # 连接依赖路径到数据名称，再从数据名称到被依赖路径
            dot_1.edge(from_id, data_name)
            dot_1.edge(data_name, to_id)

# 保存和渲染图形
output_path_1 = os.path.join("..", "datas//dependency_path_pic")
dot_1.render(output_path_1, view=False, format='png')

#需求图版本
# 创建另一个有向图
dot_2 = Digraph(comment='依赖图')
dot_2.attr('node', fontname='SimHei')
dot_2.attr('edge', fontname='SimHei')
# 设置要读取的文件路径
file_path_2 = os.path.join("..", "datas//dependency_extract_graph.txt")

# 定义椭圆形节点存储，避免重复创建
ellipse_nodes = set()

# 打开文件（使用GBK编码读取）
with open(file_path_2, 'r', encoding='GBK') as file:
    for line in file:
        # 去除行末的换行符并检查是否为空行
        stripped_line = line.strip()

        # 去除可能出现的尖括号
        stripped_line = stripped_line.strip('<>')

        if not stripped_line:
            continue

        # 解析依赖路径和数据名称
        if "依赖图名：" in stripped_line and "被依赖图名：" in stripped_line and "数据名称：" in stripped_line:
            # 按逻辑分割字符串来获取id和数据名称
            from_id = stripped_line.split("依赖图名：")[1].split(" ")[0]
            to_id = stripped_line.split("被依赖图名：")[1].split(" ")[0]
            data_name = stripped_line.split("数据名称：")[1].strip()

            # 如果数据名称对应的信息节点还没有创建，则创建它
            if data_name not in ellipse_nodes:
                dot_2.node(data_name, data_name, shape='ellipse')
                ellipse_nodes.add(data_name)

            # 设置从和到的节点为矩形
            dot_2.node(from_id, from_id, shape='box')
            dot_2.node(to_id, to_id, shape='box')

            # 连接依赖路径到数据名称，再从数据名称到被依赖路径
            dot_2.edge(from_id, data_name)
            dot_2.edge(data_name, to_id)

# 保存和渲染图形
output_path_2 = os.path.join("..", "datas//dependency_graph_pic")
dot_2.render(output_path_2, view=False, format='png')