from .block_structure import Block, Statement
from .graph_structure import Graph, Node, Edge
import re
import os

# 单个需求(包括文档原文, 生成的 if-else-block, 生成的 flow-chart-code与 graph)
class Requiement:
    def __init__(self):
        self.requirement_id = None
        self.requirement_content = None
        self.block = None
        self.flow = None
        self.graph = None

    # 输入对应需求的全部原始文件内容, 给requirement_id, content, block赋值
    def read_content_and_block_from_file(self, content_complete: str):
        content_complete = content_complete.strip()
        if not content_complete:
            return

        # 提取需求 ID
        match_id = re.search(r"需求ID:\s*(.+)", content_complete)
        # 提取原始输入
        match_content = re.search(r"原始输入:\s*(.*?)结构化输出:", content_complete, re.DOTALL)
        # 提取结构化输出
        match_block = re.search(r"结构化输出:\s*(.+)", content_complete, re.DOTALL)

        if match_id and match_content and match_block:
            self.requirement_id = match_id.group(1).strip()
            self.requirement_content = match_content.group(1).strip()
            self.block = Block(match_block.group(1).strip())

    # 根据 block, 生成 flow
    def generate_flowchart_code(self):

        # 根据statement_id生成流程图中的元素id
        def _label(statement: Statement, requirement_id: str) -> str:
            gid = re.sub(r"[^0-9A-Za-z_]", "_", requirement_id)
            return f"{gid}_{statement.statement_id}"

        # 找到节点的子节点中所有的 action 节点
        def _find_all_actions(statement: Statement) -> list[Statement]:
            actions = []
            def dfs(n: Statement):
                if n.statement_type == "action":
                    actions.append(n)
                for c in n.children:
                    dfs(c)
            dfs(statement)
            return actions

        lines: List[str] = ["flowchart TD"]
        lines.append('    Start([Start])')
        lines.append('    End([End])')

        # 1. 声明所有中间节点（跳过 else）
        for statement in self.block.statements:
            label = _label(statement, self.requirement_id)
            if statement.statement_type in ("if", "elif"):
                condition = statement.content.strip()
                if condition.startswith("if "):
                    condition = condition[3:]
                if condition.endswith(":"):
                    condition = condition[:-1]
                lines.append(f'    {label}{{"{condition}?"}}')
            elif statement.statement_type == "action":
                lines.append(f'    {label}["{statement.content}"]')

        outgoing_statements = set()

        # 2. 连接节点
        for statement in self.block.statements:
            if statement.statement_type == "else":
                continue

            parent = statement.parent
            if parent is None:
                continue

            lbl = _label(statement, self.requirement_id)
            plbl = _label(parent, self.requirement_id)

            # 自己是Action 节点
            if statement.statement_type == "action":
                sibs = parent.children
                idx = sibs.index(statement)

                # 如果父亲是if/elif,说明有对应的实体节点
                if parent.statement_type in ("if", "elif"):
                    # 如果有更早的兄弟节点,说明要把所有兄弟节点的所有无连线节点连到自己
                    if idx > 0:
                        for sib in sibs[:idx]:
                            for act in _find_all_actions(sib):
                                act_lbl = _label(act, self.requirement_id)
                                if act_lbl not in outgoing_statements:
                                    lines.append(f"    {act_lbl} --> {lbl}")
                                    outgoing_statements.add(act_lbl)
                    # 如果自己就是最早的孩子,则要父亲连自己
                    else:
                        lines.append(f"    {plbl} -->|yes| {lbl}")
                        outgoing_statements.add(plbl)

                # 如果父亲是else,说明没有对应的实体节点
                elif parent.statement_type == "else":
                    # 如果有更早的兄弟节点,说明要把所有兄弟节点的所有无连线节点连到自己
                    if idx > 0:
                        for sib in sibs[:idx]:
                            for act in _find_all_actions(sib):
                                act_lbl = _label(act, self.requirement_id)
                                if act_lbl not in outgoing_statements:
                                    lines.append(f"    {act_lbl} --> {lbl}")
                                    outgoing_statements.add(act_lbl)
                    # 如果自己就是最早的孩子,说明要找父亲的前一个兄弟(if/elif)来连自己
                    else:
                        gp = parent.parent
                        gp_sibs = gp.children
                        pidx = gp_sibs.index(parent)
                        prev = gp_sibs[pidx - 1]
                        lines.append(f"    {_label(prev, self.requirement_id)} -->|no| {lbl}")
                        outgoing_statements.add(_label(prev, self.requirement_id))

            # 自己是 If 节点
            elif statement.statement_type == "if":
                sibs = parent.children
                idx = sibs.index(statement)

                # 如果父节点也是if/elif
                if parent.statement_type in ("if", "elif"):
                    # 如果自己不是第一个孩子,说明一定有更早的,并列关系的兄弟if节点/action节点,此时所有兄弟节点所有没连线的节点都连自己
                    if idx > 0:
                        for sib in sibs[:idx]:
                            for act in _find_all_actions(sib):
                                act_lbl = _label(act, self.requirement_id)
                                if act_lbl not in outgoing_statements:
                                    lines.append(f"    {act_lbl} --> {lbl}")
                                    outgoing_statements.add(act_lbl)
                    # 如果自己是第一个孩子,说明只需要父亲连自己即可
                    else:
                        lines.append(f"    {plbl} -->|yes| {lbl}")
                        outgoing_statements.add(plbl)

                # 如果父节点是start,情况是类似的
                elif parent.statement_type == "start":
                    # 如果自己不是第一个孩子,说明有更早的,并列的if兄弟节点/action节点,所有兄弟的无连线子节点都要连自己
                    if idx > 0:
                        for sib in sibs[:idx]:
                            for act in _find_all_actions(sib):
                                act_lbl = _label(act, self.requirement_id)
                                if act_lbl not in outgoing_statements:
                                    lines.append(f"    {act_lbl} --> {lbl}")
                                    outgoing_statements.add(act_lbl)
                    # 如果自己是第一个孩子,那么父亲(start)连自己即可
                    else:
                        lines.append(f"    Start --> {lbl}")
                        outgoing_statements.add("Start")

                # 如果父节点是else(不是实体节点)
                elif parent.statement_type == "else":
                    # 如果自己有更靠前的兄弟,说明有并列的if兄弟节点/action节点,此时所有兄弟的无连线的子节点都要连自己
                    if idx > 0:
                        for sib in sibs[:idx]:
                            for act in _find_all_actions(sib):
                                act_lbl = _label(act, self.requirement_id)
                                if act_lbl not in outgoing_statements:
                                    lines.append(f"    {act_lbl} --> {lbl}")
                                    outgoing_statements.add(act_lbl)
                    # 如果自己就是最早的孩子节点,只需找else父节点的最早的兄弟,让它连接自己即可(对应它的no分支)
                    else:
                        gp = parent.parent
                        gp_sibs = gp.children
                        pidx = gp_sibs.index(parent)
                        prev = gp_sibs[pidx - 1]
                        lines.append(f"    {_label(prev, self.requirement_id)} -->|no| {lbl}")
                        outgoing_statements.add(_label(prev, self.requirement_id))

            # Elif 节点:之前一定有更早的if/elif节点,并且一定对应兄弟if/elif节点的no分支
            elif statement.statement_type == "elif":
                sibs = parent.children
                idx = sibs.index(statement)
                prev = sibs[idx - 1]
                lines.append(f"    {_label(prev, self.requirement_id)} -->|no| {lbl}")
                outgoing_statements.add(_label(prev, self.requirement_id))

        # 3. 将所有没有出边的节点连接到 End
        for statement in self.block.statements:
            if statement.statement_type not in ("start", "end", "else"):
                lbl = _label(statement, self.requirement_id)
                if lbl not in outgoing_statements:
                    lines.append(f"    {lbl} --> End")

        self.flow = "\n".join(lines)

    # 根据 block, 生成 graph
    def generate_graph_code(self):
        graph = Graph()
        graph.graph_id = self.requirement_id
        graph.graph_name = f"Flowchart of {self.requirement_id}"

        node_map = {}  # statement_id -> node_id
        node_list = []
        edge_list = []
        edge_count = 0

        # 生成 START 节点
        start_node = Node()
        start_node.node_id = "Start"
        start_node.node_type = "write"
        start_node.node_information = "START"
        node_list.append(start_node)
        # 生成对应的标签
        def _label(statement):
            return f"{self.requirement_id}_node_{statement.statement_id}"
        # 寻找节点的所有action子节点
        def _find_all_actions(statement):
            actions = []
            def dfs(n):
                if n.statement_type == "action":
                    actions.append(n)
                for c in n.children:
                    dfs(c)
            dfs(statement)
            return actions
        # 添加边
        def _add_edge(src_id, tgt_id, label=None):
            nonlocal edge_count
            edge = Edge()
            edge.edge_id = f"{self.requirement_id}_edge_{edge_count}"
            edge.source_node_id = src_id
            edge.target_node_id = tgt_id
            edge.edge_information = label
            edge_list.append(edge)
            edge_count += 1
            outgoing_statements.add(src_id)
        # 添加所有非 else 节点
        for stmt in self.block.statements:
            if stmt.statement_type == "else":
                continue
            node = Node()
            node.node_id = _label(stmt)
            node.node_information = stmt.content
            if stmt.statement_type in ("if", "elif"):
                node.node_type = "read"
            elif stmt.statement_type == "action":
                node.node_type = "write"
            else:
                continue  # start, end, else 不添加
            node_map[stmt.statement_id] = node.node_id
            node_list.append(node)

        outgoing_statements = set()

        for statement in self.block.statements:
            if statement.statement_type == "else":
                continue

            parent = statement.parent
            if parent is None:
                continue

            stmt_id = _label(statement)
            parent_id = _label(parent) if parent.statement_type != "start" else "Start"

            if statement.statement_type == "action":
                sibs = parent.children
                idx = sibs.index(statement)

                if parent.statement_type in ("if", "elif"):
                    if idx > 0:
                        for sib in sibs[:idx]:
                            for act in _find_all_actions(sib):
                                act_id = _label(act)
                                if act_id not in outgoing_statements:
                                    _add_edge(act_id, stmt_id)
                    else:
                        _add_edge(parent_id, stmt_id, "yes")

                elif parent.statement_type == "else":
                    if idx > 0:
                        for sib in sibs[:idx]:
                            for act in _find_all_actions(sib):
                                act_id = _label(act)
                                if act_id not in outgoing_statements:
                                    _add_edge(act_id, stmt_id)
                    else:
                        gp = parent.parent
                        prev = gp.children[gp.children.index(parent) - 1]
                        _add_edge(_label(prev), stmt_id, "no")

            elif statement.statement_type == "if":
                sibs = parent.children
                idx = sibs.index(statement)

                if parent.statement_type in ("if", "elif"):
                    if idx > 0:
                        for sib in sibs[:idx]:
                            for act in _find_all_actions(sib):
                                act_id = _label(act)
                                if act_id not in outgoing_statements:
                                    _add_edge(act_id, stmt_id)
                    else:
                        _add_edge(parent_id, stmt_id, "yes")
                elif parent.statement_type == "start":
                    if idx > 0:
                        for sib in parent.children[:idx]:
                            for act in _find_all_actions(sib):
                                act_id = _label(act)
                                if act_id not in outgoing_statements:
                                    _add_edge(act_id, stmt_id)
                    else:
                        _add_edge("Start", stmt_id)
                elif parent.statement_type == "else":
                    if idx > 0:
                        for sib in parent.children[:idx]:
                            for act in _find_all_actions(sib):
                                act_id = _label(act)
                                if act_id not in outgoing_statements:
                                    _add_edge(act_id, stmt_id)
                    else:
                        gp = parent.parent
                        prev = gp.children[gp.children.index(parent) - 1]
                        _add_edge(_label(prev), stmt_id, "no")

            elif statement.statement_type == "elif":
                prev = parent.children[parent.children.index(statement) - 1]
                _add_edge(_label(prev), stmt_id, "no")

        graph.nodes = node_list
        graph.edges = edge_list

        # 设置节点的 branch_yes 和 branch_no
        node_by_id = {node.node_id:node for node in node_list}
        for edge in edge_list:
            source_node = node_by_id.get(edge.source_node_id)
            if source_node and source_node.node_type == "read":
                if edge.edge_information == "yes":
                    source_node.branch_yes = edge.edge_id
                elif edge.edge_information == "no":
                    source_node.branch_no = edge.edge_id
                # 在前面的处理逻辑中, 为了识别哪个边是yes / no分支的出边, 给边的information赋了值, 现在要清空
                edge.edge_information = None

        self.graph = graph

    # 打印graph到控制台
    def print_graph_to_console(self):
        if not self.graph:
            print("No graph found.")
            return

        print(f"graph_id: {self.graph.graph_id}")
        print(f"graph_name: {self.graph.graph_id}")

        for node in self.graph.nodes:
            print(f"node_id: {node.node_id}")
            print(f"node_type: {node.node_type}")
            if node.branch_no is not None:
                print(f"branch_no: {node.branch_no}")
            if node.branch_yes is not None:
                print(f"branch_yes: {node.branch_yes}")
            print(f"node_information: {node.node_information}")
            print("node_over")

        for edge in self.graph.edges:
            print(f"edge_id: {edge.edge_id}")
            print(f"source_node_id: {edge.source_node_id}")
            print(f"target_node_id: {edge.target_node_id}")
            print(f"edge_information: {edge.edge_information or ''}")
            print("edge_over")

        print("graph_over")

