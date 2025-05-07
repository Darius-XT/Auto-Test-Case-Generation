import sys
import re
import time
import os
from fsm.env.environment_runtime import RuntimeNamespace
from fsm.fsm_fsm import FSM
from fsm.solution import Solution

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

def write_to_file(file_path, content):
    with open(file_path, 'a', encoding='utf-8', errors='ignore') as f:
        f.write(content + '\n')

if __name__ == '__main__':
    # 记录开始时间
    start_time = time.time()
    if len(sys.argv) > 1:
        """
        (solution_path) ..\\demo_分支_环路_输出
        (graph_id) d5bcc160-f830-11ee-93f4-db3fea7624c7
        (env_name) default
        (database_id) c263f6cb-c1e3-4f04-a714-fa54b58abbca-default
        """
        solution_path = sys.argv[1]
        graph_id = sys.argv[2]
        env_name = sys.argv[3]
        database_id = sys.argv[4]
    else:
        print('usage: python run_test_case_gen solution_path graph_id env_name [template_path]')
        sys.exit()

    try:
        file_path = os.path.join("..", "..", "datas\\input.txt")
        with open(file_path, 'w') as f:
            pass  # 清空文件内容

        fsm = get_fsm_from_solution(solution_path, env_name, database_id)
        if fsm:
            patterns_blank = {
                '==': ' == ',
                '= =':' == ',
                '!=': ' != ',
                '<=': ' <= ',
                '>=': ' >= ',
                '<': ' < ',
                '>': ' > ',
                '=': ' = ',
                '≥': ' ≥ ',
                '≤': ' ≤ '
            }

            patterns_others = {
                '= =':'=='
            }

            # 预编译正则表达式
            blank_patterns = {re.compile(r'(?<! ){}(?! )'.format(re.escape(pattern))): repl for pattern, repl in patterns_blank.items()}
            other_patterns = {re.compile(re.escape(pattern)): repl for pattern, repl in patterns_others.items()}

            output = []

            for graph in fsm.graphs:
                output.append(f"graph_id: {graph.id}")
                output.append(f"graph_name: {graph.graph_name}")

                for node in graph.nodes:
                    output.append(f"node_id: {node.id}")
                    if node.type_name == 'state':  # 是写节点
                        output.append("node_type: write")
                        if node.is_init_node:
                            output.append("node_information: START")
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
                            actions_str = " and ".join(actions)
                            output.append(f"node_information: {actions_str}")

                    elif node.type_name == 'condition':  # 是读节点
                        output.append("node_type: read")
                        if node.condition != "":
                            modified_str = node.condition
                            for pattern, repl in blank_patterns.items():
                                modified_str = pattern.sub(repl, modified_str)
                            for pattern, repl in other_patterns.items():
                                modified_str = pattern.sub(repl, modified_str)
                            node.condition = modified_str
                            output.append(f"branch_yes: {node.branch_yes}")
                            output.append(f"branch_no: {node.branch_no}")
                            output.append(f"node_information: {node.condition}")
                    output.append("node_over")

                for transition in graph.transitions:
                    output.append(f"edge_id: {transition.id}")
                    output.append(f"source_node_id: {transition.source_node}")
                    output.append(f"target_node_id: {transition.target_node}")
                    # if transition.condition != "":
                    modified_str = transition.condition
                    for pattern, repl in blank_patterns.items():
                        modified_str = pattern.sub(repl, modified_str)
                    for pattern, repl in other_patterns.items():
                        modified_str = pattern.sub(repl, modified_str)
                    output.append(f"edge_information: {modified_str}")

                    output.append("edge_over")
                output.append("graph_over")
            write_to_file(file_path, "\n".join(output))
        else:
            write_to_file(file_path, 'Failed to create FSM.')

    except Exception as e:
        write_to_file(file_path, str(e))

    # 记录结束时间
    end_time = time.time()

    # 计算并输出执行时间（以秒为单位）
    elapsed_time = end_time - start_time
    print("Elapsed time:", elapsed_time, "s")
