import requests
import json
import sys
import re
import time
import os
from fsm.env.environment_runtime import RuntimeNamespace
from fsm.fsm_fsm import FSM
from fsm.solution import Solution

def send_graph(solution_path,graph_ids,env_name,database_id):
    data = {"graphs":[]}
    fsm = get_fsm_from_solution(solution_path, env_name, database_id)
    if fsm:
        patterns_blank = {
            '==':' == ',
            '= =':' == ',
            '!=':' != ',
            '<=':' <= ',
            '>=':' >= ',
            '<':' < ',
            '>':' > ',
            '=':' = ',
            '≥':' ≥ ',
            '≤':' ≤ '
        }

        patterns_others = {
            '= =':'=='
        }

        # 预编译正则表达式
        blank_patterns = {re.compile(r'(?<! ){}(?! )'.format(re.escape(pattern))):repl for pattern, repl in
                          patterns_blank.items()}
        other_patterns = {re.compile(re.escape(pattern)):repl for pattern, repl in patterns_others.items()}
        # 确保 graph_ids 是一个列表
        if isinstance(graph_ids, str):
            graph_ids = graph_ids.split(",")  # 将逗号分隔的字符串转换为列表
        for graph in fsm.graphs:
            for graph_id in graph_ids:
                if graph.id == graph_id:
                    graph_data = {"graph_id":graph.id, "nodes":[], "edges":[]}

                    # 处理节点
                    for node in graph.nodes:
                        node_data = {"node_id":node.id, "node_type":"", "node_information":""}

                        if node.type_name == 'state':  # 是写节点
                            node_data["node_type"] = "write"
                            if node.is_init_node:
                                node_data["node_information"] = "START"
                            else:
                                actions = []
                                for action in node.during_action_list:
                                    modified_str = action.express
                                    for pattern, repl in blank_patterns.items():
                                        modified_str = pattern.sub(repl, modified_str)
                                    for pattern, repl in other_patterns.items():
                                        modified_str = pattern.sub(repl, modified_str)
                                    action.express = modified_str
                                    actions.append(action.express)  # 把表达式存储到列表中
                                node_data["node_information"] = " and ".join(actions)

                        elif node.type_name == 'condition':  # 是读节点
                            node_data["node_type"] = "read"
                            if node.condition != "":
                                modified_str = node.condition
                                for pattern, repl in blank_patterns.items():
                                    modified_str = pattern.sub(repl, modified_str)
                                for pattern, repl in other_patterns.items():
                                    modified_str = pattern.sub(repl, modified_str)
                                node.condition = modified_str
                                node_data["branch_yes"] = node.branch_yes
                                node_data["branch_no"] = node.branch_no
                                node_data["node_information"] = node.condition

                        graph_data["nodes"].append(node_data)

                    # 处理边
                    for transition in graph.transitions:
                        edge_data = {
                            "edge_id":transition.id,
                            "source_node_id":transition.source_node,
                            "target_node_id":transition.target_node,
                            "edge_information":""
                        }
                        if transition.condition != "":
                            modified_str = transition.condition
                            for pattern, repl in blank_patterns.items():
                                modified_str = pattern.sub(repl, modified_str)
                            for pattern, repl in other_patterns.items():
                                modified_str = pattern.sub(repl, modified_str)
                            transition.condition = modified_str
                            edge_data["edge_information"] = transition.condition

                        graph_data["edges"].append(edge_data)

                    data["graphs"].append(graph_data)
    else:
        print("Fail to create fsm")
    # 要发送的JSON数据
    print(json.dumps(data, indent=4,ensure_ascii=False).encode('gbk'))  # 格式化输出 JSON 数据

    # 向C++服务器发送POST请求，使用正确的路由 /data
    response = requests.post("http://localhost:8080/data", json=data)

    # 解析并打印响应
    if response.status_code == 200:
        print("Response from C++ server:", response.text)  # 使用 text 来获取纯文本响应
    else:
        print("Failed to connect to the server. Status code:", response.status_code)

def get_fsm_from_solution(solution_path, env_name, database_id):
    solution = Solution(solution_path, database_id)
    if not solution.check_valid():
        solution = None
    if solution is None:
        print('Cannot load the solution folder from:' + solution_path)
        return None
    env = solution.env_config.select_env(env_name)
    graphs = solution.get_all_graph_list()
    local_runtime = {}
    env.global_runtime = env.runtime.shallow_copy()
    for graph_item in graphs:
        env.local_runtime[graph_item.id] = RuntimeNamespace()
        local_runtime_item = env.local_runtime[graph_item.id].gen_local_runtime_item(graph_item, env)
        local_runtime[graph_item.id] = local_runtime_item
    env.local_runtime = local_runtime
    if env is None:
        print('Failed to open env:' + env_name)
        return None
    if graphs is None:
        print('Failed to get graph list')
        return None
    return FSM(graphs, env)

if __name__ == '__main__':
    if len(sys.argv) > 1:
        """
        (solution_path) ..\\demo_分支_环路_输出
        (graph_ids) d5bcc160-f830-11ee-93f4-db3fea7624c7
        (env_name) default
        (database_id) c263f6cb-c1e3-4f04-a714-fa54b58abbca-default
        """
        solution_path = sys.argv[1]
        graph_ids = sys.argv[2]
        env_name = sys.argv[3]
        database_id = sys.argv[4]
        send_graph(solution_path,graph_ids,env_name,database_id)
    else:
        print('usage: python run_test_case_gen solution_path graph_id env_name [template_path]')
        sys.exit()