# 存储整个工程(所有需求的合集)
class Requiements:
    def __init__(self):
        self.requiements = [] # 所有的 Requiement

    # 读取整个 txt 文件，提取每个需求块并构造成 Requiement 实例
    def read_file(self, file_path: str):
        with open(file_path, 'r', encoding='utf-8') as f:
            file_content = f.read()

        # 按分隔符“=”分割多个需求块
        requirement_blocks = file_content.split("=" * 60)

        for block in requirement_blocks:
            block = block.strip()
            if not block:
                continue

            # 创建并填充一个 Requiement 实例
            req = Requiement()
            req.read_content_and_block_from_file(block)

            # 加入到工程集合中
            self.requiements.append(req)

    # 生成所有的流程图代码
    def generate_flowchart_code(self):
        for requiement in self.requiements:
            requiement.generate_flowchart_code()

    def generate_graph_code(self):
        for requiement in self.requiements:
            requiement.generate_graph_code()

    # 打印所有的数据结构信息
    def print_blocks_data_structrue(self):
        for requiement in self.requiements:
            requiement.print_block()

    # 打印对应的流程图代码
    def print_blocks_if_else_and_flowchart(self):
        for requiement in self.requiements:
            requiement.print_block_with_flowchart()

    # 打印图到控制台
    def print_graph_to_console(self):
        for requiement in self.requiements:
            requiement.print_graph_to_console()

    # 保存流程图代码到文件
    def output_flowcharts(self):
        output_path = os.path.join(os.path.dirname(__file__), "..", "..", "datas", "flowchart.txt")
        output_path = os.path.abspath(output_path)

        with open(output_path, "w", encoding="utf-8") as f:
            for requiement in self.requiements:
                f.write("==== Requiement ID: {} ====\n".format(requiement.requirement_id))
                f.write("```mermaid\n")
                f.write(requiement.flow)
                f.write("\n```\n\n")

    # 保存 graph 到文件
    def output_graphs(self):
        output_path = os.path.join(os.path.dirname(__file__), "..", "..", "datas", "input.txt")
        with open(output_path, "w", encoding="utf-8") as f:
            for requiement in self.requiements:
                # 写入 graph_id 和 graph_name（用 graph_id 代替）
                f.write(f"graph_id: {requiement.graph.graph_id}\n")
                f.write(f"graph_name: {requiement.graph.graph_id}\n")

                # 写入所有节点
                for node in requiement.graph.nodes:
                    f.write(f"node_id: {node.node_id}\n")
                    f.write(f"node_type: {node.node_type}\n")
                    if node.branch_no is not None:
                        f.write(f"branch_no: {node.branch_no}\n")
                    if node.branch_yes is not None:
                        f.write(f"branch_yes: {node.branch_yes}\n")
                    f.write(f"node_information: {node.node_information}\n")
                    f.write("node_over\n")

                # 写入所有边
                for edge in requiement.graph.edges:
                    f.write(f"edge_id: {edge.edge_id}\n")
                    f.write(f"source_node_id: {edge.source_node_id}\n")
                    f.write(f"target_node_id: {edge.target_node_id}\n")
                    f.write(f"edge_information: {edge.edge_information or ''}\n")
                    f.write("edge_over\n")

                # 写入图结束标志
                f.write("graph_over\n")