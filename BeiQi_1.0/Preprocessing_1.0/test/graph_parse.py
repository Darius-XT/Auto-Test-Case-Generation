import sys
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

if __name__ == '__main__':
    if len(sys.argv) > 1:
        """
        (solution_path) ..\demo_分支_环路_输出 
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
        fsm = get_fsm_from_solution(solution_path, env_name, database_id)
        # fsm.env.gen_mapping_file_in_json(f'{solution_path}\\env')
        ######################################################################
        # 获取解析后的图形信息
        for graph in fsm.graphs:
            print(graph)
    except Exception as e:
        print(e, file=sys.stderr)


#################################################################################
# 可只关注关键属性，其余属性主要应用于测试覆盖分析，后续需要再提供
# fsm.graphs中存储的是解析后的图形实例列表
# *************图形关键属性如下:
# 1.graph_name: 图名
# 2.graph_type: 图类型，需求图对应为request
# 3.id: uuid
# 4.nodes: 列表，存储除transition外的所有节点实例
# 5.transitions: 列表，存储transition的所有实例
# *************transition关键属性如下:
# 1.id: uuid
# 2.condition: 跳转线上的条件
# 3.parent: 上级父类（一般为图形实例）
# 4.source_node: 起始节点id
# 5.target_node: 目标节点id（箭头指向的节点）
# 6.test_coverage: 针对condition的覆盖原则设置
# 7.test_layer: 测试人员针对condition所设置的自定义测试点（边界值、等价类算法会自动生成）
# *************state关键属性如下:
# 1.id: uuid
# 2.input_transitions: 输入的跳转线列表
# 3.output_transitions: 输出的跳转线列表
# 4.parent: 上级父类（一般为图形实例）
# 5.during_action_list: 当前状态的赋值动作
# 6.forward_propagation: 若勾选，则会将当前状态对于信号的赋值结果记录，参与到后续跳转上条件的判定于覆盖
# *************condition关键属性如下:
# 1.id: uuid
# 2.input_transitions: 输入的跳转线列表
# 3.output_transitions: 输出的跳转线列表
# 4.branch_no: 分支条件取false对应的transition id
# 5.branch_yes: 分支条件取true对应的transition id
# 6.condition: 分支条件
# 7.parent: 上级父类（一般为图形实例）
# 8.test_layer: 测试人员针对condition所设置的自定义测试点（边界值、等价类算法会自动生成）
# *************call关键属性如下:
# 1.id: uuid
# 2.params_list: 形参列表
# 3.script: 函数体
# 4.desc: 函数名
# *************GraphRef关键属性如下:
# 1.id: uuid
# 2.input_transitions: 输入的跳转线列表
# 3.output_transitions: 输出的跳转线列表
# 4.graph_id: 引用的图id
# *************Then关键属性如下:
# 1.id: uuid
# 2.input_transitions: 输入的跳转线列表
# 3.output_transitions: 输出的跳转线列表
# *************Goto关键属性如下:
# 1.id: uuid
# 2.input_transitions: 输入的跳转线列表
# 3.output_transitions: 应为空
# 4.friend_id: 关联的From组件的id
# *************From关键属性如下:
# 1.id: uuid
# 2.input_transitions: 应为空
# 3.output_transitions: 输出的跳转线列表
# 4.friend_id: 关联的From组件的id
